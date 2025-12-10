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
#if defined(BOXEDWINE_DYNAMIC32) || defined(BOXEDWINE_JIT_X64)

#include "jitX86CodeGen.h"
#include "../jit/jitSSE.h"
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
#define INVALID_REG 0xff

#ifdef BOXEDWINE_64
static U8 regCache[] = { 0, 1, 2, 3, 10, 5, 6, 7 };
#else
static U8 regCache[] = { 5, INVALID_REG, INVALID_REG, INVALID_REG, INVALID_REG, INVALID_REG, INVALID_REG, INVALID_REG };
#endif

#ifdef BOXEDWINE_64
// r8, r9, r11, r12, r14 will be used for tmp, see findTmpReg

// r13/r15 chosen since they are non volitile
//#define HOST_MMU x86.r14
#define HOST_RAM x86.r15
#define HOST_CPU x86.r13
#define MEM_PTR X86Asm::Mem64
#define RN(x) X86Asm::Reg64::from(x)
#define PARAM_CALL_TMP x86.rax
#else

#define HOST_CPU x86.edi
#define MEM_PTR X86Asm::Mem32
#define RN(x) X86Asm::Reg32::from(x)
#endif

#define R64(x) X86Asm::Reg64::from(x)
#define R32(x) X86Asm::Reg32::from(x)
#define R16(x) X86Asm::Reg16::from(x)
#define R8(x) X86Asm::Reg8::from(x)
#define R(x) X86Asm::Reg::from(x)

class JitX86CodeGen : public JitSSE {
public:    
    JitX86CodeGen(CPU* cpu) : JitSSE(cpu) {
#ifdef BOXEDWINE_64
#ifdef BOXEDWINE_MSVC
        params[0] = x86.rcx;
        params[1] = x86.rdx;
        params[2] = x86.r8;
        params[3] = x86.r9;
#else
        params[0] = x86.rdi;
        params[1] = x86.rsi;
        params[2] = x86.rdx;
        params[3] = x86.rcx;
#endif
#endif
    }

    void preOp(DecodedOp* op) override;
    RegPtr getReg(U8 reg, S8 hint = -1, bool load = true) override;
    RegPtr getReg8(U8 reg) override;
    RegPtr getReadOnlyReg(U8 reg, bool delayed = false, S8 hint = -1) override;
    RegPtr getReadOnlyReg8(U8 reg, bool delayed = false, S8 hint = -1) override;
    RegPtr getTmpReg() override;
    RegPtr getTmpRegWithHint(S8 hint) override;
    RegPtr getTmpReg(U8 reg, bool delayed = false, S8 hint = -1) override;
    RegPtr getTmpReg8(U8 reg, bool delayed = false, S8 hint = -1) override;
    RegPtr getTmpReg8() override;
    RegPtr getTmpRegForCallResult() override;
    RegPtr getReadOnlySegAddress(U8 reg) override;
    RegPtr getTmpSegAddress(U8 reg) override;
    RegPtr getReadOnlySegValue(U8 reg) override;
    RegPtr getTmpEip() override;
    void pushReg(RegPtr reg) override;
    void popReg(RegPtr reg) override;
    bool isTmpRegAvailable() override;    
    void forceSyncBackIfNotCached(RegPtr reg) override;

    RegPtr getEip(bool load = true) override;
    U8 findTmpReg(bool needs8bitReg, S8 hint = -1);
    void emulateSingleOp() override;

    void addReg(JitWidth regWidth, RegPtr reg, RegPtr rm) override;
    void addValue(JitWidth regWidth, RegPtr reg, U32 imm) override;
    void orReg(JitWidth regWidth, RegPtr reg, RegPtr rm) override;
    void orValue(JitWidth regWidth, RegPtr reg, U32 imm) override;
    void subReg(JitWidth regWidth, RegPtr reg, RegPtr rm) override;
    void subValue(JitWidth regWidth, RegPtr reg, U32 imm) override;
    void andReg(JitWidth regWidth, RegPtr reg, RegPtr rm) override;
    void andValue(JitWidth regWidth, RegPtr reg, U32 immm) override;
    void xorReg(JitWidth regWidth, RegPtr reg, RegPtr rm) override;
    void xorValue(JitWidth regWidth, RegPtr reg, U32 immm) override;
    void shrReg(JitWidth regWidth, RegPtr reg, RegPtr rm) override;
    void shrValue(JitWidth regWidth, RegPtr reg, U32 immm) override;    
    void shlReg(JitWidth regWidth, RegPtr reg, RegPtr rm) override;
    void shlValue(JitWidth regWidth, RegPtr reg, U32 immm) override;
    void sarReg(JitWidth regWidth, RegPtr reg, RegPtr rm) override;
    void sarValue(JitWidth regWidth, RegPtr reg, U32 immm) override;
    void notReg2(JitWidth regWidth, RegPtr reg) override;
    void negReg2(JitWidth regWidth, RegPtr reg) override;
    void bsfReg(JitWidth regWidth, RegPtr reg, RegPtr rm) override;
    void bsrReg(JitWidth regWidth, RegPtr reg, RegPtr rm) override;
    void rolReg(JitWidth regWidth, RegPtr reg, RegPtr rm) override;
    void rolValue(JitWidth regWidth, RegPtr reg, U32 imm) override;
    void rorReg(JitWidth regWidth, RegPtr reg, RegPtr rm) override;
    void rorValue(JitWidth regWidth, RegPtr reg, U32 imm) override;
    void rclReg(JitWidth regWidth, RegPtr reg, RegPtr rm) override;
    void rclValue(JitWidth regWidth, RegPtr reg, U32 imm) override;
    void rcrReg(JitWidth regWidth, RegPtr reg, RegPtr rm) override;
    void rcrValue(JitWidth regWidth, RegPtr reg, U32 imm) override;
    void shldReg(JitWidth regWidth, RegPtr reg, RegPtr rm, RegPtr cl) override;
    void shldValue(JitWidth regWidth, RegPtr reg, RegPtr rm, U32 imm) override;
    void shrdReg(JitWidth regWidth, RegPtr reg, RegPtr rm, RegPtr cl) override;
    void shrdValue(JitWidth regWidth, RegPtr reg, RegPtr rm, U32 imm) override;    
    void xchgReg(JitWidth regWidth, RegPtr dest, RegPtr src) override;
    void xaddReg(JitWidth regWidth, RegPtr reg, RegPtr rm) override;
    void mulReg(JitWidth regWidth, RegPtr reg) override;
    void imulReg(JitWidth regWidth, RegPtr reg) override;
    void imulRRI(JitWidth regWidth, RegPtr dst, RegPtr src, U32 src2) override;
    void imulRR(JitWidth regWidth, RegPtr dst, RegPtr src) override;
    void divRegRegWithRemainder32(RegPtr destLow, RegPtr destHigh, RegPtr src, RegPtr remainder) override;
    void divRegRegWithRemainder(JitWidth regWidth, RegPtr dest, RegPtr src, RegPtr remainder) override;
    void idivRegRegWithRemainder(JitWidth regWidth, RegPtr dest, RegPtr src, RegPtr remainder) override;
    void idivRegRegWithRemainder32(RegPtr destLow, RegPtr destHigh, RegPtr src, RegPtr remainder) override;

    void byteSwapReg32(RegPtr reg) override;
    RegPtr compareReg(JitWidth regWidth, RegPtr reg1, RegPtr reg2, JitEvaluate condition, RegPtr resultReg = nullptr) override;    
    RegPtr compareValue(JitWidth regWidth, RegPtr reg, U32 value, JitEvaluate condition, RegPtr resultReg = nullptr) override;

    void readRamPage(RegPtr dest, RegPtr index) override;
    void readMMU(RegPtr dest, RegPtr index) override;
    void read(JitWidth width, RegPtr dest, RegPtr reg, U8 lsl, U32 disp) override;
    void read(JitWidth width, RegPtr dest, RegPtr reg, RegPtr sib, U8 lsl, U32 disp) override;
    void write(JitWidth width, RegPtr reg, U32 disp, RegPtr src) override;
    void write(JitWidth width, RegPtr reg, RegPtr sib, U8 lsl, U32 disp, RegPtr src) override;
    void write(JitWidth width, RegPtr reg, RegPtr sib, U8 lsl, U32 disp, U32 value) override;

    RegPtr readCPU(JitWidth width, U32 offset, RegPtr resultReg = nullptr) override;
    RegPtr readCPU(JitWidth width, RegPtr sib, U8 lsl, U32 offset) override;
    void writeCPU(JitWidth width, U32 offset, RegPtr src) override;
    void writeCPU(JitWidth width, RegPtr sib, U8 lsl, U32 offset, RegPtr src) override;
    void writeCPUValue(JitWidth width, RegPtr sib, U8 lsl, U32 offset, DYN_PTR_SIZE src) override;
    void writeCPUValue(JitWidth width, U32 offset, DYN_PTR_SIZE src) override;

    void mov(JitWidth regWidth, RegPtr dest, RegPtr src) override;
    void movValue(JitWidth regWidth, RegPtr dst, DYN_PTR_SIZE imm) override;
    void movzx(JitWidth dstWidth, RegPtr dest, JitWidth srcWidth, RegPtr src) override;
    void movsx(JitWidth dstWidth, RegPtr dest, JitWidth srcWidth, RegPtr src) override;

    void If(JitWidth regWidth, RegPtr reg) override;
    void IfTest(JitWidth regWidth, RegPtr reg, RegPtr mask) override;
    void IfTest(JitWidth regWidth, RegPtr reg, U32 value) override;
    void IfNotTest(JitWidth regWidth, RegPtr reg, U32 value) override;
    void IfEqual(JitWidth regWidth, RegPtr reg, DYN_PTR_SIZE value) override;
    void IfEqual(JitWidth regWidth, RegPtr reg1, RegPtr reg2) override;
    void IfNotEqual(JitWidth regWidth, RegPtr reg, DYN_PTR_SIZE value) override;
    void IfNotEqual(JitWidth regWidth, RegPtr reg, RegPtr reg2) override;
    void IfLessThan2(JitWidth regWidth, RegPtr reg, U32 value) override;
    void IfLessThan2(JitWidth regWidth, RegPtr reg1, RegPtr reg2) override;
    void IfNot(JitWidth regWidth, RegPtr reg) override;
    void IfNotCPU(JitWidth regWidth, RegPtr sib, U8 lsl, U32 offset) override;    
    void IfFlagSet(U32 flags) override;
    void IfSmallStack() override;
    void JumpIfCondition(JitConditional condition, U32 address) override;
    void IfCompareReg(JitWidth regWidth, RegPtr reg1, RegPtr reg2, JitEvaluate condition) override;
    U32 MarkJumpLocation() override;
    void Goto(U32 location) override;
    void jmp(RegPtr reg) override;
    void updateFlagsIfNecessary();    
    RegPtr getReadOnlyFlags() override;
    void setFlags(RegPtr flags, U32 mask) override;

    void callHostFunction(void* address, const std::vector<DynParam>& params, bool restoreCache = true) override;
    void callHostFunctionWithResult(RegPtr result, void* address, const std::vector<DynParam>& params) override;
#ifdef BOXEDWINE_64
    void setParam(X86Asm::Reg reg, const DynParam& param);
#else
    void pushParam(const DynParam& param);
#endif

#ifdef BOXEDWINE_64
    std::array<bool, 16> regUsed2{ 0 };
#else
    std::array<bool, 8> regUsed2{ 0 };
#endif

    void incrementEip(U32 inc) override;
    void JumpInBlock(U32 address) override;
    void StartElse() override;
    void EndIf() override;
    void blockExit(bool syncCache = true) override;

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
    void updateHardwareFlags(U32 flags);
    void dynamic_rdtsc(DecodedOp* op) override;
    void dynamic_arithE32R32_lock(DecodedOp* op, std::function<void(RegPtr dest, RegPtr address, RegPtr offset)> callback, bool writeReg = false);
    void dynamic_arithE16R16_lock(DecodedOp* op, std::function<void(RegPtr dest, RegPtr address, RegPtr offset)> callback, bool writeReg = false);
    void dynamic_arithE8R8_lock(DecodedOp* op, std::function<void(RegPtr dest, RegPtr address, RegPtr offset)> callback, bool writeReg = false);
    void dynamic_arithE32_lock(DecodedOp* op, std::function<void(RegPtr address, RegPtr offset)> callback);
    void dynamic_arithE16_lock(DecodedOp* op, std::function<void(RegPtr address, RegPtr offset)> callback);
    void dynamic_arithE8_lock(DecodedOp* op, std::function<void(RegPtr address, RegPtr offset)> callback);

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

    void setCC(X86Asm::Reg32 reg, JitEvaluate condition);

    void preCommitJIT() override;
    void patch(U8* begin) override;
    U8* createStartJITCode() override;

    void loadCache();
    void writeCache();
    void setParams(const std::vector<DynParam>& params);
    U8* createSyncToHost() override;
    U8* createSyncFromHost() override;

    X86Asm x86;
#ifdef BOXEDWINE_64
    X86Asm::Reg64 params[4];
#endif
};

void JitX86CodeGen::preOp(DecodedOp* op) {
    regUsed2.fill(false);
    currentOp = op;
}

void JitX86CodeGen::emulateSingleOp() {
#ifdef BOXEDWINE_64
    x86.mov(x86.r8, (DYN_PTR_SIZE)cpu->thread->process->emulateSingleOp);
    x86.jmp(x86.r8);
#else
    x86.mov(x86.eax, (U32)cpu->thread->process->emulateSingleOp);
    x86.jmp(x86.eax);
#endif
}

U8 JitX86CodeGen::findTmpReg(bool needs8bitReg, S8 hint) {        
#ifdef BOXEDWINE_64
    if (!regUsed2[8]) {
        regUsed2[8] = true;
        return 8;
    }
    if (!regUsed2[9]) {
        regUsed2[9] = true;
        return 9;
    }
    if (!regUsed2[11]) {
        regUsed2[11] = true;
        return 11;
    }
    if (!regUsed2[12]) {
        regUsed2[12] = true;
        return 12;
    }
    if (!regUsed2[14]) {
        regUsed2[14] = true;
        return 14;
    }
    kpanic("JitX86CodeGen::getTmpReg ran out of tmp regs");
    return 0xff;
#else
    if (hint >= 0 && !regUsed2[hint]) {
        regUsed2[hint] = true;
        return (U8)hint;
    }
    U8 tmpReg = 0xff;
    if (!needs8bitReg && !regUsed2[6]) {
        regUsed2[6] = true;
        return 6;
    }
    for (int i = 3; i >= 0; i--) {
        if (!regUsed2[i]) {
            regUsed2[i] = true;
            tmpReg = i;
            break;
        }
    }
    if (tmpReg == 0xff) {
        kpanic("JitX86CodeGen::getTmpReg ran out of tmp regs");
    }
    return tmpReg;
#endif
}

RegPtr JitX86CodeGen::getTmpReg() {
    return getTmpRegWithHint(-1);
}

RegPtr JitX86CodeGen::getTmpRegWithHint(S8 hint) {
    return std::shared_ptr<JitReg>(new JitReg(findTmpReg(false, hint), 0xff), [this](JitReg* p) {
        if (p->isLoaded()) {
            regUsed2[p->hardwareReg()] = false;
        }
        delete p;
    });
}

RegPtr JitX86CodeGen::getTmpReg8() {
    return std::shared_ptr<JitReg>(new JitReg(findTmpReg(true), 0xff), [this](JitReg* p) {
        if (p->isLoaded()) {
            regUsed2[p->hardwareReg()] = false;
        }
        delete p;
    });
}

RegPtr JitX86CodeGen::getTmpReg(U8 reg, bool delayed, S8 hint) {
    if (delayed) {
        auto getTmp = [reg, hint, this]() {
            U8 hardwareReg = findTmpReg(false, hint);
            if (regCache[reg] != INVALID_REG) {
                x86.mov(R32(hardwareReg), R32(regCache[reg]));
            } else {
                x86.mov(R32(hardwareReg), X86Asm::Mem32(HOST_CPU, CPU::offsetofReg32(reg)));
            }
            return hardwareReg;
        };

        return std::shared_ptr<JitReg>(new JitReg(0xff, 0xff, getTmp), [this](JitReg* p) {
            if (p->isLoaded()) {
                regUsed2[p->hardwareReg()] = false;
            }
            delete p;
        });
    } else {
        RegPtr result = std::shared_ptr<JitReg>(new JitReg(findTmpReg(false, hint), 0xff), [this](JitReg* p) {
            if (p->isLoaded()) {
                regUsed2[p->hardwareReg()] = false;
            }
            delete p;
        });
        if (regCache[reg] != INVALID_REG) {
            x86.mov(R32(result->hardwareReg()), R32(regCache[reg]));
        } else {
            x86.mov(R32(result->hardwareReg()), X86Asm::Mem32(HOST_CPU, CPU::offsetofReg32(reg)));
        }
        return result;
    }
}

RegPtr JitX86CodeGen::getTmpRegForCallResult() {
#ifdef BOXEDWINE_64
    return getTmpReg();
#else
    U8 reg;

    if (regUsed2[0]) {
        reg = findTmpReg(true);
    } else {
        reg = 0;
        regUsed2[0] = true;
    }
    return std::shared_ptr<JitReg>(new JitReg(reg, 0xff), [this](JitReg* p) {
        if (p->isLoaded()) {
            regUsed2[p->hardwareReg()] = false;
        }
        delete p;
    });
#endif
}

RegPtr JitX86CodeGen::getTmpSegAddress(U8 reg) {
    return getReadOnlySegAddress(reg);
}

RegPtr JitX86CodeGen::getReadOnlySegAddress(U8 reg) {
    RegPtr result = getTmpReg();
    x86.mov(R32(result->hardwareReg()), X86Asm::Mem32(HOST_CPU, CPU::offsetofSegAddress(reg)));
    return result;
}

RegPtr JitX86CodeGen::getReadOnlySegValue(U8 reg) {
    RegPtr result = getTmpReg();
    x86.mov(R32(result->hardwareReg()), X86Asm::Mem32(HOST_CPU, CPU::offsetofSegValue(reg)));
    return result;
}

RegPtr JitX86CodeGen::getTmpReg8(U8 reg, bool delayed, S8 hint) {
    U8 originalReg = reg;
    bool isHigh = reg > 3;
    RegPtr result;

    if (isHigh) {
        reg -= 4;
    }
    if (delayed) {
        auto getTmp = [originalReg, isHigh, reg, hint, this]() {
            U8 hardwareReg = findTmpReg(true, hint);
            if (regCache[reg] != INVALID_REG) {
                if (isHigh) {
                    if (regCache[reg] < 4 && hardwareReg < 4) {
                        x86.mov(R8(hardwareReg), R8(regCache[reg] + 4));
                    } else {
                        x86.mov(R32(hardwareReg), R32(regCache[reg]));
                        x86.shr(R16(hardwareReg), 8);
                    }
                } else {
                    x86.mov(R32(hardwareReg), R32(regCache[reg]));
                }
            } else {
                x86.mov(R8(hardwareReg), X86Asm::Mem8(HOST_CPU, CPU::offsetofReg8(originalReg)));
            }
            return hardwareReg;
        };

        result = std::shared_ptr<JitReg>(new JitReg(0xff, 0xff, false, getTmp), [this](JitReg* p) {
            if (p->isLoaded()) {
                regUsed2[p->hardwareReg()] = false;
            }
            delete p;
        });
    } else {
        result = std::shared_ptr<JitReg>(new JitReg(findTmpReg(true, hint), 0xff, false), [this](JitReg* p) {
            if (p->isLoaded()) {
                regUsed2[p->hardwareReg()] = false;
            }
            delete p;
        });
        if (regCache[reg] != INVALID_REG) {
            if (isHigh) {
                if (regCache[reg] < 4 && result->hardwareReg() < 4) {
                    x86.mov(R8(result->hardwareReg()), R8(regCache[reg] + 4));
                } else {
                    x86.mov(R32(result->hardwareReg()), R32(regCache[reg]));
                    x86.shr(R16(result->hardwareReg()), 8);
                }
            } else {
                x86.mov(R32(result->hardwareReg()), R32(regCache[reg]));
            }
        } else {
            x86.mov(R8(result->hardwareReg()), X86Asm::Mem8(HOST_CPU, CPU::offsetofReg8(originalReg)));
        }
    }
    return result;
}

RegPtr JitX86CodeGen::getTmpEip() {
    RegPtr result = getTmpReg();
    x86.mov(R32(result->hardwareReg()), X86Asm::Mem32(HOST_CPU, offsetof(CPU, eip.u32)));
    return result;
}

void JitX86CodeGen::pushReg(RegPtr reg) {
    x86.push(RN(reg->hardwareReg()));
}

void JitX86CodeGen::popReg(RegPtr reg) {
    x86.pop(RN(reg->hardwareReg()));
}

bool JitX86CodeGen::isTmpRegAvailable() {
    for (int i = 0; i < 4; i++) {
        if (!regUsed2[i]) {
            return true;
        }
    }
    return false;
}

RegPtr JitX86CodeGen::getEip(bool load) {
    RegPtr result = std::shared_ptr<JitReg>(new JitReg(findTmpReg(false), 0xff), [this](JitReg* p) {
        x86.mov(X86Asm::Mem32(HOST_CPU, offsetof(CPU, eip.u32)), R32(p->hardwareReg()));
        regUsed2[p->hardwareReg()] = false;
        delete p;
    });
    if (load) {
        x86.mov(R32(result->hardwareReg()), X86Asm::Mem32(HOST_CPU, offsetof(CPU, eip.u32)));
    }
    return result;
}

void JitX86CodeGen::forceSyncBackIfNotCached(RegPtr reg) {
    if (reg->emulatedReg != 0xff && regCache[reg->emulatedReg] == INVALID_REG) {
        x86.mov(X86Asm::Mem32(HOST_CPU, CPU::offsetofReg32(reg->emulatedReg)), R32(reg->hardwareReg()));
    }
}

RegPtr JitX86CodeGen::getReg(U8 reg, S8 hint, bool load) {
    if (regCache[reg] != INVALID_REG) {
        if (hint >= 0 && !regUsed2[hint] && (S8)regCache[reg] != hint) {
            regUsed2[hint] = true;
            RegPtr result = std::shared_ptr<JitReg>(new JitReg(hint, reg), [reg, this](JitReg* p) {
                x86.mov(R32(regCache[reg]), R32(p->hardwareReg()));
                regUsed2[p->hardwareReg()] = false;
                delete p;
            });
            if (load) {
                x86.mov(R32(result->hardwareReg()), R32(regCache[reg]));
            }
            return result;
        }
        return std::shared_ptr<JitReg>(new JitReg(regCache[reg], reg), [this](JitReg* p) {
            if (p->isLoaded()) {
                regUsed2[p->hardwareReg()] = false;
            }
            delete p;
        });
    } else {
        RegPtr result = std::shared_ptr<JitReg>(new JitReg(findTmpReg(false, hint), reg), [this](JitReg* p) {
            x86.mov(X86Asm::Mem32(HOST_CPU, CPU::offsetofReg32(p->emulatedReg)), R32(p->hardwareReg()));
            regUsed2[p->hardwareReg()] = false;
            delete p;
        });
        if (load) {
            x86.mov(R32(result->hardwareReg()), X86Asm::Mem32(HOST_CPU, CPU::offsetofReg32(reg)));
        }
        return result;
    }
}

RegPtr JitX86CodeGen::getReg8(U8 reg) {
    bool isHigh = reg > 3;
    if (isHigh) {
        reg -= 4;
#ifdef BOXEDWINE_64
        // on x64, if a rex prefix is used, 4 will map to spl (low byte of rsp) instead of AH, etc
        // because of that we need to copy AH/CH/DH/BH into a tmp and sync back
        RegPtr result = std::shared_ptr<JitReg>(new JitReg(findTmpReg(true), reg, isHigh), [this](JitReg* p) {
            if (p->isLoaded()) {
                if (regCache[p->emulatedReg] != INVALID_REG) {
                    if (p->hardwareReg() > 3 || regCache[p->emulatedReg] > 3) {
                        x86.and_(R32(regCache[p->emulatedReg]), 0xffff00ff);
                        x86.and_(R32(p->hardwareReg()), 0xff);
                        x86.shl(R32(p->hardwareReg()), 8);
                        x86.or_(R32(regCache[p->emulatedReg]), R32(p->hardwareReg()));
                    } else {
                        x86.mov(R8(regCache[p->emulatedReg] + 4), R8(p->hardwareReg()));
                    }
                } else {
                    x86.mov(X86Asm::Mem8(HOST_CPU, CPU::offsetofReg32(p->emulatedReg) + 1), R8(p->hardwareReg()));
                }
                regUsed2[p->hardwareReg()] = false;
            }
            delete p;
            });
        if (regCache[reg] != INVALID_REG) {
            if (result->hardwareReg() > 3 || regCache[reg] > 3) {
                x86.mov(R32(result->hardwareReg()), R32(regCache[reg]));
                x86.shr(R16(result->hardwareReg()), 8);
            } else {
                x86.mov(R8(result->hardwareReg()), R8(regCache[reg] + 4));
            }
        } else {
            x86.mov(R8(result->hardwareReg()), X86Asm::Mem8(HOST_CPU, CPU::offsetofReg32(reg) + 1));
        }
        return result;
#endif
    }
    if (regCache[reg] < 4) {
        return getReg(reg);
    }
    // for 8-bit, only the first 4 registers in x86 can use them
    RegPtr result = std::shared_ptr<JitReg>(new JitReg(findTmpReg(true), reg, isHigh), [this](JitReg* p) {
        if (p->isLoaded()) {
            if (regCache[p->emulatedReg] != INVALID_REG) {
                x86.mov(R32(regCache[p->emulatedReg]), R32(p->hardwareReg()));
            } else {
                x86.mov(X86Asm::Mem32(HOST_CPU, CPU::offsetofReg32(p->emulatedReg)), R32(p->hardwareReg()));
            }
            regUsed2[p->hardwareReg()] = false;
        }
        delete p;
    });
    if (regCache[reg] != INVALID_REG) {
        x86.mov(R32(result->hardwareReg()), R32(regCache[reg]));
    } else {
        x86.mov(R32(result->hardwareReg()), X86Asm::Mem32(HOST_CPU, CPU::offsetofReg32(reg)));
    }
    return result;
}

RegPtr JitX86CodeGen::getReadOnlyReg(U8 reg, bool delayed, S8 hint) {
    if (regCache[reg] != INVALID_REG) {
        return std::shared_ptr<JitReg>(new JitReg(regCache[reg], reg), [this](JitReg* p) {
            if (p->isLoaded()) {
                regUsed2[p->hardwareReg()] = false;
            }
            delete p;
        });
    } else {
        return getTmpReg(reg, delayed, hint);
    }
}

RegPtr JitX86CodeGen::getReadOnlyReg8(U8 reg, bool delayed, S8 hint) {
    if (hint >= 0 && hint < 4 && regCache[reg] != INVALID_REG && regCache[reg] < 4) {
        return std::shared_ptr<JitReg>(new JitReg(regCache[reg], reg), [this](JitReg* p) {
            if (p->isLoaded()) {
                regUsed2[p->hardwareReg()] = false;
            }
            delete p;
        });
    }
    return getTmpReg8(reg, delayed, hint);
}

static U8 get8bitReg(RegPtr reg) {
#ifndef BOXEDWINE_64
    if (reg->isHigh) {
        return reg->hardwareReg() + 4;
    }
    if (reg->hardwareReg() > 3) {
        kpanic("get8bitReg failed");
    }
#endif
    return reg->hardwareReg();
}

void JitX86CodeGen::addReg(JitWidth regWidth, RegPtr reg, RegPtr rm) {
    if (regWidth == JitWidth::b32) {
        x86.add(R32(reg->hardwareReg()), R32(rm->hardwareReg()));
    } else if (regWidth == JitWidth::b16) {
        x86.add(R16(reg->hardwareReg()), R16(rm->hardwareReg()));
    } else if (regWidth == JitWidth::b8) {
        x86.add(R8(get8bitReg(reg)), R8(get8bitReg(rm)));
    } else {
        kpanic("JitX86CodeGen::addReg");
    }
}

void JitX86CodeGen::addValue(JitWidth regWidth, RegPtr reg, U32 imm) {
    if (regWidth == JitWidth::b32) {
        x86.add(R32(reg->hardwareReg()), imm);
    } else if (regWidth == JitWidth::b16) {
        x86.add(R16(reg->hardwareReg()), (U16)imm);
    } else if (regWidth == JitWidth::b8) {
        x86.add(R8(get8bitReg(reg)), (U8)imm);
    } else {
        kpanic("JitX86CodeGen::addValue");
    }
}

void JitX86CodeGen::orReg(JitWidth regWidth, RegPtr reg, RegPtr rm) {
    if (regWidth == JitWidth::b32) {
        x86.or_(R32(reg->hardwareReg()), R32(rm->hardwareReg()));
    } else if (regWidth == JitWidth::b16) {
        x86.or_(R16(reg->hardwareReg()), R16(rm->hardwareReg()));
    } else if (regWidth == JitWidth::b8) {
        x86.or_(R8(get8bitReg(reg)), R8(get8bitReg(rm)));
    } else {
        kpanic("JitX86CodeGen::orReg");
    }
}

void JitX86CodeGen::orValue(JitWidth regWidth, RegPtr reg, U32 imm) {
    if (regWidth == JitWidth::b32) {
        x86.or_(R32(reg->hardwareReg()), imm);
    } else if (regWidth == JitWidth::b16) {
        x86.or_(R16(reg->hardwareReg()), (U16)imm);
    } else if (regWidth == JitWidth::b8) {
        x86.or_(R8(get8bitReg(reg)), (U8)imm);
    } else {
        kpanic("JitX86CodeGen::orValue");
    }
}

void JitX86CodeGen::subReg(JitWidth regWidth, RegPtr reg, RegPtr rm) {
    if (regWidth == JitWidth::b32) {
        x86.sub(R32(reg->hardwareReg()), R32(rm->hardwareReg()));
    } else if (regWidth == JitWidth::b16) {
        x86.sub(R16(reg->hardwareReg()), R16(rm->hardwareReg()));
    } else if (regWidth == JitWidth::b8) {
        x86.sub(R8(get8bitReg(reg)), R8(get8bitReg(rm)));
    } else {
        kpanic("JitX86CodeGen::subReg");
    }
}

void JitX86CodeGen::subValue(JitWidth regWidth, RegPtr reg, U32 imm) {
    if (regWidth == JitWidth::b32) {
        x86.sub(R32(reg->hardwareReg()), imm);
    } else if (regWidth == JitWidth::b16) {
        x86.sub(R16(reg->hardwareReg()), (U16)imm);
    } else if (regWidth == JitWidth::b8) {
        x86.sub(R8(get8bitReg(reg)), (U8)imm);
    } else {
        kpanic("JitX86CodeGen::subValue");
    }
}

void JitX86CodeGen::andReg(JitWidth regWidth, RegPtr reg, RegPtr rm) {
    if (regWidth == JitWidth::b32) {
        x86.and_(R32(reg->hardwareReg()), R32(rm->hardwareReg()));
    } else if (regWidth == JitWidth::b16) {
        x86.and_(R16(reg->hardwareReg()), R16(rm->hardwareReg()));
    } else if (regWidth == JitWidth::b8) {
        x86.and_(R8(get8bitReg(reg)), R8(get8bitReg(rm)));
    } else {
        kpanic("JitX86CodeGen::andReg");
    }
}

void JitX86CodeGen::andValue(JitWidth regWidth, RegPtr reg, U32 imm) {
    if (regWidth == JitWidth::b32) {
        x86.and_(R32(reg->hardwareReg()), imm);
    } else if (regWidth == JitWidth::b16) {
        x86.and_(R16(reg->hardwareReg()), (U16)imm);
    } else if (regWidth == JitWidth::b8) {
        x86.and_(R8(get8bitReg(reg)), (U8)imm);
    } else {
        kpanic("JitX86CodeGen::andValue");
    }
}

void JitX86CodeGen::xorReg(JitWidth regWidth, RegPtr reg, RegPtr rm) {
    if (regWidth == JitWidth::b32) {
        x86.xor_(R32(reg->hardwareReg()), R32(rm->hardwareReg()));
    } else if (regWidth == JitWidth::b16) {
        x86.xor_(R16(reg->hardwareReg()), R16(rm->hardwareReg()));
    } else if (regWidth == JitWidth::b8) {
        x86.xor_(R8(get8bitReg(reg)), R8(get8bitReg(rm)));
    } else {
        kpanic("JitX86CodeGen::xorReg");
    }
}

void JitX86CodeGen::xorValue(JitWidth regWidth, RegPtr reg, U32 imm) {
    if (regWidth == JitWidth::b32) {
        x86.xor_(R32(reg->hardwareReg()), imm);
    } else if (regWidth == JitWidth::b16) {
        x86.xor_(R16(reg->hardwareReg()), (U16)imm);
    } else if (regWidth == JitWidth::b8) {
        x86.xor_(R8(get8bitReg(reg)), (U8)imm);
    } else {
        kpanic("JitX86CodeGen::xorValue");
    }
}

#ifdef BOXEDWINE_64
static bool hasBMI2 = platformHasBMI2();
#endif

void JitX86CodeGen::shrReg(JitWidth regWidth, RegPtr reg, RegPtr rm) {
#ifdef BOXEDWINE_64
    if (hasBMI2) {
        if (regWidth == JitWidth::b32 && rm->hardwareReg() != 1) {
            x86.shrx(R32(reg->hardwareReg()), R32(rm->hardwareReg()));
            return;
        }
    }
#endif
    if (rm->hardwareReg() != 1) {
        x86.push(RN(1));
        if (reg->hardwareReg() == 1) {
            kpanic("JitX86CodeGen::shlReg 1");
        }
    }
    if (regWidth == JitWidth::b32) {
        if (rm->hardwareReg() != 1) {
            x86.mov(x86.ecx, R32(rm->hardwareReg()));
        }
        x86.shr(R32(reg->hardwareReg()), x86.ecx);
    } else if (regWidth == JitWidth::b16) {
        if (rm->hardwareReg() != 1) {
            x86.mov(x86.cx, R16(rm->hardwareReg()));
        }
        x86.shr(R16(reg->hardwareReg()), x86.cx);
    } else if (regWidth == JitWidth::b8) {
        if (rm->hardwareReg() != 1) {
            x86.mov(x86.cl, R8(get8bitReg(rm)));
        }
        x86.shr(R8(get8bitReg(reg)), x86.cl);
    } else {
        kpanic("JitX86CodeGen::shrReg");
    }
    if (rm->hardwareReg() != 1) {
        x86.pop(RN(1));
    }
}

void JitX86CodeGen::shrValue(JitWidth regWidth, RegPtr reg, U32 imm) {
    if (regWidth == JitWidth::b32) {
        x86.shr(R32(reg->hardwareReg()), imm);
    } else if (regWidth == JitWidth::b16) {
        x86.shr(R16(reg->hardwareReg()), (U16)imm);
    } else if (regWidth == JitWidth::b8) {
        x86.shr(R8(get8bitReg(reg)), (U8)imm);
    } else {
        kpanic("JitX86CodeGen::shrValue");
    }
}

void JitX86CodeGen::shlReg(JitWidth regWidth, RegPtr reg, RegPtr rm) {
#ifdef BOXEDWINE_64
    if (hasBMI2) {
        if (regWidth == JitWidth::b32 && rm->hardwareReg() != 1) {
            x86.shlx(R32(reg->hardwareReg()), R32(rm->hardwareReg()));
            return;
        }
    }
#endif
    if (rm->hardwareReg() != 1) {
        x86.push(RN(1));
        if (reg->hardwareReg() == 1) {
            kpanic("JitX86CodeGen::shlReg 1");
        }
    }
    if (regWidth == JitWidth::b32) {
        if (rm->hardwareReg() != 1) {
            x86.mov(x86.ecx, R32(rm->hardwareReg()));
        }
        x86.shl(R32(reg->hardwareReg()), x86.ecx);
    } else if (regWidth == JitWidth::b16) {
        if (rm->hardwareReg() != 1) {
            x86.mov(x86.cx, R16(rm->hardwareReg()));
        }
        x86.shl(R16(reg->hardwareReg()), x86.cx);
    } else if (regWidth == JitWidth::b8) {
        if (rm->hardwareReg() != 1) {
            x86.mov(x86.cl, R8(get8bitReg(rm)));
        }
        x86.shl(R8(get8bitReg(reg)), x86.cl);
    } else {
        kpanic("JitX86CodeGen::shlReg");
    }
    if (rm->hardwareReg() != 1) {
        x86.pop(RN(1));
    }
}

void JitX86CodeGen::shlValue(JitWidth regWidth, RegPtr reg, U32 imm) {
    if (regWidth == JitWidth::b32) {
        x86.shl(R32(reg->hardwareReg()), imm);
    } else if (regWidth == JitWidth::b16) {
        x86.shl(R16(reg->hardwareReg()), (U16)imm);
    } else if (regWidth == JitWidth::b8) {
        x86.shl(R8(get8bitReg(reg)), (U8)imm);
    } else {
        kpanic("JitX86CodeGen::shlValue");
    }
}

void JitX86CodeGen::sarReg(JitWidth regWidth, RegPtr reg, RegPtr rm) {
#ifdef BOXEDWINE_64
    if (hasBMI2) {
        if (regWidth == JitWidth::b32 && rm->hardwareReg() != 1) {
            x86.sarx(R32(reg->hardwareReg()), R32(rm->hardwareReg()));
            return;
        }
    }
#endif
    if (rm->hardwareReg() != 1) {
        x86.push(RN(1));
        if (reg->hardwareReg() == 1) {
            kpanic("JitX86CodeGen::shlReg 1");
        }
    }
    if (regWidth == JitWidth::b32) {
        if (rm->hardwareReg() != 1) {
            x86.mov(x86.ecx, R32(rm->hardwareReg()));
        }
        x86.sar(R32(reg->hardwareReg()), x86.ecx);
    } else if (regWidth == JitWidth::b16) {
        if (rm->hardwareReg() != 1) {
            x86.mov(x86.cx, R16(rm->hardwareReg()));
        }
        x86.sar(R16(reg->hardwareReg()), x86.cx);
    } else if (regWidth == JitWidth::b8) {
        if (rm->hardwareReg() != 1) {
            x86.mov(x86.cl, R8(get8bitReg(rm)));
        }
        x86.sar(R8(get8bitReg(reg)), x86.cl);
    } else {
        kpanic("JitX86CodeGen::sarReg");
    }
    if (rm->hardwareReg() != 1) {
        x86.pop(RN(1));
    }
}

void JitX86CodeGen::sarValue(JitWidth regWidth, RegPtr reg, U32 imm) {
    if (regWidth == JitWidth::b32) {
        x86.sar(R32(reg->hardwareReg()), imm);
    } else if (regWidth == JitWidth::b16) {
        x86.sar(R16(reg->hardwareReg()), (U16)imm);
    } else if (regWidth == JitWidth::b8) {
        x86.sar(R8(get8bitReg(reg)), (U8)imm);
    } else {
        kpanic("JitX86CodeGen::sarValue");
    }
}

void JitX86CodeGen::notReg2(JitWidth regWidth, RegPtr reg) {
    if (regWidth == JitWidth::b32) {
        x86.not_(R32(reg->hardwareReg()));
    } else if (regWidth == JitWidth::b16) {
        x86.not_(R16(reg->hardwareReg()));
    } else if (regWidth == JitWidth::b8) {
        x86.not_(R8(get8bitReg(reg)));
    } else {
        kpanic("JitX86CodeGen::notReg");
    }
    // not doesn't set flags
}

void JitX86CodeGen::negReg2(JitWidth regWidth, RegPtr reg) {
    if (regWidth == JitWidth::b32) {
        x86.neg(R32(reg->hardwareReg()));
    } else if (regWidth == JitWidth::b16) {
        x86.neg(R16(reg->hardwareReg()));
    } else if (regWidth == JitWidth::b8) {
        x86.neg(R8(get8bitReg(reg)));
    } else {
        kpanic("JitX86CodeGen::negReg");
    }
}

void JitX86CodeGen::bsfReg(JitWidth regWidth, RegPtr reg, RegPtr rm) {
    if (regWidth == JitWidth::b32) {
        x86.bsf(R32(reg->hardwareReg()), R32(rm->hardwareReg()));
    } else if (regWidth == JitWidth::b16) {
        x86.bsf(R16(reg->hardwareReg()), R16(rm->hardwareReg()));
    } else {
        kpanic("JitX86CodeGen::bsfReg");
    }
}

void JitX86CodeGen::bsrReg(JitWidth regWidth, RegPtr reg, RegPtr rm) {
    if (regWidth == JitWidth::b32) {
        x86.bsr(R32(reg->hardwareReg()), R32(rm->hardwareReg()));
    } else if (regWidth == JitWidth::b16) {
        x86.bsr(R16(reg->hardwareReg()), R16(rm->hardwareReg()));
    } else {
        kpanic("JitX86CodeGen::bsrReg");
    }
}

void JitX86CodeGen::rolReg(JitWidth regWidth, RegPtr reg, RegPtr rm) {
    if (regWidth == JitWidth::b32) {
        x86.rol(R32(reg->hardwareReg()), R32(rm->hardwareReg()));
    } else if (regWidth == JitWidth::b16) {
        x86.rol(R16(reg->hardwareReg()), R16(rm->hardwareReg()));
    } else if (regWidth == JitWidth::b8) {
        x86.rol(R8(get8bitReg(reg)), R8(get8bitReg(rm)));
    } else {
        kpanic("JitX86CodeGen::rolReg");
    }
}

void JitX86CodeGen::rolValue(JitWidth regWidth, RegPtr reg, U32 imm) {
    if (regWidth == JitWidth::b32) {
        x86.rol(R32(reg->hardwareReg()), imm);
    } else if (regWidth == JitWidth::b16) {
        x86.rol(R16(reg->hardwareReg()), (U16)imm);
    } else if (regWidth == JitWidth::b8) {
        x86.rol(R8(get8bitReg(reg)), (U8)imm);
    } else {
        kpanic("JitX86CodeGen::rolValue");
    }
}

void JitX86CodeGen::rorReg(JitWidth regWidth, RegPtr reg, RegPtr rm) {
    if (regWidth == JitWidth::b32) {
        x86.ror(R32(reg->hardwareReg()), R32(rm->hardwareReg()));
    } else if (regWidth == JitWidth::b16) {
        x86.ror(R16(reg->hardwareReg()), R16(rm->hardwareReg()));
    } else if (regWidth == JitWidth::b8) {
        x86.ror(R8(get8bitReg(reg)), R8(get8bitReg(rm)));
    } else {
        kpanic("JitX86CodeGen::rorReg");
    }
}

void JitX86CodeGen::rorValue(JitWidth regWidth, RegPtr reg, U32 imm) {
    if (regWidth == JitWidth::b32) {
        x86.ror(R32(reg->hardwareReg()), imm);
    } else if (regWidth == JitWidth::b16) {
        x86.ror(R16(reg->hardwareReg()), (U16)imm);
    } else if (regWidth == JitWidth::b8) {
        x86.ror(R8(get8bitReg(reg)), (U8)imm);
    } else {
        kpanic("JitX86CodeGen::rorValue");
    }
}

void JitX86CodeGen::rclReg(JitWidth regWidth, RegPtr reg, RegPtr rm) {
    // shr will move the bottom bit into the carry flag
    x86.shr(R32(getCF()->hardwareReg()), 1);

    if (regWidth == JitWidth::b32) {
        x86.rcl(R32(reg->hardwareReg()), R32(rm->hardwareReg()));
    } else if (regWidth == JitWidth::b16) {
        x86.rcl(R16(reg->hardwareReg()), R16(rm->hardwareReg()));
    } else if (regWidth == JitWidth::b8) {
        x86.rcl(R8(get8bitReg(reg)), R8(get8bitReg(rm)));
    } else {
        kpanic("JitX86CodeGen::rclReg");
    }
}

void JitX86CodeGen::rclValue(JitWidth regWidth, RegPtr reg, U32 imm) {
    // shr will move the bottom bit into the carry flag
    x86.shr(R32(getCF()->hardwareReg()), 1);

    if (regWidth == JitWidth::b32) {
        x86.rcl(R32(reg->hardwareReg()), imm);
    } else if (regWidth == JitWidth::b16) {
        x86.rcl(R16(reg->hardwareReg()), (U16)imm);
    } else if (regWidth == JitWidth::b8) {
        x86.rcl(R8(get8bitReg(reg)), (U8)imm);
    } else {
        kpanic("JitX86CodeGen::rclValue");
    }
}

void JitX86CodeGen::rcrReg(JitWidth regWidth, RegPtr reg, RegPtr rm) {
    // shr will move the bottom bit into the carry flag
    x86.shr(R32(getCF()->hardwareReg()), 1);

    if (regWidth == JitWidth::b32) {
        x86.rcr(R32(reg->hardwareReg()), R32(rm->hardwareReg()));
    } else if (regWidth == JitWidth::b16) {
        x86.rcr(R16(reg->hardwareReg()), R16(rm->hardwareReg()));
    } else if (regWidth == JitWidth::b8) {
        x86.rcr(R8(get8bitReg(reg)), R8(get8bitReg(rm)));
    } else {
        kpanic("JitX86CodeGen::rcrReg");
    }
}

void JitX86CodeGen::rcrValue(JitWidth regWidth, RegPtr reg, U32 imm) {
    // shr will move the bottom bit into the carry flag
    x86.shr(R32(getCF()->hardwareReg()), 1);

    if (regWidth == JitWidth::b32) {
        x86.rcr(R32(reg->hardwareReg()), imm);
    } else if (regWidth == JitWidth::b16) {
        x86.rcr(R16(reg->hardwareReg()), (U16)imm);
    } else if (regWidth == JitWidth::b8) {
        x86.rcr(R8(get8bitReg(reg)), (U8)imm);
    } else {
        kpanic("JitX86CodeGen::rcrValue");
    }
}

void JitX86CodeGen::shldReg(JitWidth regWidth, RegPtr reg, RegPtr rm, RegPtr cl) {
    if (rm->hardwareReg() != 1) {
        x86.push(RN(1));
    }
    if (reg->hardwareReg() == 1 || rm->hardwareReg() == 1) {
        kpanic("JitX86CodeGen::shldReg cl");
    }
    if (regWidth == JitWidth::b32) {
        if (rm->hardwareReg() != 1) {
            x86.mov(x86.ecx, R32(cl->hardwareReg()));
        }
        x86.shld(R32(reg->hardwareReg()), R32(rm->hardwareReg()), x86.ecx);
    } else if (regWidth == JitWidth::b16) {
        if (rm->hardwareReg() != 1) {
            x86.mov(x86.cx, R16(cl->hardwareReg()));
        }
        x86.shld(R16(reg->hardwareReg()), R16(rm->hardwareReg()), x86.cx);
    } else {
        kpanic("JitX86CodeGen::shldReg");
    }
    if (rm->hardwareReg() != 1) {
        x86.pop(RN(1));
    }
}

void JitX86CodeGen::shldValue(JitWidth regWidth, RegPtr reg, RegPtr rm, U32 imm) {
    // don't need to check if imm is 0, that was handled in the decoder, if it was 0, the decoder will replace shld with nop

    if (regWidth == JitWidth::b32) {
        x86.shld(R32(reg->hardwareReg()), R32(rm->hardwareReg()), imm);
    } else if (regWidth == JitWidth::b16) {
        x86.shld(R16(reg->hardwareReg()), R16(rm->hardwareReg()), (U16)imm);
    } else {
        kpanic("JitX86CodeGen::shldReg");
    }
}

void JitX86CodeGen::shrdReg(JitWidth regWidth, RegPtr reg, RegPtr rm, RegPtr cl) {
    if (rm->hardwareReg() != 1) {
        x86.push(RN(1));
    }
    if (reg->hardwareReg() == 1 || rm->hardwareReg() == 1) {
        kpanic("JitX86CodeGen::shrdReg cl");
    }
    if (regWidth == JitWidth::b32) {
        if (rm->hardwareReg() != 1) {
            x86.mov(x86.ecx, R32(cl->hardwareReg()));
        }
        x86.shrd(R32(reg->hardwareReg()), R32(rm->hardwareReg()), x86.ecx);
    } else if (regWidth == JitWidth::b16) {
        if (rm->hardwareReg() != 1) {
            x86.mov(x86.cx, R16(cl->hardwareReg()));
        }
        x86.shrd(R16(reg->hardwareReg()), R16(rm->hardwareReg()), x86.cx);
    } else {
        kpanic("JitX86CodeGen::shrdReg");
    }
    if (rm->hardwareReg() != 1) {
        x86.pop(RN(1));
    }
}

void JitX86CodeGen::shrdValue(JitWidth regWidth, RegPtr reg, RegPtr rm, U32 imm) {
    // don't need to check if imm is 0, that was handled in the decoder, if it was 0, the decoder will replace shrd with nop

    if (regWidth == JitWidth::b32) {
        x86.shrd(R32(reg->hardwareReg()), R32(rm->hardwareReg()), imm);
    } else if (regWidth == JitWidth::b16) {
        x86.shrd(R16(reg->hardwareReg()), R16(rm->hardwareReg()), (U16)imm);
    } else {
        kpanic("JitX86CodeGen::shrdReg");
    }
}

void JitX86CodeGen::xchgReg(JitWidth regWidth, RegPtr dest, RegPtr src) {
    if (regWidth == JitWidth::b32) {
        x86.xchg(R32(dest->hardwareReg()), R32(src->hardwareReg()));
    } else if (regWidth == JitWidth::b16) {
        x86.xchg(R16(dest->hardwareReg()), R16(src->hardwareReg()));
    } else if (regWidth == JitWidth::b8) {
        x86.xchg(R8(get8bitReg(dest)), R8(get8bitReg(src)));
    } else {
        kpanic("JitX86CodeGen::xchgReg");
    }
}

void JitX86CodeGen::byteSwapReg32(RegPtr reg) {
    x86.bswap(R32(reg->hardwareReg()));
}

RegPtr JitX86CodeGen::compareReg(JitWidth regWidth, RegPtr reg1, RegPtr reg2, JitEvaluate condition, RegPtr result) {
    if (regWidth == JitWidth::b32) {
        x86.cmp(R32(reg1->hardwareReg()), R32(reg2->hardwareReg()));
    } else if (regWidth == JitWidth::b16) {
        x86.cmp(R16(reg1->hardwareReg()), R16(reg2->hardwareReg()));
    } else if (regWidth == JitWidth::b8) {
        x86.cmp(R8(get8bitReg(reg1)), R8(get8bitReg(reg2)));
    } else {
        kpanic_fmt("JitX86CodeGen::compareReg reg width %d", regWidth);
    }
    if (!result) {
        result = getTmpReg8();
    }
    setCC(R32(result->hardwareReg()), condition);
    x86.movzx(R32(result->hardwareReg()), R8(get8bitReg(result)));
    return result;
}

RegPtr JitX86CodeGen::compareValue(JitWidth regWidth, RegPtr reg, U32 value, JitEvaluate condition, RegPtr result) {
    if (regWidth == JitWidth::b32) {
        x86.cmp(R32(reg->hardwareReg()), value);
    } else if (regWidth == JitWidth::b16) {
        x86.cmp(R16(reg->hardwareReg()), (U16)value);
    } else if (regWidth == JitWidth::b8) {
        x86.cmp(R8(get8bitReg(reg)), (U8)value);
    } else {
        kpanic_fmt("JitX86CodeGen::compareValue reg width %d", regWidth);
    }
    if (!result) {
        result = getTmpReg8();
    }
    setCC(R32(result->hardwareReg()), condition);
    x86.movzx(R32(result->hardwareReg()), R8(get8bitReg(result)));
    return result;
}

void JitX86CodeGen::xaddReg(JitWidth regWidth, RegPtr reg, RegPtr rm) {
    if (regWidth == JitWidth::b32) {
        x86.xadd(R32(rm->hardwareReg()), R32(reg->hardwareReg()));
    } else if (regWidth == JitWidth::b16) {
        x86.xadd(R16(rm->hardwareReg()), R16(reg->hardwareReg()));
    } else if (regWidth == JitWidth::b8) {
        x86.xadd(R8(get8bitReg(rm)), R8(get8bitReg(reg)));
    } else {
        kpanic("JitX86CodeGen::xaddReg");
    }
}

void JitX86CodeGen::mulReg(JitWidth regWidth, RegPtr reg) {
    if (regWidth == JitWidth::b32) {
        // EDX:EAX = (U64)EAX * src;
        if (regUsed2[0] || regUsed2[2]) {
            kpanic("JitX86CodeGen::mulReg 32");
        }
        RegPtr eax = getReg(0);
        regUsed2[2] = true;
        regUsed2[0] = true;
        x86.mov(x86.eax, R32(eax->hardwareReg()));
        x86.mulEax(R32(reg->hardwareReg()));
        x86.mov(R32(eax->hardwareReg()), x86.eax);
        x86.mov(R32(getReg(2)->hardwareReg()), x86.edx);
        regUsed2[2] = false;
        regUsed2[0] = false;
    } else if (regWidth == JitWidth::b16) {
        // DX:AX = AX * src;
        if (regUsed2[0] || regUsed2[2]) {
            kpanic("JitX86CodeGen::mulReg 16");
        }
        RegPtr eax = getReg(0);
        regUsed2[2] = true;
        regUsed2[0] = true;
        x86.mov(x86.eax, R32(eax->hardwareReg()));
        x86.mulAx(R16(reg->hardwareReg()));
        x86.mov(R16(eax->hardwareReg()), x86.ax);
        x86.mov(R16(getReg(2)->hardwareReg()), x86.dx);
        regUsed2[2] = false;
        regUsed2[0] = false;
    } else if (regWidth == JitWidth::b8) {
        // AX = AL * src;
        if (regUsed2[0]) {
            kpanic("JitX86CodeGen::mulReg 8");
        }
        RegPtr eax = getReg(0);
        x86.mov(x86.ax, R16(eax->hardwareReg()));
        x86.mulAl(R8(get8bitReg(reg)));
        x86.mov(R16(eax->hardwareReg()), x86.ax);
    } else {
        kpanic("JitX86CodeGen::mulReg");
    }
}

void JitX86CodeGen::imulRR(JitWidth regWidth, RegPtr dst, RegPtr src) {
    if (regWidth == JitWidth::b32) {
        x86.imul(R32(dst->hardwareReg()), R32(src->hardwareReg()));
    } else if (regWidth == JitWidth::b16) {
        x86.imul(R16(dst->hardwareReg()), R16(src->hardwareReg()));
    } else {
        kpanic("JitX86CodeGen::imulRR");
    }
}

void JitX86CodeGen::imulRRI(JitWidth regWidth, RegPtr dst, RegPtr src, U32 src2) {
    if (regWidth == JitWidth::b32) {
        x86.imul(R32(dst->hardwareReg()), R32(src->hardwareReg()), src2);
    } else if (regWidth == JitWidth::b16) {
        x86.imul(R16(dst->hardwareReg()), R16(src->hardwareReg()), src2);
    } else {
        kpanic("JitX86CodeGen::imulRRI");
    }
}

void JitX86CodeGen::imulReg(JitWidth regWidth, RegPtr reg) {
    if (regWidth == JitWidth::b32) {
        // EDX:EAX = (S64)((S32)EAX) * ((S32)(src));
        if (regUsed2[0] || regUsed2[2]) {
            kpanic("JitX86CodeGen::imulReg 32");
        }
        RegPtr eax = getReg(0);
        regUsed2[2] = true;
        regUsed2[0] = true;
        x86.mov(x86.eax, R32(eax->hardwareReg()));
        x86.imulEax(R32(reg->hardwareReg()));
        x86.mov(R32(eax->hardwareReg()), x86.eax);
        x86.mov(R32(getReg(2)->hardwareReg()), x86.edx);
        regUsed2[2] = false;
        regUsed2[0] = false;
    } else if (regWidth == JitWidth::b16) {
        // DX:AX = (S32)((S16)AX) * (S16)src;
        if (regUsed2[0] || regUsed2[2]) {
            kpanic("JitX86CodeGen::imulReg 16");
        }
        RegPtr eax = getReg(0);
        regUsed2[2] = true;
        regUsed2[0] = true;
        x86.mov(x86.eax, R32(eax->hardwareReg()));
        x86.imulAx(R16(reg->hardwareReg()));
        x86.mov(R16(eax->hardwareReg()), x86.ax);
        x86.mov(R16(getReg(2)->hardwareReg()), x86.dx);
        regUsed2[2] = false;
        regUsed2[0] = false;
    } else if (regWidth == JitWidth::b8) {
        // AX = (S16)((S8)AL) * (S8)(src);
        if (regUsed2[0]) {
            kpanic("JitX86CodeGen::imulReg 8");
        }
        RegPtr eax = getReg(0);
        x86.mov(x86.ax, R16(eax->hardwareReg()));
        x86.imulAl(R8(get8bitReg(reg)));
        x86.mov(R16(eax->hardwareReg()), x86.ax);
    } else {
        kpanic("JitX86CodeGen::imulReg");
    }
}

void JitX86CodeGen::divRegRegWithRemainder32(RegPtr destLow, RegPtr destHigh, RegPtr src, RegPtr remainder) {    
#ifdef BOXEDWINE_64
    x86.push(RN(0));
    x86.push(RN(2));
    x86.mov(R64(0), R64(destLow->hardwareReg()));
    x86.shl(R64(0), 32);
    x86.shrd(R64(0), R64(destHigh->hardwareReg()), 32);
    x86.xor_(R32(2), R32(2));  // will clear top bits too
    x86.div(R64(src->hardwareReg()));
    x86.mov(R64(remainder->hardwareReg()), R64(2));
    x86.mov(R64(destLow->hardwareReg()), R64(0));
    x86.xor_(R32(destHigh->hardwareReg()), R32(destHigh->hardwareReg()));
    x86.pop(RN(2));
    x86.pop(RN(0));
#else
    if (destLow->hardwareReg() != 0 || destHigh->hardwareReg() != 2 || remainder->hardwareReg() == 2) {
        kpanic("JitX86CodeGen::divRegRegWithRemainder32");
    }
    If(JitWidth::b32, destHigh); {
        // divide high 32-bit first
        x86.push(RN(0));
        x86.mov(x86.eax, x86.edx);
        x86.xor_(x86.edx, x86.edx);
        x86.div(R32(src->hardwareReg()));
        x86.mov(R32(remainder->hardwareReg()), x86.eax); // remainder will hold on to the high 32-bit of the result, caller will turn this into an exception if it has a value
        x86.pop(RN(0));
        // divide low 32-bits
        x86.div(R32(src->hardwareReg())); // first div remainder : eax / src
        x86.xchg(x86.edx, R32(remainder->hardwareReg()));
    } StartElse(); {
        x86.div(R32(src->hardwareReg()));
        x86.mov(R32(remainder->hardwareReg()), R32(destHigh->hardwareReg()));
        x86.xor_(R32(destHigh->hardwareReg()), R32(destHigh->hardwareReg()));        
    } EndIf();
#endif
}

void JitX86CodeGen::idivRegRegWithRemainder32(RegPtr destLow, RegPtr destHigh, RegPtr src, RegPtr remainder) {
#ifdef BOXEDWINE_64
    x86.push(RN(0));
    x86.push(RN(2));
    x86.mov(R64(0), R64(destLow->hardwareReg()));
    x86.shl(R64(0), 32);
    x86.shrd(R64(0), R64(destHigh->hardwareReg()), 32);
    x86.cqo(); // sign extend rax into rdx:rax
    x86.movsx(R64(src->hardwareReg()), R32(src->hardwareReg()));
    x86.idiv(R64(src->hardwareReg()));
    x86.mov(R64(remainder->hardwareReg()), R64(2));
    x86.mov(R32(destLow->hardwareReg()), R32(0));
    x86.shr(R64(0), 32);
    x86.mov(R32(destHigh->hardwareReg()), R32(0));
    x86.pop(RN(2));
    x86.pop(RN(0));
#else
    if (destLow->hardwareReg() != 0 || destHigh->hardwareReg() != 2 || remainder->hardwareReg() == 2) {
        kpanic("JitX86CodeGen::idivRegRegWithRemainder32");
    }
    IfNot(JitWidth::b32, destHigh); {
        x86.idiv(R32(src->hardwareReg()));
        x86.mov(R32(remainder->hardwareReg()), R32(destHigh->hardwareReg()));
        x86.xor_(R32(destHigh->hardwareReg()), R32(destHigh->hardwareReg()));
    } StartElse(); {
        IfEqual(JitWidth::b32, destHigh, 0xffffffff); {
            x86.idiv(R32(src->hardwareReg()));
            x86.mov(R32(remainder->hardwareReg()), R32(destHigh->hardwareReg()));
            x86.xor_(R32(destHigh->hardwareReg()), R32(destHigh->hardwareReg()));            
        } StartElse(); {
            emulateSingleOp();
        } EndIf();
    } EndIf();
#endif
}

void JitX86CodeGen::divRegRegWithRemainder(JitWidth regWidth, RegPtr dest, RegPtr src, RegPtr remainder) {
    if (dest->hardwareReg() != 0) {
        x86.push(RN(0));
        x86.mov(R32(0), R32(dest->hardwareReg()));
    }
    if (remainder->hardwareReg() != 2) {
        x86.push(RN(2));
    }
    if (regWidth == JitWidth::b16) {
        x86.xor_(x86.dx, x86.dx); // 16-bit div on x86 makes a 32-bit word from DX and AX, then divides that by the source
        x86.div(R16(src->hardwareReg()));
        if (remainder->hardwareReg() != 2) {
            x86.mov(R16(remainder->hardwareReg()), R16(2));
        }
    } else if (regWidth == JitWidth::b32) {
        x86.xor_(x86.edx, x86.edx); // 32-bit div on x86 makes a 64-bit word from EDX and EAX, then divides that by the source
        x86.div(R32(src->hardwareReg()));
        if (remainder->hardwareReg() != 2) {
            x86.mov(R32(remainder->hardwareReg()), R32(2));
        }
    } else {
        kpanic("JitX86CodeGen::divRegRegWithRemainder");
    }    
    if (remainder->hardwareReg() != 2) {
        x86.pop(RN(2));
    }
    if (dest->hardwareReg() != 0) {
        x86.mov(R32(dest->hardwareReg()), R32(0));
        x86.pop(RN(0));        
    }
}

void JitX86CodeGen::idivRegRegWithRemainder(JitWidth regWidth, RegPtr dest, RegPtr src, RegPtr remainder) {
    if (dest->hardwareReg() != 0) {
        x86.push(RN(0));
        x86.mov(R32(0), R32(dest->hardwareReg()));
    }
    if (remainder->hardwareReg() != 2) {
        x86.push(RN(2));
    }
    if (regWidth == JitWidth::b16) {
        // sign extend ax into dx, sinc idiv uses dx:ax
        x86.mov(x86.dx, x86.ax);
        x86.sar(x86.dx, 15);
        x86.idiv(R16(src->hardwareReg()));
        if (remainder->hardwareReg() != 2) {
            x86.mov(R16(remainder->hardwareReg()), R16(2));
        }
    } else if (regWidth == JitWidth::b32) {
        // sign extend eax into edx, sinc idiv uses edx:eax
        x86.mov(x86.edx, x86.eax);
        x86.sar(x86.edx, 31);
        x86.idiv(R32(src->hardwareReg()));
        if (remainder->hardwareReg() != 2) {
            x86.mov(R32(remainder->hardwareReg()), R32(2));
        }
    } else {
        kpanic("JitX86CodeGen::idivRegRegWithRemainder");
    }
    if (remainder->hardwareReg() != 2) {
        x86.pop(RN(2));
    }
    if (dest->hardwareReg() != 0) {
        x86.mov(R32(dest->hardwareReg()), R32(0));
        x86.pop(RN(0));
    }
}

extern U8* ramPages[K_NUMBER_OF_PAGES];
void JitX86CodeGen::readRamPage(RegPtr dest, RegPtr index) {
#ifdef BOXEDWINE_64
    x86.mov(R32(dest->hardwareReg()), X86Asm::Mem32(HOST_RAM, R(index->hardwareReg()), 3, 0));
#else
    read(JitWidth::b32, dest, index, 2, (U32)ramPages);
#endif
}
#include "../../softmmu/kmemory_soft.h"
void JitX86CodeGen::readMMU(RegPtr dest, RegPtr index) {
#ifdef BOXEDWINE_64
    //x86.mov(R32(dest->hardwareReg()), X86Asm::Mem32(HOST_MMU, R(index->hardwareReg()), 2, 0));
    if ((U64)getMemData(KThread::currentThread()->memory)->mmu > 0x7fffffff) {
        kpanic("JitX86CodeGen::readMMU");
    }
    read(JitWidth::b32, dest, index, 2, (U32)(U64)getMemData(KThread::currentThread()->memory)->mmu);
#else
    read(JitWidth::b32, dest, index, 2, (U32)getMemData(KThread::currentThread()->memory)->mmu);
#endif
}

void JitX86CodeGen::read(JitWidth width, RegPtr dest, RegPtr reg, U8 lsl, U32 disp) {
    if (width == JitWidth::b32) {
        x86.mov(R32(dest->hardwareReg()), X86Asm::Mem32(R(reg->hardwareReg()), lsl, disp));
    } else if (width == JitWidth::b16) {
        x86.mov(R16(dest->hardwareReg()), X86Asm::Mem16(R(reg->hardwareReg()), lsl, disp));
    } else if (width == JitWidth::b8) {
        x86.mov(R8(get8bitReg(dest)), X86Asm::Mem8(R(reg->hardwareReg()), lsl, disp));
#ifdef BOXEDWINE_64
    } else if (width == JitWidth::b64) {
        x86.mov(R64(dest->hardwareReg()), X86Asm::Mem64(R(reg->hardwareReg()), lsl, disp));
#endif
    } else {
        kpanic_fmt("JitX86CodeGen::readMem unexpected width: %d", (U32)width);
    }
}

void JitX86CodeGen::read(JitWidth width, RegPtr dest, RegPtr reg, RegPtr sib, U8 lsl, U32 disp) {
    if (width == JitWidth::b32) {
        x86.mov(R32(dest->hardwareReg()), X86Asm::Mem32(R(reg->hardwareReg()), R(sib->hardwareReg()), lsl, disp));
    } else if (width == JitWidth::b16) {
        x86.mov(R16(dest->hardwareReg()), X86Asm::Mem16(R(reg->hardwareReg()), R(sib->hardwareReg()), lsl, disp));
    } else if (width == JitWidth::b8) {
        x86.mov(R8(get8bitReg(dest)), X86Asm::Mem8(R(reg->hardwareReg()), R(sib->hardwareReg()), lsl, disp));
#ifdef BOXEDWINE_64
    } else if (width == JitWidth::b64) {
        x86.mov(R64(dest->hardwareReg()), X86Asm::Mem64(R(reg->hardwareReg()), R(sib->hardwareReg()), lsl, disp));
#endif
    } else {
        kpanic_fmt("JitX86CodeGen::readMem unexpected width: %d", (U32)width);
    }
}

void JitX86CodeGen::write(JitWidth width, RegPtr reg, U32 disp, RegPtr src) {
    if (width == JitWidth::b32) {
        x86.mov(X86Asm::Mem32(R(reg->hardwareReg()), disp), R32(src->hardwareReg()));
    } else if (width == JitWidth::b16) {
        x86.mov(X86Asm::Mem16(R(reg->hardwareReg()), disp), R16(src->hardwareReg()));
    } else if (width == JitWidth::b8) {
        x86.mov(X86Asm::Mem8(R(reg->hardwareReg()), disp), R8(get8bitReg(src)));
#ifdef BOXEDWINE_64
    } else if (width == JitWidth::b64) {
        x86.mov(X86Asm::Mem64(R(reg->hardwareReg()), disp), R64(src->hardwareReg()));
#endif
    } else {
        kpanic_fmt("JitX86CodeGen::write unexpected width: %d", (U32)width);
    }
}

void JitX86CodeGen::write(JitWidth width, RegPtr reg, RegPtr sib, U8 lsl, U32 disp, U32 value) {
    if (width == JitWidth::b32) {
        x86.mov(X86Asm::Mem32(R(reg->hardwareReg()), R(sib->hardwareReg()), lsl, disp), (U32)value);
    } else if (width == JitWidth::b16) {
        x86.mov(X86Asm::Mem16(R(reg->hardwareReg()), R(sib->hardwareReg()), lsl, disp), (U16)value);
    } else if (width == JitWidth::b8) {
        x86.mov(X86Asm::Mem8(R(reg->hardwareReg()), R(sib->hardwareReg()), lsl, disp), (U8)value);
    } else {
        kpanic_fmt("JitX86CodeGen::write unexpected width: %d", (U32)width);
    }
}

void JitX86CodeGen::write(JitWidth width, RegPtr reg, RegPtr sib, U8 lsl, U32 disp, RegPtr src) {
    if (width == JitWidth::b32) {
        x86.mov(X86Asm::Mem32(R(reg->hardwareReg()), R(sib->hardwareReg()), lsl, disp), R32(src->hardwareReg()));
    } else if (width == JitWidth::b16) {
        x86.mov(X86Asm::Mem16(R(reg->hardwareReg()), R(sib->hardwareReg()), lsl, disp), R16(src->hardwareReg()));
    } else if (width == JitWidth::b8) {
        x86.mov(X86Asm::Mem8(R(reg->hardwareReg()), R(sib->hardwareReg()), lsl, disp), R8(get8bitReg(src)));
#ifdef BOXEDWINE_64
    } else if (width == JitWidth::b64) {
        x86.mov(X86Asm::Mem64(R(reg->hardwareReg()), R(sib->hardwareReg()), lsl, disp), R64(src->hardwareReg()));
#endif
    } else {
        kpanic_fmt("JitX86CodeGen::write unexpected width: %d", (U32)width);
    }
}

RegPtr JitX86CodeGen::readCPU(JitWidth width, U32 offset, RegPtr reg) {
    if (!reg) {
        reg = getTmpReg();
    }
    // mov reg, [edi+srcOffset]    
    if (width == JitWidth::b32) {
        x86.mov(R32(reg->hardwareReg()), X86Asm::Mem32(HOST_CPU, offset));
    } else if (width == JitWidth::b16) {
        x86.mov(R16(reg->hardwareReg()), X86Asm::Mem16(HOST_CPU, offset));
    } else if (width == JitWidth::b8) {
        x86.mov(R8(get8bitReg(reg)), X86Asm::Mem8(HOST_CPU, offset));
#ifdef BOXEDWINE_64
    } else if (width == JitWidth::b64) {
        x86.mov(R64(reg->hardwareReg()), X86Asm::Mem64(HOST_CPU, offset));
#endif
    } else {
        kpanic_fmt("unknown dstWidth in JitX86CodeGen::readCPU %d", width);
    }
    return reg;
}

RegPtr JitX86CodeGen::readCPU(JitWidth width, RegPtr sib, U8 lsl, U32 offset) {
    RegPtr reg = getTmpReg();

    if (width == JitWidth::b32) {
        x86.mov(R32(reg->hardwareReg()), X86Asm::Mem32(HOST_CPU, R(sib->hardwareReg()), lsl, offset));
    } else if (width == JitWidth::b16) {
        x86.mov(R16(reg->hardwareReg()), X86Asm::Mem16(HOST_CPU, R(sib->hardwareReg()), lsl, offset));
    } else if (width == JitWidth::b8) {
        x86.mov(R8(get8bitReg(reg)), X86Asm::Mem8(HOST_CPU, R(sib->hardwareReg()), lsl, offset));
#ifdef BOXEDWINE_64
    } else if (width == JitWidth::b64) {
        x86.mov(R64(reg->hardwareReg()), X86Asm::Mem64(HOST_CPU, R(sib->hardwareReg()), lsl, offset));
#endif
    } else {
        kpanic_fmt("unknown dstWidth in JitX86CodeGen::readCPU %d", width);
    }
    return reg;
}

void JitX86CodeGen::writeCPUValue(JitWidth width, RegPtr sib, U8 lsl, U32 offset, DYN_PTR_SIZE src) {
    if (width == JitWidth::b32) {
        x86.mov(X86Asm::Mem32(HOST_CPU, R32(sib->hardwareReg()), lsl, offset), (U32)src);
    } else if (width == JitWidth::b16) {
        x86.mov(X86Asm::Mem16(HOST_CPU, R32(sib->hardwareReg()), lsl, offset), (U16)src);
    } else if (width == JitWidth::b8) {
        x86.mov(X86Asm::Mem8(HOST_CPU, R32(sib->hardwareReg()), lsl, offset), (U8)src);
    } else if (width == JitWidth::b64) {
        x86.mov(X86Asm::Mem64(HOST_CPU, R(sib->hardwareReg()), lsl, offset), src);
    } else {
        kpanic_fmt("unknown dstWidth in JitX86CodeGen::writeCPUValue %d", width);
    }
}

void JitX86CodeGen::writeCPUValue(JitWidth width, U32 offset, DYN_PTR_SIZE src) {
    if (width == JitWidth::b32) {
        x86.mov(X86Asm::Mem32(HOST_CPU, offset), (U32)src);
    } else if (width == JitWidth::b16) {
        x86.mov(X86Asm::Mem16(HOST_CPU, offset), (U16)src);
    } else if (width == JitWidth::b8) {
        x86.mov(X86Asm::Mem8(HOST_CPU, offset), (U8)src);
    } else if (width == JitWidth::b64) {
        x86.mov(X86Asm::Mem64(HOST_CPU, offset), src);
    } else {
        kpanic_fmt("unknown dstWidth in JitX86CodeGen::writeCPUValue %d", width);
    }
}

void JitX86CodeGen::mov(JitWidth regWidth, RegPtr dest, RegPtr src) {
    if (regWidth == JitWidth::b32) {
        x86.mov(R32(dest->hardwareReg()), R32(src->hardwareReg()));
    } else if (regWidth == JitWidth::b16) {
        x86.mov(R16(dest->hardwareReg()), R16(src->hardwareReg()));
    } else if (regWidth == JitWidth::b8) {
        x86.mov(R8(get8bitReg(dest)), R8(get8bitReg(src)));
#ifdef BOXEDWINE_64
    } else if (regWidth == JitWidth::b64) {
        x86.mov(R64(dest->hardwareReg()), R64(src->hardwareReg()));
#endif
    } else {
        kpanic_fmt("JitX86CodeGen::mov unexpected width: %d", (U32)regWidth);
    }
}

void JitX86CodeGen::movValue(JitWidth regWidth, RegPtr dst, DYN_PTR_SIZE imm) {
    if (regWidth == JitWidth::b32) {
        x86.mov(R32(dst->hardwareReg()), (U32)imm);
    } else if (regWidth == JitWidth::b16) {
        x86.mov(R16(dst->hardwareReg()), (U16)imm);
    } else if (regWidth == JitWidth::b8) {
        x86.mov(R8(get8bitReg(dst)), (U8)imm);
#ifdef BOXEDWINE_64
    } else if (regWidth == JitWidth::b64) {
        x86.mov(R64(dst->hardwareReg()), imm);
#endif
    } else {
        kpanic_fmt("JitX86CodeGen::mov unexpected width: %d", (U32)regWidth);
    }
}

void JitX86CodeGen::movzx(JitWidth dstWidth, RegPtr dest, JitWidth srcWidth, RegPtr src) {
    if (dstWidth == JitWidth::b32) {
        if (srcWidth == JitWidth::b16) {
            x86.movzx(R32(dest->hardwareReg()), R16(src->hardwareReg()));
        } else if (srcWidth == JitWidth::b8) {
            x86.movzx(R32(dest->hardwareReg()), R8(get8bitReg(src)));
            dest->isHigh = false; // for when src == dest
        } else {
            kpanic_fmt("unknown width in JitX86CodeGen::movzx %d <= %d", dstWidth, srcWidth);
        }
    } else if (dstWidth == JitWidth::b16) {
        if (srcWidth == JitWidth::b8) {
            x86.movzx(R16(dest->hardwareReg()), R8(get8bitReg(src)));
            dest->isHigh = false; // for when src == dest
        } else {
            kpanic_fmt("unknown width in JitX86CodeGen::movzx %d <= %d", dstWidth, srcWidth);
        }
    } else {
        kpanic_fmt("unknown width in JitX86CodeGen::movzx %d <= %d", dstWidth, srcWidth);
    }
}

void JitX86CodeGen::movsx(JitWidth dstWidth, RegPtr dest, JitWidth srcWidth, RegPtr src) {
    if (dstWidth == JitWidth::b32) {
        if (srcWidth == JitWidth::b16) {
            x86.movsx(R32(dest->hardwareReg()), R16(src->hardwareReg()));
        } else if (srcWidth == JitWidth::b8) {
            x86.movsx(R32(dest->hardwareReg()), R8(get8bitReg(src)));
            dest->isHigh = false; // for when src == dest
        } else {
            kpanic_fmt("unknown width in JitX86CodeGen::movsx %d <= %d", dstWidth, srcWidth);
        }
    } else if (dstWidth == JitWidth::b16) {
        if (srcWidth == JitWidth::b8) {
            x86.movsx(R16(dest->hardwareReg()), R8(get8bitReg(src)));
            dest->isHigh = false; // for when src == dest
        } else {
            kpanic_fmt("unknown width in JitX86CodeGen::movsx %d <= %d", dstWidth, srcWidth);
        }
    } else {
        kpanic_fmt("unknown width in JitX86CodeGen::movsx %d <= %d", dstWidth, srcWidth);
    }
}

#ifdef BOXEDWINE_64
void JitX86CodeGen::setParam(X86Asm::Reg reg, const DynParam& param) {
    switch (param.type) {
    case JitCallParamType::REG_8:
        x86.movzx(R32(reg.reg), R8(get8bitReg(param.reg)));
        break;
    case JitCallParamType::REG_16:
        x86.movzx(R32(reg.reg), R16(param.reg->hardwareReg()));
        break;
    case JitCallParamType::REG_32:
        x86.mov(R32(reg.reg), R32(param.reg->hardwareReg()));
        break;
    case JitCallParamType::CPU:
        x86.mov(R64(reg.reg), HOST_CPU);
        break;
    case JitCallParamType::CONST_8:
        x86.mov(R32(reg.reg), (U32)(param.value & 0xFF));
        break;
    case JitCallParamType::CONST_16:
        x86.mov(R32(reg.reg), (U32)(param.value & 0xFFFF));
        break;
    case JitCallParamType::CONST_32:
        x86.mov(R32(reg.reg), (U32)param.value);
        break;
    case JitCallParamType::CONST_PTR:
        x86.mov(R64(reg.reg), param.value);
        break;
    default:
        kpanic_fmt("x32CPU: unknown argType: %d", param.type);
        break;
    }
}

#else
void JitX86CodeGen::pushParam(const DynParam& param) {
    switch (param.type) {
    case JitCallParamType::REG_8:
        x86.movzx(R32(param.reg->hardwareReg()), R8(get8bitReg(param.reg)));
        x86.push(R32(param.reg->hardwareReg()));
        break;
    case JitCallParamType::REG_16:
        x86.movzx(R32(param.reg->hardwareReg()), R16(param.reg->hardwareReg()));
        x86.push(R32(param.reg->hardwareReg()));
        break;
    case JitCallParamType::REG_32:
        x86.push(R32(param.reg->hardwareReg()));
        break;
    case JitCallParamType::CPU:
        x86.push(HOST_CPU);
        break;
    case JitCallParamType::CONST_8:
        x86.push((U32)(param.value & 0xFF));
        break;
    case JitCallParamType::CONST_16:
        x86.push((U32)(param.value & 0xFFFF));
        break;
    case JitCallParamType::CONST_32:
        x86.push(param.value);
        break;
    case JitCallParamType::CONST_PTR:
        x86.push((U32)param.value);
        break;
    default:
        kpanic_fmt("x32CPU: unknown argType: %d", param.type);
        break;
    }
}
#endif
void JitX86CodeGen::setParams(const std::vector<DynParam>& params) {
#ifdef BOXEDWINE_64
    bool clobberedReg[16] = { 0 };
    bool pushedReg[16] = { 0 };
    std::vector<X86Asm::Reg64> needToPush;

    for (int i = 0; i < 4 && i < params.size(); i++) {
        if (params[i].usesReg() && clobberedReg[params[i].reg->hardwareReg()]) {
            needToPush.push_back(R64(params[i].reg->hardwareReg()));
            pushedReg[i] = true;
        }
        clobberedReg[this->params[i].reg] = true;
    }
    if (needToPush.size()) {
        for (int i = (int)needToPush.size() - 1; i >= 0; i--) {
            x86.push(needToPush[i]);
        }
    }

    for (int i = 0; i < 4 && i < params.size(); i++) {
        if (params[i].usesReg() && pushedReg[i]) {
            x86.pop(R64(this->params[i].reg));
        } else {
            setParam(this->params[i], params[i]);
        }
    }
#else
    for (int i = (int)params.size() - 1; i >= 0; i--) {
        pushParam(params[i]);
    }
#endif
}

void JitX86CodeGen::callHostFunction(void* address, const std::vector<DynParam>& params, bool restoreCache) {
    int pushed = 0;
    U32 stackAdjust = 0;

#ifdef BOXEDWINE_64
    // volitile tmpRegs
    if (regUsed2[8]) {
        x86.push(RN(8));
        pushed++;
    }
    if (regUsed2[9]) {
        x86.push(RN(9));
        pushed++;
    }
    if (regUsed2[11]) {
        x86.push(RN(11));
        pushed++;
    }
#else
    if (regUsed2[x86.eax.reg]) {
        x86.push(RN(0));
    }
    if (regUsed2[x86.ecx.reg]) {
        x86.push(RN(1));
    }
    if (regUsed2[x86.edx.reg]) {
        x86.push(RN(2));
    }
#endif
#ifdef BOXEDWINE_64
    x86.mov(x86.r14, (U64)cpu->thread->process->syncToHost);
    x86.call(x86.r14);
#else
    writeCache();
#endif
    setParams(params);    
#ifdef BOXEDWINE_64   
    if ((pushed % 2) == 0) {
        stackAdjust = 8;
    }
    // part of the x64 windows ABI, shadow store
#ifdef BOXEDWINE_MSVC
    x86.sub(x86.rsp, 32 + stackAdjust);
#else
    if (stackAdjust) {
        x86.sub(x86.rsp, stackAdjust);
    }
#endif
    x86.mov(PARAM_CALL_TMP, (U64)address);
    x86.call(PARAM_CALL_TMP);
#ifdef BOXEDWINE_MSVC
    x86.add(x86.rsp, 32 + stackAdjust);
#else
    if (stackAdjust) {
        x86.add(x86.rsp, stackAdjust);
    }
#endif
    if (params.size() > 4) {
        kpanic("JitX86CodeGen::callHostFunction x64 doesn't support passing more than 4 parameters");
    }
#else
    x86.call(address);

    if (params.size()) {
        x86.add(x86.esp, sizeof(void*) * params.size());
    }
#endif
    if (restoreCache) {
#ifdef BOXEDWINE_64
        x86.mov(x86.r8, (U64)cpu->thread->process->syncFromHost);
        x86.call(x86.r8);
#else
        loadCache();
#endif
    }
#ifdef BOXEDWINE_64
    if (regUsed2[11]) {
        x86.pop(RN(11));
    }
    if (regUsed2[9]) {
        x86.pop(RN(9));
    }
    if (regUsed2[8]) {
        x86.pop(RN(8));
    }
#else
    if (regUsed2[x86.edx.reg]) {
        x86.pop(RN(2));
    }
    if (regUsed2[x86.ecx.reg]) {
        x86.pop(RN(1));
    }
    if (regUsed2[x86.eax.reg]) {
        x86.pop(RN(0));
    }
#endif
}

void JitX86CodeGen::callHostFunctionWithResult(RegPtr result, void* address, const std::vector<DynParam>& params) {
    bool pushed1 = false;
    bool pushed2 = false;
    bool pushed3 = false;
    int pushed = 0;
    U32 stackAdjust = 0; // if 64-bit stack isn't aligned to 16-bytes, things like FPU::F2XM1()

#ifdef BOXEDWINE_64
    // volitile tmpRegs
    if (regUsed2[8] && (!result->isLoaded() || result->hardwareReg() != 8)) {
        x86.push(RN(8));
        pushed1 = true;
        pushed++;
    }
    if (regUsed2[9] && (!result->isLoaded() || result->hardwareReg() != 9)) {
        x86.push(RN(9));
        pushed2 = true;
        pushed++;
    }
    if (regUsed2[11] && (!result->isLoaded() || result->hardwareReg() != 11)) {
        x86.push(RN(11));
        pushed3 = true;
        pushed++;
    }
#else
    if (regUsed2[x86.eax.reg] && (!result->isLoaded() || result->hardwareReg() != 0)) {
        x86.push(RN(0));
        pushed1 = true;
    }
    if (regUsed2[x86.ecx.reg] && (!result->isLoaded() || result->hardwareReg() != 1)) {
        x86.push(RN(1));
        pushed2 = true;
    }
    if (regUsed2[x86.edx.reg] && (!result->isLoaded() || result->hardwareReg() != 2)) {
        x86.push(RN(2));
        pushed3 = true;
    }
#endif
#ifdef BOXEDWINE_64
    x86.mov(x86.r14, (U64)cpu->thread->process->syncToHost);
    x86.call(x86.r14);
#else
    writeCache();
#endif
    setParams(params);
    
#ifdef BOXEDWINE_64
    if ((pushed % 2) == 0) {
        stackAdjust = 8;
    }
    // part of the x64 windows ABI, shadow store
#ifdef BOXEDWINE_MSVC
    x86.sub(x86.rsp, 32 + stackAdjust);
#else
    if (stackAdjust) {
        x86.sub(x86.rsp, stackAdjust);
    }
#endif
    x86.mov(PARAM_CALL_TMP, (U64)address);
    x86.call(PARAM_CALL_TMP);
#ifdef BOXEDWINE_MSVC
    x86.add(x86.rsp, 32 + stackAdjust);
#else
    if (stackAdjust) {
        x86.add(x86.rsp, stackAdjust);
    }
#endif
#else
    x86.call(address);

    if (params.size()) {
        x86.add(x86.esp, sizeof(void*) * params.size());
    }
#endif
#ifdef BOXEDWINE_64
    if (pushed3) {
        x86.pop(RN(11));
    }
    if (pushed2) {
        x86.pop(RN(9));
    }    
    if (pushed1) {
        x86.pop(RN(8));
    }
    x86.mov(R32(result->hardwareReg()), x86.eax);
    RegPtr tmp = getTmpReg();
    x86.mov(R64(tmp->hardwareReg()), (U64)cpu->thread->process->syncFromHost);
    x86.call(R64(tmp->hardwareReg()));
#else
    if (pushed3) {
        x86.pop(RN(2));
    }
    if (pushed2) {
        x86.pop(RN(1));
    }
    if (result->hardwareReg() != 0) {
        x86.mov(R32(result->hardwareReg()), x86.eax);
    }
    if (pushed1) {
        x86.pop(RN(0));
    }
    loadCache();
#endif
}

void JitX86CodeGen::If(JitWidth regWidth, RegPtr reg) {
    if (regWidth == JitWidth::b32) {
        x86.IfNotZero(R32(reg->hardwareReg()));
    } else if (regWidth == JitWidth::b16) {
        x86.IfNotZero(R16(reg->hardwareReg()));
    } else if (regWidth == JitWidth::b8) {
        x86.IfNotZero(R8(get8bitReg(reg)));
#ifdef BOXEDWINE_64
    } else if (regWidth == JitWidth::b64) {
        x86.IfNotZero(R64(reg->hardwareReg()));
#endif
    } else {
        kpanic_fmt("JitX86CodeGen::If unexpected width: %d", (U32)regWidth);
    }
}

void JitX86CodeGen::IfTest(JitWidth regWidth, RegPtr reg, RegPtr mask) {
    if (regWidth == JitWidth::b8) {
        x86.IfBitSet(R32(get8bitReg(reg)), R32(get8bitReg(mask)));
    } else {
        x86.IfBitSet(R32(reg->hardwareReg()), R32(mask->hardwareReg()));
    }
}

void JitX86CodeGen::IfTest(JitWidth regWidth, RegPtr reg, U32 value) {
    if (regWidth == JitWidth::b8) {
        x86.IfBitSet(R32(get8bitReg(reg)), value);
    } else {
        x86.IfBitSet(R32(reg->hardwareReg()), value);
    }
}

void JitX86CodeGen::IfNotTest(JitWidth regWidth, RegPtr reg, U32 value) {
    if (regWidth == JitWidth::b8) {
        x86.IfNotBitSet(R32(get8bitReg(reg)), value);
    } else {
        x86.IfNotBitSet(R32(reg->hardwareReg()), value);
    }
}

void JitX86CodeGen::IfEqual(JitWidth regWidth, RegPtr reg1, RegPtr reg2) {
    if (regWidth == JitWidth::b32) {
        x86.IfEqual(R32(reg1->hardwareReg()), R32(reg2->hardwareReg()));
    } else if (regWidth == JitWidth::b16) {
        x86.IfEqual(R16(reg1->hardwareReg()), R16(reg2->hardwareReg()));
    } else if (regWidth == JitWidth::b8) {
        x86.IfEqual(R8(get8bitReg(reg1)), R8(get8bitReg(reg2)));
    } else {
        kpanic_fmt("JitX86CodeGen::IfEqual unexpected width: %d", (U32)regWidth);
    }
}

void JitX86CodeGen::IfEqual(JitWidth regWidth, RegPtr reg, DYN_PTR_SIZE value) {
    if (regWidth == JitWidth::b32) {
        x86.IfEqual(R32(reg->hardwareReg()), (U32)value);
    } else if (regWidth == JitWidth::b16) {
        x86.IfEqual(R16(reg->hardwareReg()), (U16)value);
    } else if (regWidth == JitWidth::b8) {
        x86.IfEqual(R8(get8bitReg(reg)), (U8)value);
#ifdef BOXEDWINE_64
    } else if (regWidth == JitWidth::b64) {
        if (value <= 0xffffffffl) {
            x86.IfEqual(R64(reg->hardwareReg()), (U32)value);
        } else {
            RegPtr r = getTmpReg();
            x86.mov(R64(r->hardwareReg()), value);
            x86.IfEqual(R64(reg->hardwareReg()), R64(r->hardwareReg()));
        }
#endif
    } else {
        kpanic_fmt("JitX86CodeGen::IfEqual unexpected width: %d", (U32)regWidth);
    }
}

void JitX86CodeGen::IfNotEqual(JitWidth regWidth, RegPtr reg, DYN_PTR_SIZE value) {
    if (regWidth == JitWidth::b32) {
        x86.IfNotEqual(R32(reg->hardwareReg()), (U32)value);
    } else if (regWidth == JitWidth::b16) {
        x86.IfNotEqual(R16(reg->hardwareReg()), (U16)value);
    } else if (regWidth == JitWidth::b8) {
        x86.IfNotEqual(R8(get8bitReg(reg)), (U8)value);
#ifdef BOXEDWINE_64
    } else if (regWidth == JitWidth::b64) {
        if (value <= 0xffffffffl) {
            x86.IfNotEqual(R64(reg->hardwareReg()), (U32)value);
        } else {
            RegPtr r = getTmpReg();
            x86.mov(R64(r->hardwareReg()), value);
            x86.IfNotEqual(R64(reg->hardwareReg()), R64(r->hardwareReg()));
        }
#endif
    } else {
        kpanic_fmt("JitX86CodeGen::IfNotEqual unexpected width: %d", (U32)regWidth);
    }
}

void JitX86CodeGen::IfNotEqual(JitWidth regWidth, RegPtr reg, RegPtr reg2) {
    if (regWidth == JitWidth::b32) {
        x86.IfNotEqual(R32(reg->hardwareReg()), R32(reg2->hardwareReg()));
    } else if (regWidth == JitWidth::b16) {
        x86.IfNotEqual(R16(reg->hardwareReg()), R16(reg2->hardwareReg()));
    } else if (regWidth == JitWidth::b8) {
        x86.IfNotEqual(R8(get8bitReg(reg)), R8(get8bitReg(reg2)));
    } else {
        kpanic_fmt("JitX86CodeGen::IfNotEqual unexpected width: %d", (U32)regWidth);
    }
}

void JitX86CodeGen::IfLessThan2(JitWidth regWidth, RegPtr reg, U32 value) {
    if (regWidth == JitWidth::b32) {
        x86.IfLessThan(R32(reg->hardwareReg()), value);
    } else if (regWidth == JitWidth::b16) {
        x86.IfLessThan(R16(reg->hardwareReg()), (U16)value);
    } else if (regWidth == JitWidth::b8) {
        x86.IfLessThan(R8(get8bitReg(reg)), (U8)value);
    } else {
        kpanic_fmt("JitX86CodeGen::IfLessThan unexpected width: %d", (U32)regWidth);
    }
}

void JitX86CodeGen::IfLessThan2(JitWidth regWidth, RegPtr reg1, RegPtr reg2) {
    if (regWidth == JitWidth::b32) {
        x86.IfLessThan(R32(reg1->hardwareReg()), R32(reg2->hardwareReg()));
    } else if (regWidth == JitWidth::b16) {
        x86.IfLessThan(R16(reg1->hardwareReg()), R16(reg2->hardwareReg()));
    } else if (regWidth == JitWidth::b8) {
        x86.IfLessThan(R8(get8bitReg(reg1)), R8(get8bitReg(reg2)));
    } else {
        kpanic_fmt("JitX86CodeGen::IfLessThan unexpected width: %d", (U32)regWidth);
    }
}

void JitX86CodeGen::IfNot(JitWidth regWidth, RegPtr reg) {
    if (regWidth == JitWidth::b32) {
        x86.IfZero(R32(reg->hardwareReg()));
    } else if (regWidth == JitWidth::b16) {
        x86.IfZero(R16(reg->hardwareReg()));
    } else if (regWidth == JitWidth::b8) {
        x86.IfZero(R8(get8bitReg(reg)));
#ifdef BOXEDWINE_64
    } else if (regWidth == JitWidth::b64) {
        x86.IfZero(R64(reg->hardwareReg()));
#endif
    } else {
        kpanic_fmt("JitX86CodeGen::IfNot unexpected width: %d", (U32)regWidth);
    }
}

void JitX86CodeGen::IfNotCPU(JitWidth regWidth, RegPtr sib, U8 lsl, U32 offset) {
    if (regWidth == JitWidth::b32) {
        x86.cmp(X86Asm::Mem32(HOST_CPU, R32(sib->hardwareReg()), lsl, offset), 0);
    } else if (regWidth == JitWidth::b16) {
        x86.cmp(X86Asm::Mem16(HOST_CPU, R32(sib->hardwareReg()), lsl, offset), 0);
    } else if (regWidth == JitWidth::b8) {
        x86.cmp(X86Asm::Mem8(HOST_CPU, R32(sib->hardwareReg()), lsl, offset), 0);
    } else {
        kpanic_fmt("JitX86CodeGen::IfNotCPU unexpected width: %d", (U32)regWidth);
    }
    x86.IfZF();
}

void JitX86CodeGen::IfCompareReg(JitWidth regWidth, RegPtr reg1, RegPtr reg2, JitEvaluate condition) {
    if (regWidth == JitWidth::b32) {
        x86.cmp(R32(reg1->hardwareReg()), R32(reg2->hardwareReg()));
    } else if (regWidth == JitWidth::b16) {
        x86.cmp(R16(reg1->hardwareReg()), R16(reg2->hardwareReg()));
    } else if (regWidth == JitWidth::b8) {
        x86.cmp(R8(get8bitReg(reg1)), R8(get8bitReg(reg2)));
    } else {
        kpanic_fmt("JitX86CodeGen::IfCompareReg unexpected width: %d", (U32)regWidth);
    }
    switch (condition) {
    case JitEvaluate::EQUALS:
        x86.jnz();
        break;
    case JitEvaluate::NOT_EQUALS:
        x86.jz();
        break;
    case JitEvaluate::LESS_THAN_UNSIGNED:
        x86.jnb();
        break;
    case JitEvaluate::LESS_THAN_EQUAL_UNSIGNED:
        x86.jnbe();
        break;
    case JitEvaluate::GREATER_THAN_UNSIGNED:
        x86.jbe();
        break;
    case JitEvaluate::GREATER_THAN_EQUAL_UNSIGNED:
        x86.jb();
        break;
    case JitEvaluate::LESS_THAN_SIGNED:
        x86.jnl();
        break;
    case JitEvaluate::GREATER_THAN_EQUAL_SIGNED:
        x86.jl();
        break;
    case JitEvaluate::LESS_THAN_EQUAL_SIGNED:
        x86.jnle();
        break;
    case JitEvaluate::GREATER_THAN_SIGNED:
        x86.jle();
        break;
    // no default, should get compiler error if not all enum cases handled
    }
}

void JitX86CodeGen::JumpIfCondition(JitConditional condition, U32 address) {
    IfCondition(condition);
        x86.jmp(address);
    EndIf();
}

void JitX86CodeGen::IfFlagSet(U32 flag) {
    RegPtr reg = getTmpReg();
    x86.mov(R32(reg->hardwareReg()), X86Asm::Mem32(HOST_CPU, offsetof(CPU, flags)));
    x86.IfBitSet(R32(reg->hardwareReg()), flag);
}

void JitX86CodeGen::IfSmallStack() {
    RegPtr reg = getTmpReg();
    x86.mov(R32(reg->hardwareReg()), X86Asm::Mem32(HOST_CPU, offsetof(CPU, stackNotMask)));
    If(JitWidth::b32, reg);
}

U32 JitX86CodeGen::MarkJumpLocation() {
    return (U32)x86.buffer.size();
}

void JitX86CodeGen::Goto(U32 location) {
    U32 amount = location - (U32)x86.buffer.size() - 1;

    if (amount > 127) {
        amount = location - (U32)x86.buffer.size() - 5;
        x86.goto32(amount);
    } else {
        x86.goto8(amount);
    }
}

void JitX86CodeGen::jmp(RegPtr reg) {
    x86.jmp(R(reg->hardwareReg()));
}

void JitX86CodeGen::writeCPU(JitWidth width, U32 offset, RegPtr src) {
    if (width == JitWidth::b32) {
        x86.mov(X86Asm::Mem32(HOST_CPU, offset), R32(src->hardwareReg()));
    } else if (width == JitWidth::b16) {
        x86.mov(X86Asm::Mem16(HOST_CPU, offset), R16(src->hardwareReg()));
    } else if (width == JitWidth::b8) {
        x86.mov(X86Asm::Mem8(HOST_CPU, offset), R8(get8bitReg(src)));
#ifdef BOXEDWINE_64
    } else if (width == JitWidth::b64) {
        x86.mov(X86Asm::Mem64(HOST_CPU, offset), R64(src->hardwareReg()));
#endif
    } else {
        kpanic_fmt("unknown dstWidth in JitX86CodeGen::writeCPU %d", width);
    }
}

void JitX86CodeGen::writeCPU(JitWidth width, RegPtr sib, U8 lsl, U32 offset, RegPtr src) {
    if (width == JitWidth::b32) {
        x86.mov(X86Asm::Mem32(HOST_CPU, R(sib->hardwareReg()), lsl, offset), R32(src->hardwareReg()));
    } else if (width == JitWidth::b16) {
        x86.mov(X86Asm::Mem16(HOST_CPU, R(sib->hardwareReg()), lsl, offset), R16(src->hardwareReg()));
    } else if (width == JitWidth::b8) {
        x86.mov(X86Asm::Mem8(HOST_CPU, R(sib->hardwareReg()), lsl, offset), R8(get8bitReg(src)));
#ifdef BOXEDWINE_64
    } else if (width == JitWidth::b64) {
        x86.mov(X86Asm::Mem64(HOST_CPU, R(sib->hardwareReg()), lsl, offset), R64(src->hardwareReg()));
#endif
    } else {
        kpanic_fmt("unknown dstWidth in JitX86CodeGen::writeCPU %d", width);
    }
}

void JitX86CodeGen::storeCpuXMMReg(DynXMMReg reg, U32 index) {
    x86.movaps(X86Asm::Mem128(HOST_CPU, index * 16 + offsetof(CPU, xmm)), X86Asm::XMM(reg));
}

void JitX86CodeGen::storeXMMToMem128(DynXMMReg reg, RegPtr rm, RegPtr sib, U8 lsl, U32 disp) {
    x86.movups(X86Asm::Mem128(R(rm->hardwareReg()), R(sib->hardwareReg()), lsl, disp), X86Asm::XMM(reg));
}

void JitX86CodeGen::storeXMMToMem64(DynXMMReg reg, RegPtr rm, RegPtr sib, U8 lsl, U32 disp) {
    x86.movlps(X86Asm::Mem64(R(rm->hardwareReg()), R(sib->hardwareReg()), lsl, disp), X86Asm::XMM(reg));
}

void JitX86CodeGen::storeXMMToMem32(DynXMMReg reg, RegPtr rm, RegPtr sib, U8 lsl, U32 disp) {
    x86.movss(X86Asm::Mem32(R(rm->hardwareReg()), R(sib->hardwareReg()), lsl, disp), X86Asm::XMM(reg));
}

void JitX86CodeGen::storeHighXMMToMem64(DynXMMReg reg, RegPtr rm, RegPtr sib, U8 lsl, U32 disp) {
    x86.movhps(X86Asm::Mem64(R(rm->hardwareReg()), R(sib->hardwareReg()), lsl, disp), X86Asm::XMM(reg));
}

void JitX86CodeGen::loadCpuXMMReg(DynXMMReg reg, U32 index) {
    x86.movaps(X86Asm::XMM(reg), X86Asm::Mem128(HOST_CPU, index * 16 + offsetof(CPU, xmm)));
}

void JitX86CodeGen::loadCpuXMMReg64ZeroExtend(DynXMMReg reg, U32 index) {
    x86.movq(X86Asm::XMM(reg), X86Asm::Mem64(HOST_CPU, index * 16 + offsetof(CPU, xmm)));
}

void JitX86CodeGen::loadXMMFromMem128(DynXMMReg reg, RegPtr rm, RegPtr sib, U8 lsl, U32 disp) {
    x86.movups(X86Asm::XMM(reg), X86Asm::Mem128(R(rm->hardwareReg()), R(sib->hardwareReg()), lsl, disp));
}

void JitX86CodeGen::loadXMMFromMem64(DynXMMReg reg, RegPtr rm, RegPtr sib, U8 lsl, U32 disp) {
    x86.movq(X86Asm::XMM(reg), X86Asm::Mem64(R(rm->hardwareReg()), R(sib->hardwareReg()), lsl, disp));
}

void JitX86CodeGen::loadLowXMMFromMem64(DynXMMReg reg, RegPtr rm, RegPtr sib, U8 lsl, U32 disp) {
    x86.movlps(X86Asm::XMM(reg), X86Asm::Mem64(R(rm->hardwareReg()), R(sib->hardwareReg()), lsl, disp));
}

void JitX86CodeGen::loadHighXMMFromMem64(DynXMMReg reg, RegPtr rm, RegPtr sib, U8 lsl, U32 disp) {
    x86.movhps(X86Asm::XMM(reg), X86Asm::Mem64(R(rm->hardwareReg()), R(sib->hardwareReg()), lsl, disp));
}

void JitX86CodeGen::loadXMMFromMem32(DynXMMReg reg, RegPtr rm, RegPtr sib, U8 lsl, U32 disp) {
    x86.movss(X86Asm::XMM(reg), X86Asm::Mem32(R(rm->hardwareReg()), R(sib->hardwareReg()), lsl, disp));
}

void JitX86CodeGen::addpsXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.addps(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::addssXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.addss(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::subpsXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.subps(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::subssXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.subss(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::mulpsXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.mulps(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::mulssXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.mulss(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::divpsXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.divps(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::divssXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.divss(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::rcppsXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.rcpps(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::rcpssXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.rcpss(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::sqrtpsXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.sqrtps(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::sqrtssXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.sqrtss(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::rsqrtpsXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.rsqrtps(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::rsqrtssXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.rsqrtss(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::maxpsXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.maxps(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::maxssXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.maxss(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::minpsXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.minps(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::minssXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.minss(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::pavgbMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.pavgb(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::pavgwMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.pavgw(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::psadbwMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.psadbw(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::pextrwRegMmx(RegPtr dst, DynMMXReg src, U8 srcIndex) {
    x86.pextrw(R32(dst->hardwareReg()), X86Asm::XMM(src), srcIndex);
}

void JitX86CodeGen::pinsrwMmxReg(DynMMXReg dst, RegPtr src, U8 dstIndex) {
    x86.pinsrw(X86Asm::XMM(dst), R32(src->hardwareReg()), dstIndex);
}

void JitX86CodeGen::pmaxswMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.pmaxsw(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::pmaxubMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.pmaxub(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::pminswMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.pminsw(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::pminubMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.pminub(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::pmovmskbMmxMmx(RegPtr dst, DynMMXReg src) {
    x86.pmovmskb(R32(dst->hardwareReg()), X86Asm::XMM(src));
}

void JitX86CodeGen::pmulhuwMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.pmulhuw(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::pshufwMmxMmx(DynMMXReg dst, DynMMXReg src, U8 order) {
    x86.pshuflw(X86Asm::XMM(dst), X86Asm::XMM(src), order);
}

void JitX86CodeGen::maskmovq(DynMMXReg src, DynMMXReg mask, RegPtr destAddress) {
    x86.push(RN(7));
    x86.mov(RN(7), RN(destAddress->hardwareReg()));
    // this works because the top 64-bits of the mask should be 0's since its used for MMX
    x86.maskmovdqu(X86Asm::XMM(src), X86Asm::XMM(mask));
    x86.pop(RN(7));
}

void JitX86CodeGen::paddqMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.paddq(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::psubqMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.psubq(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::pmuludqMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.pmuludq(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::andnpsXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.andnps(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::andpsXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.andps(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::orpsXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.orps(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::xorpsXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.xorps(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::cvtpi2psXmmMmx(DynXMMReg dst, DynMMXReg src) {
    // cvtpi2ps need to keep top 64-bits of the xmm dst
    DynXMMReg tmp = getTmpXMM(dst);
    x86.cvtdq2ps(X86Asm::XMM(tmp), X86Asm::XMM(src));
    x86.movsd(X86Asm::XMM(dst), X86Asm::XMM(tmp));
}

void JitX86CodeGen::cvtps2piMmxXmm(DynMMXReg dst, DynXMMReg src) {
    x86.cvtps2dq(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::cvtsi2ssXmmR32(DynXMMReg dst, RegPtr src) {
    x86.cvtsi2ss(X86Asm::XMM(dst), R32(src->hardwareReg()));
}

void JitX86CodeGen::cvtss2siR32Xmm(RegPtr dst, DynXMMReg src) {
    x86.cvtss2si(R32(dst->hardwareReg()), X86Asm::XMM(src));
}

void JitX86CodeGen::cvttps2piMmxXmm(DynMMXReg dst, DynXMMReg src) {
    x86.cvttps2dq(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::cvttss2siR32Xmm(RegPtr dst, DynXMMReg src) {
    x86.cvttss2si(R32(dst->hardwareReg()), X86Asm::XMM(src));
}

void JitX86CodeGen::movhlpsXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.movhlps(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::movlhpsXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.movlhps(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::movssXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.movss(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::shufpsXmmXmm(DynXMMReg dst, DynXMMReg src, U32 imm) {
    x86.shufps(X86Asm::XMM(dst), X86Asm::XMM(src), (U8)imm);
}

void JitX86CodeGen::cmppsXmmXmm(DynXMMReg dst, DynXMMReg src, U32 imm) {
    x86.cmpps(X86Asm::XMM(dst), X86Asm::XMM(src), (U8)imm);
}

void JitX86CodeGen::cmpssXmmXmm(DynXMMReg dst, DynXMMReg src, U32 imm) {
    x86.cmpss(X86Asm::XMM(dst), X86Asm::XMM(src), (U8)imm);
}

void JitX86CodeGen::setFlags(RegPtr flags, U32 mask) {
    if (mask != FMASK_TEST) {
        call_RI(common_setFlags, JitWidth::b32, flags, mask);
    } else {
        RegPtr reg = getTmpReg();
        x86.mov(R32(reg->hardwareReg()), X86Asm::Mem32(HOST_CPU, offsetof(CPU, flags)));
        x86.and_(R32(reg->hardwareReg()), ~mask);
        x86.and_(R32(flags->hardwareReg()), mask);
        x86.or_(R32(reg->hardwareReg()), R32(flags->hardwareReg()));
        x86.mov(X86Asm::Mem32(HOST_CPU, offsetof(CPU, flags)), R32(reg->hardwareReg()));
        storeLazyFlags(FLAGS_NONE);
    }
    currentLazyFlags = FLAGS_NONE;
}

RegPtr JitX86CodeGen::getReadOnlyFlags() {
    fillFlags();

    RegPtr reg = getTmpReg8(); // lahf will do 8-bit on this
    x86.mov(R32(reg->hardwareReg()), X86Asm::Mem32(HOST_CPU, offsetof(CPU, flags)));
    orValue(JitWidth::b32, reg, 2);
    andValue(JitWidth::b32, reg, 0xFCFFFF);
    return reg;
}

void JitX86CodeGen::updateFlagsIfNecessary() {
    U32 neededFlags = currentOp->needsToSetFlags(cpu);
    if (neededFlags) {
        currentLazyFlags = FLAGS_NONE;
        if (neededFlags == CF) {
            x86.IfCF();
            x86.bts(X86Asm::Mem32(HOST_CPU, offsetof(CPU, flags)), 0);
            x86.Else();
            x86.btr(X86Asm::Mem32(HOST_CPU, offsetof(CPU, flags)), 0);
            x86.EndIf();
            storeLazyFlags(FLAGS_NONE);
            return;
        }
        if (neededFlags == ZF) {
            x86.IfZF();
            x86.bts(X86Asm::Mem32(HOST_CPU, offsetof(CPU, flags)), 6);
            x86.Else();
            x86.btr(X86Asm::Mem32(HOST_CPU, offsetof(CPU, flags)), 6);
            x86.EndIf();
            storeLazyFlags(FLAGS_NONE);
            return;
        }
        if (neededFlags == SF) {
            x86.IfSF();
            x86.bts(X86Asm::Mem32(HOST_CPU, offsetof(CPU, flags)), 7);
            x86.Else();
            x86.btr(X86Asm::Mem32(HOST_CPU, offsetof(CPU, flags)), 7);
            x86.EndIf();
            storeLazyFlags(FLAGS_NONE);
            return;
        }
        if (neededFlags == OF) {
            x86.IfOF();
            x86.bts(X86Asm::Mem32(HOST_CPU, offsetof(CPU, flags)), 11);
            x86.Else();
            x86.btr(X86Asm::Mem32(HOST_CPU, offsetof(CPU, flags)), 11);
            x86.EndIf();
            storeLazyFlags(FLAGS_NONE);
            return;
        }
        if (neededFlags == PF) {
            x86.IfPF();
            x86.bts(X86Asm::Mem32(HOST_CPU, offsetof(CPU, flags)), 2);
            x86.Else();
            x86.btr(X86Asm::Mem32(HOST_CPU, offsetof(CPU, flags)), 2);
            x86.EndIf();
            storeLazyFlags(FLAGS_NONE);
            return;
        }
        bool savedEAX = false;

        if (regUsed2[0] || regCache[0] == 0) {
            x86.push(RN(0));
            savedEAX = true;
        }
        if (neededFlags & OF) {
            x86.lahf();
            x86.seto(R8(0));
            x86.shl(R8(0), 3);
            x86.xchg(R8(4), R8(0));
        } else {
            x86.lahf();
            x86.shr(R16(0), 8);
        }
        // mask so we don't clobber DF
        x86.and_(X86Asm::Mem32(HOST_CPU, offsetof(CPU, flags)), ~FMASK_TEST);
        x86.or_(X86Asm::Mem16(HOST_CPU, offsetof(CPU, flags)), x86.ax);
        if (savedEAX) {
            x86.pop(RN(0));
        }
        storeLazyFlags(FLAGS_NONE);
    }
}

void JitX86CodeGen::comissXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.comiss(X86Asm::XMM(dst), X86Asm::XMM(src));
    updateFlagsIfNecessary();
}

void JitX86CodeGen::ucomissXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.ucomiss(X86Asm::XMM(dst), X86Asm::XMM(src));
    updateFlagsIfNecessary();
}

void JitX86CodeGen::stmxcsr(RegPtr address) {
    x86.stmxcsr(X86Asm::Mem32(R32(address->hardwareReg()), 0));
}

void JitX86CodeGen::ldmxcsr(RegPtr address) {
    x86.ldmxcsr(X86Asm::Mem32(R32(address->hardwareReg()), 0));
}

void JitX86CodeGen::sfence() {
    x86.sfence();
}

void JitX86CodeGen::unpckhpsXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.unpckhps(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::unpcklpsXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.unpcklps(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::movmskpsR32Xmm(RegPtr dst, DynXMMReg src) {
    x86.movmskps(R32(dst->hardwareReg()), X86Asm::XMM(src));
}

void JitX86CodeGen::loadMMXFromReg(DynMMXReg dst, RegPtr src) {
    x86.movd(X86Asm::XMM(dst), R32(src->hardwareReg()));
}

void JitX86CodeGen::storeCpuMMXReg(DynMMXReg reg, U32 index) {
    x86.movq(X86Asm::Mem64(HOST_CPU, index * cpu->fpu.sizeofRegInRegsArray() + offsetof(CPU, fpu.regs[0].signif)), X86Asm::XMM(reg));
}

void JitX86CodeGen::storeMMXToReg(DynMMXReg src, RegPtr dst) {
    x86.movd(R32(dst->hardwareReg()), X86Asm::XMM(src));
}

void JitX86CodeGen::loadCpuMMXReg(DynMMXReg reg, U32 index) {
    x86.movq(X86Asm::XMM(reg), X86Asm::Mem64(HOST_CPU, index * cpu->fpu.sizeofRegInRegsArray() + offsetof(CPU, fpu.regs[0].signif)));
}

void JitX86CodeGen::loadMMXFromMem32(DynMMXReg reg, RegPtr rm, RegPtr sib, U8 lsl, U32 disp) {
    x86.movd(X86Asm::XMM(reg), X86Asm::Mem32(R32(rm->hardwareReg()), R32(sib->hardwareReg()), lsl, disp));
}

void JitX86CodeGen::loadMMXFromMem64(DynMMXReg reg, RegPtr rm, RegPtr sib, U8 lsl, U32 disp) {
    x86.movq(X86Asm::XMM(reg), X86Asm::Mem64(R32(rm->hardwareReg()), R32(sib->hardwareReg()), lsl, disp));
}

void JitX86CodeGen::storeMMXToMem32(DynMMXReg reg, RegPtr rm, RegPtr sib, U8 lsl, U32 disp) {
    x86.movd(X86Asm::Mem32(R32(rm->hardwareReg()), R32(sib->hardwareReg()), lsl, disp), X86Asm::XMM(reg));
}

void JitX86CodeGen::storeMMXToMem64(DynMMXReg reg, RegPtr rm, RegPtr sib, U8 lsl, U32 disp) {
    x86.movq(X86Asm::Mem64(R32(rm->hardwareReg()), R32(sib->hardwareReg()), lsl, disp), X86Asm::XMM(reg));
}

void JitX86CodeGen::xorMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.pxor(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::orMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.por(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::andMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.pand(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::andnMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.pandn(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::psllwMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.psllw(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::psrlwMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.psrlw(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::psrawMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.psraw(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::psllwMmx(DynMMXReg dst, U32 imm) {
    x86.psllw(X86Asm::XMM(dst), imm);
}

void JitX86CodeGen::psrlwMmx(DynMMXReg dst, U32 imm) {
    x86.psrlw(X86Asm::XMM(dst), imm);
}

void JitX86CodeGen::psrawMmx(DynMMXReg dst, U32 imm) {
    x86.psraw(X86Asm::XMM(dst), imm);
}

void JitX86CodeGen::pslldMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.pslld(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::psrldMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.psrld(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::psradMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.psrad(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::pslldMmx(DynMMXReg dst, U32 imm) {
    x86.pslld(X86Asm::XMM(dst), imm);
}

void JitX86CodeGen::psrldMmx(DynMMXReg dst, U32 imm) {
    x86.psrld(X86Asm::XMM(dst), imm);
}

void JitX86CodeGen::psradMmx(DynMMXReg dst, U32 imm) {
    x86.psrad(X86Asm::XMM(dst), imm);
}

void JitX86CodeGen::psllqMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.psllq(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::psrlqMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.psrlq(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::psllqMmx(DynMMXReg dst, U32 imm) {
    x86.psllq(X86Asm::XMM(dst), imm);
}

void JitX86CodeGen::psrlqMmx(DynMMXReg dst, U32 imm) {
    x86.psrlq(X86Asm::XMM(dst), imm);
}

void JitX86CodeGen::paddbMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.paddb(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::paddwMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.paddw(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::padddMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.paddd(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::paddsbMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.paddsb(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::paddswMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.paddsw(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::paddusbMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.paddusb(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::padduswMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.paddusw(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::psubbMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.psubb(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::psubwMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.psubw(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::psubdMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.psubd(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::psubsbMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.psubsb(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::psubswMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.psubsw(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::psubusbMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.psubusb(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::psubuswMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.psubusw(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::pmulhwMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.pmulhw(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::pmullwMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.pmullw(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::pmaddwdMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.pmaddwd(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::pcmpeqbMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.pcmpeqb(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::pcmpeqwMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.pcmpeqw(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::pcmpeqdMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.pcmpeqd(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::pcmpgtbMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.pcmpgtb(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::pcmpgtwMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.pcmpgtw(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::pcmpgtdMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.pcmpgtd(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::packsswbMmxMmx(DynMMXReg dst, DynMMXReg src) {
    // xmm is twice as big as mm, so we need to adjust a little before the pack
    x86.movlhps(X86Asm::XMM(dst), X86Asm::XMM(src));
    x86.packsswb(X86Asm::XMM(dst), X86Asm::XMM(dst));
}

void JitX86CodeGen::packssdwMmxMmx(DynMMXReg dst, DynMMXReg src) {
    // xmm is twice as big as mm, so we need to adjust a little before the pack
    x86.movlhps(X86Asm::XMM(dst), X86Asm::XMM(src));
    x86.packssdw(X86Asm::XMM(dst), X86Asm::XMM(dst));
}

void JitX86CodeGen::packuswbMmxMmx(DynMMXReg dst, DynMMXReg src) {
    // xmm is twice as big as mm, so we need to adjust a little before the pack
    x86.movlhps(X86Asm::XMM(dst), X86Asm::XMM(src));
    x86.packuswb(X86Asm::XMM(dst), X86Asm::XMM(dst));
}

void JitX86CodeGen::punpckhbwMmxMmx(DynMMXReg dst, DynMMXReg src) {
    // :TODO: maybe move bytes 4-7 to 8-11 instead of 0-7 to 8-15 so that we don't have to do the movhlps to mov them back down?
    x86.movlhps(X86Asm::XMM(src), X86Asm::XMM(src));
    x86.movlhps(X86Asm::XMM(dst), X86Asm::XMM(dst));
    x86.punpckhbw(X86Asm::XMM(dst), X86Asm::XMM(src));
    x86.movhlps(X86Asm::XMM(dst), X86Asm::XMM(dst));
}

void JitX86CodeGen::punpckhwdMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.movlhps(X86Asm::XMM(src), X86Asm::XMM(src));
    x86.movlhps(X86Asm::XMM(dst), X86Asm::XMM(dst));
    x86.punpckhwd(X86Asm::XMM(dst), X86Asm::XMM(src));
    x86.movhlps(X86Asm::XMM(dst), X86Asm::XMM(dst));
}

void JitX86CodeGen::punpckhdqMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.movlhps(X86Asm::XMM(src), X86Asm::XMM(src));
    x86.movlhps(X86Asm::XMM(dst), X86Asm::XMM(dst));
    x86.punpckhdq(X86Asm::XMM(dst), X86Asm::XMM(src));
    x86.movhlps(X86Asm::XMM(dst), X86Asm::XMM(dst));
}

void JitX86CodeGen::punpcklbwMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.punpcklbw(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::punpcklwdMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.punpcklwd(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::punpckldqMmxMmx(DynMMXReg dst, DynMMXReg src) {
    x86.punpckldq(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::addpdXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.addpd(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::addsdXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.addsd(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::subpdXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.subpd(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::subsdXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.subsd(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::mulpdXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.mulpd(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::mulsdXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.mulsd(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::divpdXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.divpd(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::divsdXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.divsd(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::maxpdXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.maxpd(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::maxsdXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.maxsd(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::minpdXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.minpd(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::minsdXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.minsd(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::paddbXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.paddb(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::paddwXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.paddw(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::padddXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.paddd(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::paddqXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.paddq(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::paddsbXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.paddsb(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::paddswXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.paddsw(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::paddusbXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.paddusb(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::padduswXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.paddusw(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::psubbXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.psubb(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::psubwXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.psubw(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::psubdXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.psubd(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::psubqXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.psubq(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::psubsbXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.psubsb(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::psubswXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.psubsw(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::psubusbXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.psubusb(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::psubuswXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.psubusw(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::pmaddwdXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.pmaddwd(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::pmulhwXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.pmulhw(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::pmullwXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.pmullw(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::pmuludqXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.pmuludq(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::sqrtpdXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.sqrtpd(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::sqrtsdXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.sqrtsd(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::andnpdXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.andnpd(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::andpdXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.andpd(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::pandXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.pand(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::pandnXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.pandn(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::porXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.por(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::pslldqXmm(DynXMMReg dst, U32 imm) {
    x86.pslldq(X86Asm::XMM(dst), imm);
}

void JitX86CodeGen::psllqXmm(DynXMMReg dst, U32 imm) {
    x86.psllq(X86Asm::XMM(dst), imm);
}

void JitX86CodeGen::pslldXmm(DynXMMReg dst, U32 imm) {
    x86.pslld(X86Asm::XMM(dst), imm);
}

void JitX86CodeGen::psllwXmm(DynXMMReg dst, U32 imm) {
    x86.psllw(X86Asm::XMM(dst), imm);
}

void JitX86CodeGen::psradXmm(DynXMMReg dst, U32 imm) {
    x86.psrad(X86Asm::XMM(dst), imm);
}

void JitX86CodeGen::psrawXmm(DynXMMReg dst, U32 imm) {
    x86.psraw(X86Asm::XMM(dst), imm);
}

void JitX86CodeGen::psrldqXmm(DynXMMReg dst, U32 imm) {
    x86.psrldq(X86Asm::XMM(dst), imm);
}

void JitX86CodeGen::psrlqXmm(DynXMMReg dst, U32 imm) {
    x86.psrlq(X86Asm::XMM(dst), imm);
}

void JitX86CodeGen::psrldXmm(DynXMMReg dst, U32 imm) {
    x86.psrld(X86Asm::XMM(dst), imm);
}

void JitX86CodeGen::psrlwXmm(DynXMMReg dst, U32 imm) {
    x86.psrlw(X86Asm::XMM(dst), imm);
}

void JitX86CodeGen::psllqXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.psllq(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::pslldXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.pslld(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::psllwXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.psllw(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::psradXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.psrad(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::psrawXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.psraw(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::psrlqXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.psrlq(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::psrldXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.psrld(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::psrlwXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.psrlw(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::pxorXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.pxor(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::orpdXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.orpd(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::xorpdXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.xorpd(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::cmppdXmmXmm(DynXMMReg dst, DynXMMReg src, U32 imm) {
    x86.cmppd(X86Asm::XMM(dst), X86Asm::XMM(src), (U8)imm);
}

void JitX86CodeGen::cmpsdXmmXmm(DynXMMReg dst, DynXMMReg src, U32 imm) {
    x86.cmpsd(X86Asm::XMM(dst), X86Asm::XMM(src), (U8)imm);
}

void JitX86CodeGen::comisdXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.comisd(X86Asm::XMM(dst), X86Asm::XMM(src));
    updateFlagsIfNecessary();
}

void JitX86CodeGen::ucomisdXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.ucomisd(X86Asm::XMM(dst), X86Asm::XMM(src));
    updateFlagsIfNecessary();
}

void JitX86CodeGen::pcmpgtbXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.pcmpgtb(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::pcmpgtwXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.pcmpgtw(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::pcmpgtdXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.pcmpgtd(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::pcmpeqbXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.pcmpeqb(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::pcmpeqwXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.pcmpeqw(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::pcmpeqdXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.pcmpeqd(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::cvtdq2pdXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.cvtdq2pd(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::cvtdq2psXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.cvtdq2ps(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::cvtpd2piMmxXmm(DynMMXReg dst, DynXMMReg src) {
    x86.cvtpd2dq(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::cvtpi2pdXmmMmx(DynXMMReg dst, DynMMXReg src) {
    x86.cvtdq2pd(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::cvtpd2dqXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.cvtpd2dq(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::cvtpd2psXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.cvtpd2ps(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::cvttpd2piMmxXmm(DynMMXReg dst, DynXMMReg src) {
    x86.cvttpd2dq(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::cvtps2dqXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.cvtps2dq(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::cvtps2pdXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.cvtps2pd(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::cvtsd2siR32Xmm(RegPtr dst, DynXMMReg src) {
    x86.cvtsd2si(R32(dst->hardwareReg()), X86Asm::XMM(src));
}

void JitX86CodeGen::cvtsd2ssXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.cvtsd2ss(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::cvtsi2sdXmmR32(DynXMMReg dst, RegPtr src) {
    x86.cvtsi2sd(X86Asm::XMM(dst), R32(src->hardwareReg()));
}

void JitX86CodeGen::cvtss2sdXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.cvtss2sd(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::cvttpd2dqXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.cvttpd2dq(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::cvttps2dqXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.cvttps2dq(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::cvttsd2siR32Xmm(RegPtr dst, DynXMMReg src) {
    x86.cvttsd2si(R32(dst->hardwareReg()), X86Asm::XMM(src));
}

void JitX86CodeGen::movsdXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.movsd(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::movupdXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.movdqu(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::movmskpd(RegPtr dst, DynXMMReg src) {
    x86.movmskpd(R32(dst->hardwareReg()), X86Asm::XMM(src));
}

void JitX86CodeGen::movd(RegPtr dst, DynXMMReg src) {
    x86.movd(R32(dst->hardwareReg()), X86Asm::XMM(src));
}

void JitX86CodeGen::movd(DynXMMReg dst, RegPtr src) {
    x86.movd(X86Asm::XMM(dst), R32(src->hardwareReg()));
}

void JitX86CodeGen::movdq2q(DynMMXReg dst, DynXMMReg src) {
    x86.movq(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::movq2dq(DynXMMReg dst, DynMMXReg src) {
    x86.movq(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::maskmovdqu(DynXMMReg src, DynXMMReg mask, RegPtr address) {
    x86.push(RN(7));
    x86.mov(RN(7), RN(address->hardwareReg()));
    x86.maskmovdqu(X86Asm::XMM(src), X86Asm::XMM(mask));
    x86.pop(RN(7));
}

void JitX86CodeGen::pshufdXmmXmm(DynXMMReg dst, DynXMMReg src, U32 imm) {
    x86.pshufd(X86Asm::XMM(dst), X86Asm::XMM(src), imm);
}

void JitX86CodeGen::pshufhwXmmXmm(DynXMMReg dst, DynXMMReg src, U32 imm) {
    x86.pshufhw(X86Asm::XMM(dst), X86Asm::XMM(src), imm);
}

void JitX86CodeGen::pshuflwXmmXmm(DynXMMReg dst, DynXMMReg src, U32 imm) {
    x86.pshuflw(X86Asm::XMM(dst), X86Asm::XMM(src), imm);
}

void JitX86CodeGen::shufpdXmmXmm(DynXMMReg dst, DynXMMReg src, U32 imm) {
    x86.shufpd(X86Asm::XMM(dst), X86Asm::XMM(src), imm);
}

void JitX86CodeGen::unpckhpdXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.unpckhpd(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::unpcklpdXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.unpcklpd(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::punpckhbwXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.punpckhbw(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::punpckhwdXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.punpckhwd(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::punpckhdqXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.punpckhdq(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::punpckhqdqXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.punpckhqdq(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::punpcklbwXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.punpcklbw(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::punpcklwdXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.punpcklwd(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::punpckldqXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.punpckldq(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::punpcklqdqXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.punpcklqdq(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::packssdwXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.packssdw(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::packsswbXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.packsswb(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::packuswbXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.packuswb(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::pavgbXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.pavgb(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::pavgwXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.pavgw(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::psadbwXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.psadbw(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::pmaxswXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.pmaxsw(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::pmaxubXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.pmaxub(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::pminswXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.pminsw(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::pminubXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.pminub(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::pmulhuwXmmXmm(DynXMMReg dst, DynXMMReg src) {
    x86.pmulhuw(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::lfence() {
    x86.lfence();
}

void JitX86CodeGen::mfence() {
    x86.mfence();
}

void JitX86CodeGen::clflush(RegPtr rm, RegPtr sib, U8 lsl, U32 disp) {
    x86.clflush(X86Asm::Mem8(R32(rm->hardwareReg()), R32(sib->hardwareReg()), lsl, disp));
}

void JitX86CodeGen::pause() {
    x86.pause();
}

void JitX86CodeGen::pextrwR32Xmm(RegPtr dst, DynXMMReg src, U32 imm) {
    x86.pextrw(R32(dst->hardwareReg()), X86Asm::XMM(src), (U8)imm);
}

void JitX86CodeGen::pinsrwXmmR32(DynXMMReg dst, RegPtr src, U32 imm) {
    x86.pinsrw(X86Asm::XMM(dst), R32(src->hardwareReg()), (U8)imm);
}

void JitX86CodeGen::pmovmskbR32Xmm(RegPtr dst, DynXMMReg src) {
    x86.pmovmskb(R32(dst->hardwareReg()), X86Asm::XMM(src));
}

void JitX86CodeGen::updateFPURounding() {
    x86.stmxcsr(X86Asm::Mem32(HOST_CPU, offsetof(CPU, sseControlStateTmp)));

    RegPtr sse = readCPU(JitWidth::b32, offsetof(CPU, sseControlStateTmp));
    RegPtr fpu = readCPU(JitWidth::b32, offsetof(CPU, fpu.round));

    andValue(JitWidth::b32, sse, ~0x6000); // clear rounding
    shlValue(JitWidth::b32, fpu, 13);
    orReg(JitWidth::b32, sse, fpu); // set rounding in SSE

    // there is no way to set sse rounding from a register
    writeCPU(JitWidth::b32, offsetof(CPU, sseControlStateTmp2), sse);

    x86.ldmxcsr(X86Asm::Mem32(HOST_CPU, offsetof(CPU, sseControlStateTmp2)));
}

void JitX86CodeGen::restoreFPURounding() {
    x86.ldmxcsr(X86Asm::Mem32(HOST_CPU, offsetof(CPU, sseControlStateTmp)));
};

void JitX86CodeGen::storeCpuFpuReg(DynFpuReg reg, RegPtr index, DynFpuWidth width) {
    x86.movsd(X86Asm::Mem64(HOST_CPU, R32(index->hardwareReg()), 3, offsetof(CPU, fpu.regCache[0].d)), X86Asm::XMM(reg));
}

void JitX86CodeGen::loadCpuFpuReg(DynFpuReg reg, RegPtr index, DynFpuWidth width) {
    x86.movsd(X86Asm::XMM(reg), X86Asm::Mem64(HOST_CPU, R32(index->hardwareReg()), 3, offsetof(CPU, fpu.regCache[0].d)));
}

void JitX86CodeGen::loadCpuFpuRegConst(DynFpuReg reg, U32 offset) {
    x86.movsd(X86Asm::XMM(reg), X86Asm::Mem64(HOST_CPU, offset));
}

RegPtr JitX86CodeGen::fpuRegToInt32(DynFpuReg fpuRegSrc, bool truncate) {
    RegPtr result = getTmpReg();
    if (truncate) {
        x86.cvttsd2si(R32(result->hardwareReg()), X86Asm::XMM(fpuRegSrc));
    } else {
        x86.cvtsd2si(R32(result->hardwareReg()), X86Asm::XMM(fpuRegSrc));
    }
    return result;
}

void JitX86CodeGen::fpuRegToInt64(DynFpuReg regDst, DynFpuReg fpuRegSrc, bool truncate) {
    if (truncate) {
        x86.cvttpd2dq(X86Asm::XMM(regDst), X86Asm::XMM(fpuRegSrc));
    } else {
        x86.cvtpd2dq(X86Asm::XMM(regDst), X86Asm::XMM(fpuRegSrc));
    }
}

void JitX86CodeGen::fpuRegInt64To64(DynFpuReg regDst, DynFpuReg fpuRegSrc) {
    x86.cvtdq2pd(X86Asm::XMM(regDst), X86Asm::XMM(fpuRegSrc));
}

void JitX86CodeGen::storeFpuReg(DynFpuReg reg, RegPtr rm, RegPtr sib, U8 lsl, U32 disp, DynFpuWidth width) {
    if (width == DYN_FPU_64_BIT) {
        x86.movsd(X86Asm::Mem64(R32(rm->hardwareReg()), R32(sib->hardwareReg()), lsl, disp), X86Asm::XMM(reg));
    } else {
        x86.movss(X86Asm::Mem32(R32(rm->hardwareReg()), R32(sib->hardwareReg()), lsl, disp), X86Asm::XMM(reg));
    }
}

void JitX86CodeGen::loadFpuReg(DynFpuReg reg, RegPtr rm, RegPtr sib, U8 lsl, U32 disp, DynFpuWidth width) {
    if (width == DYN_FPU_64_BIT) {
        x86.movsd(X86Asm::XMM(reg), X86Asm::Mem64(R32(rm->hardwareReg()), R32(sib->hardwareReg()), lsl, disp));
    } else {
        x86.movss(X86Asm::XMM(reg), X86Asm::Mem32(R32(rm->hardwareReg()), R32(sib->hardwareReg()), lsl, disp));
    }
}

void JitX86CodeGen::fpuRegExtend32To64(DynFpuReg dst, DynFpuReg src) {
    x86.cvtss2sd(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::fpuReg64To32(DynFpuReg dst, DynFpuReg src) {
    x86.cvtsd2ss(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::loadFpuRegFromInt(DynFpuReg reg, RegPtr rm, RegPtr sib, U8 lsl, U32 disp) {
    x86.cvtsi2sd(X86Asm::XMM(reg), X86Asm::Mem32(R32(rm->hardwareReg()), R32(sib->hardwareReg()), lsl, disp));
}

void JitX86CodeGen::regToFpuReg(DynFpuReg dst, RegPtr src) {
    x86.cvtsi2sd(X86Asm::XMM(dst), R32(src->hardwareReg()));
}

void JitX86CodeGen::fpuAdd(DynFpuReg dst, DynFpuReg src) {
    x86.addsd(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::fpuMul(DynFpuReg dst, DynFpuReg src) {
    x86.mulsd(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::fpuSub(DynFpuReg dst, DynFpuReg src) {
    x86.subsd(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::fpuDiv(DynFpuReg dst, DynFpuReg src) {
    x86.divsd(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::fpuXor(DynFpuReg dst, DynFpuReg src) {
    x86.xorpd(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::fpuAnd(DynFpuReg dst, DynFpuReg src) {
    x86.andpd(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::fpuSqrt(DynFpuReg dst, DynFpuReg src) {
    x86.sqrtsd(X86Asm::XMM(dst), X86Asm::XMM(src));
}

void JitX86CodeGen::fcompare(DynFpuReg fpuReg1, DynFpuReg fpuReg2, RegPtr ordTags, const std::function<void()>& pfnEqual, const std::function<void()>& pfnLessThan, const std::function<void()>& pfnGreaterThan, const std::function<void()>& pfnInvalid) {
    subValue(JitWidth::b8, ordTags, TAG_Empty);
    IfNot(JitWidth::b8, ordTags);
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

U32 JitX86CodeGen::getBufferSize() {
    return (U32)x86.buffer.size();
}

U8* JitX86CodeGen::getBuffer() {
    return x86.buffer.data();
}


U32 JitX86CodeGen::getIfJumpSize() {
    return (U32)x86.ifJump.size();
}

void writeBlockExitForJIT(U8* buffer) {
    X86Asm x86;

    // writeCache
    for (int i = 0; i < 8; i++) {
        if (regCache[i] != INVALID_REG) {
            x86.mov(X86Asm::Mem32(HOST_CPU, CPU::offsetofReg32(i)), R32(regCache[i]));
        }
    }

#ifdef BOXEDWINE_64
    x86.pop(x86.r15);
    x86.pop(x86.r14);
    x86.pop(x86.r13);
    x86.pop(x86.r12);    
    x86.pop(x86.rdi);
    x86.pop(x86.rsi);
    x86.pop(x86.rbp);
    x86.pop(x86.rbx);
#else
    x86.pop(x86.ebp);
    x86.pop(x86.esi);
    x86.pop(x86.edi);
    x86.pop(x86.ebx);
#endif
    x86.ret();
    memcpy(buffer, x86.buffer.data(), x86.buffer.size());
}

void JitX86CodeGen::blockExit(bool syncCache) {
    if (syncCache) {
        writeCache();
    }
#ifdef BOXEDWINE_64
    x86.pop(x86.r15);
    x86.pop(x86.r14);
    x86.pop(x86.r13);
    x86.pop(x86.r12);    
    x86.pop(x86.rdi);
    x86.pop(x86.rsi);
    x86.pop(x86.rbp);
    x86.pop(x86.rbx);
#else
    x86.pop(x86.ebp);
    x86.pop(x86.esi);
    x86.pop(x86.edi);
    x86.pop(x86.ebx);
#endif
    x86.ret();
}

void JitX86CodeGen::incrementEip(U32 inc) {
    x86.add(X86Asm::Mem32(HOST_CPU, offsetof(CPU, eip.u32)), inc);
}

void JitX86CodeGen::setCC(X86Asm::Reg32 reg, JitEvaluate condition) {

    switch (condition) {
    case JitEvaluate::EQUALS:
        x86.setz(R8(reg.reg));
        break;
    case JitEvaluate::NOT_EQUALS:
        x86.setnz(R8(reg.reg));
        break;
    case JitEvaluate::LESS_THAN_UNSIGNED:
        x86.setb(R8(reg.reg));
        break;
    case JitEvaluate::LESS_THAN_EQUAL_UNSIGNED:
        x86.setbe(R8(reg.reg));
        break;
    case JitEvaluate::GREATER_THAN_UNSIGNED:
        x86.setnbe(R8(reg.reg));
        break;
    case JitEvaluate::GREATER_THAN_EQUAL_UNSIGNED:
        x86.setnb(R8(reg.reg));
        break;    
    case JitEvaluate::LESS_THAN_SIGNED:
        x86.setl(R8(reg.reg));
        break;
    case JitEvaluate::GREATER_THAN_EQUAL_SIGNED:
        x86.setnl(R8(reg.reg));
        break;
    case JitEvaluate::LESS_THAN_EQUAL_SIGNED:
        x86.setle(R8(reg.reg));
        break;
    case JitEvaluate::GREATER_THAN_SIGNED:
        x86.setnle(R8(reg.reg));
        break;
    // no default, should get compiler error if not all enum cases handled
    }
}

void JitX86CodeGen::JumpInBlock(U32 address) {
    x86.jmp(address);
}

void JitX86CodeGen::StartElse() {
    x86.Else();
}

void JitX86CodeGen::EndIf() {
    x86.EndIf();
}

void JitX86CodeGen::dynamic_rdtsc(DecodedOp* op) {
    x86.rdtsc();
    getReg(0, -1, false); // will store EAX
    getReg(2, -1, false); // will store EDX
    incrementEip(op->len);
}

void JitX86CodeGen::updateHardwareFlags(U32 flags) {
    fillFlags();

    if (flags == CF) {
        x86.bt(X86Asm::Mem32(HOST_CPU, offsetof(CPU, flags)), 0);
        return;
    }
    bool eaxPushed = false;

#ifdef BOXEDWINE_64
    x86.push(RN(0));
    eaxPushed = true;
    regUsed2[0] = false;
    RegPtr reg = getReg(0);
#else
    if (!isTmpRegAvailable()) {
        x86.push(RN(0));
        eaxPushed = true;
        regUsed2[0] = false;
    }
    RegPtr reg = getTmpRegForCallResult();
#endif
    x86.mov(R32(reg->hardwareReg()), X86Asm::Mem32(HOST_CPU, offsetof(CPU, flags)));
    if (reg->hardwareReg() > 3) {
        kpanic("updateHardwareFlags");
    }
    x86.xchg(R8(reg->hardwareReg()), R8(reg->hardwareReg() + 4));
    if (flags & OF) {
        x86.shr(R8(reg->hardwareReg()), 3);
    }
    if (reg->hardwareReg() == 0) {
        if (flags & OF) {
            x86.add(x86.al, 127); // (will restore OF)
        }
        x86.sahf();
    } else {
        x86.xchg(R32(reg->hardwareReg()), x86.eax);
        if (flags & OF) {
            x86.add(x86.al, 127); // (will restore OF)
        }
        x86.sahf();
        x86.xchg(R32(reg->hardwareReg()), x86.eax);
    }
    reg = nullptr;
    if (eaxPushed) {
        x86.pop(RN(0));
        regUsed2[0] = true;
    }
}

void JitX86CodeGen::dynamic_cmpxchg8b_lock(DecodedOp* op) {
    JitCodeGen::write(JitWidth::b64, calculateEaa(op), nullptr, [op, this](RegPtr addressReg, RegPtr offsetReg) {
        if (currentOp->getNeededFlagsAfter(PF | SF | AF | CF | OF)) { // The ZF flag is set if the destination operand and EDX:EAX are equal; otherwise it is cleared. The CF, PF, AF, SF, and OF flags are unaffected.
            updateHardwareFlags(PF | SF | AF | CF | OF);
        }
        writeCache();
        this->x86.mov(x86.ebp, R32(addressReg->hardwareReg()));
        this->x86.mov(x86.esi, R32(offsetReg->hardwareReg()));
        for (int i = 0; i < 4; i++) {
            x86.mov(R32(i), X86Asm::Mem32(HOST_CPU, CPU::offsetofReg32(i)));
        }
        this->x86.lock();
        this->x86.cmpxchg8b(X86Asm::Mem64(x86.esi, x86.ebp));
        for (int i = 0; i < 4; i++) {
            x86.mov(X86Asm::Mem32(HOST_CPU, CPU::offsetofReg32(i)), R32(i));
        }
        loadCache();
        updateFlagsIfNecessary();
        incrementEip(op->len);
    });
}
void JitX86CodeGen::dynamic_cmpxchge32r32_lock(DecodedOp* op) {
    JitCodeGen::write(JitWidth::b32, calculateEaa(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        RegPtr eax = getReg(0, 0);
        if (eax->hardwareReg() != 0) {
            kpanic("JitX86CodeGen::dynamic_cmpxchge32r32_lock");
        }
        RegPtr reg;

        if (op->reg == 0) {
            reg = eax;
        } else {
            reg = getReadOnlyReg(op->reg);
        }
        this->x86.lock();
        this->x86.cmpxchg(X86Asm::Mem32(R32(address->hardwareReg()), R32(offset->hardwareReg())), R32(reg->hardwareReg()));
        updateFlagsIfNecessary();
        reg = nullptr;
        eax = nullptr;        
        incrementEip(op->len);
    });
}
void JitX86CodeGen::dynamic_cmpxchge16r16_lock(DecodedOp* op) {
    JitCodeGen::write(JitWidth::b16, calculateEaa(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        RegPtr eax = getReg(0, 0);
        if (eax->hardwareReg() != 0) {
            kpanic("JitX86CodeGen::dynamic_cmpxchge32r32_lock");
        }
        RegPtr reg;

        if (op->reg == 0) {
            reg = eax;
        } else {
            reg = getReadOnlyReg(op->reg);
        }
        this->x86.lock();
        this->x86.cmpxchg(X86Asm::Mem16(R32(address->hardwareReg()), R32(offset->hardwareReg())), R16(reg->hardwareReg()));
        updateFlagsIfNecessary();
        reg = nullptr;
        eax = nullptr;        
        incrementEip(op->len);
    });
}
void JitX86CodeGen::dynamic_cmpxchge8r8_lock(DecodedOp* op) {
    JitCodeGen::write(JitWidth::b8, calculateEaa(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        RegPtr eax = getReg(0, 0);
        if (eax->hardwareReg() != 0) {
            kpanic("JitX86CodeGen::dynamic_cmpxchge32r32_lock");
        }
        RegPtr reg;

        if (op->reg == 0) {
            reg = eax;
        } else {
            reg = getReadOnlyReg8(op->reg);
        }
        this->x86.lock();
        this->x86.cmpxchg(X86Asm::Mem8(R32(address->hardwareReg()), R32(offset->hardwareReg())), R8(get8bitReg(reg)));
        updateFlagsIfNecessary();
        reg = nullptr;
        eax = nullptr;        
        incrementEip(op->len);
    });
}

void JitX86CodeGen::dynamic_xchge32r32_lock(DecodedOp* op) {
    JitCodeGen::write(JitWidth::b32, calculateEaa(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        RegPtr reg = getReg(op->reg);
        this->x86.lock();
        this->x86.xchg(R32(reg->hardwareReg()), X86Asm::Mem32(R32(address->hardwareReg()), R32(offset->hardwareReg())));        
        reg = nullptr;
        incrementEip(op->len);
    });
}

void JitX86CodeGen::dynamic_xchge16r16_lock(DecodedOp* op) {
    JitCodeGen::write(JitWidth::b16, calculateEaa(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        RegPtr reg = getReg(op->reg);
        this->x86.lock();
        this->x86.xchg(R16(reg->hardwareReg()), X86Asm::Mem16(R32(address->hardwareReg()), R32(offset->hardwareReg())));        
        reg = nullptr;
        incrementEip(op->len);
    });
}

void JitX86CodeGen::dynamic_xchge8r8_lock(DecodedOp* op) {
    JitCodeGen::write(JitWidth::b8, calculateEaa(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        RegPtr reg = getReg8(op->reg);
        this->x86.lock();
        this->x86.xchg(R8(get8bitReg(reg)), X86Asm::Mem8(R32(address->hardwareReg()), R32(offset->hardwareReg())));        
        reg = nullptr;
        incrementEip(op->len);
    });
}

void JitX86CodeGen::dynamic_arithE32R32_lock(DecodedOp* op, std::function<void(RegPtr dest, RegPtr address, RegPtr offset)> callback, bool writeReg) {
    JitCodeGen::write(JitWidth::b32, calculateEaa(op), nullptr, [writeReg, op, callback, this](RegPtr address, RegPtr offset) {
        RegPtr reg;
        
        if (writeReg) {
            reg = getReg(op->reg);
        } else {
            reg = getReadOnlyReg(op->reg);
        }
        callback(reg, address, offset);        
        updateFlagsIfNecessary();
        reg = nullptr;
        incrementEip(op->len);
    });
}

void JitX86CodeGen::dynamic_arithE16R16_lock(DecodedOp* op, std::function<void(RegPtr dest, RegPtr address, RegPtr offset)> callback, bool writeReg) {
    JitCodeGen::write(JitWidth::b16, calculateEaa(op), nullptr, [writeReg, op, callback, this](RegPtr address, RegPtr offset) {
        RegPtr reg;

        if (writeReg) {
            reg = getReg(op->reg);
        } else {
            reg = getReadOnlyReg(op->reg);
        }
        callback(reg, address, offset);        
        updateFlagsIfNecessary();
        reg = nullptr;
        incrementEip(op->len);
    });
}
void JitX86CodeGen::dynamic_arithE8R8_lock(DecodedOp* op, std::function<void(RegPtr dest, RegPtr address, RegPtr offset)> callback, bool writeReg) {
    JitCodeGen::write(JitWidth::b8, calculateEaa(op), nullptr, [writeReg, op, callback, this](RegPtr address, RegPtr offset) {
        RegPtr reg;

        if (writeReg) {
            reg = getReg8(op->reg);
        } else {
            reg = getReadOnlyReg8(op->reg);
        }
        callback(reg, address, offset);        
        updateFlagsIfNecessary();
        reg = nullptr;
        incrementEip(op->len);
    });
}

void JitX86CodeGen::dynamic_arithE32_lock(DecodedOp* op, std::function<void(RegPtr address, RegPtr offset)> callback) {
    JitCodeGen::write(JitWidth::b32, calculateEaa(op), nullptr, [op, callback, this](RegPtr address, RegPtr offset) {
        callback(address, offset);
        updateFlagsIfNecessary();
        incrementEip(op->len);
    });
}

void JitX86CodeGen::dynamic_arithE16_lock(DecodedOp* op, std::function<void(RegPtr address, RegPtr offset)> callback) {
    JitCodeGen::write(JitWidth::b16, calculateEaa(op), nullptr, [op, callback, this](RegPtr address, RegPtr offset) {
        callback(address, offset);
        updateFlagsIfNecessary();
        incrementEip(op->len);
    });
}
void JitX86CodeGen::dynamic_arithE8_lock(DecodedOp* op, std::function<void(RegPtr address, RegPtr offset)> callback) {
    JitCodeGen::write(JitWidth::b8, calculateEaa(op), nullptr, [op, callback, this](RegPtr address, RegPtr offset) {
        callback(address, offset);
        updateFlagsIfNecessary();
        incrementEip(op->len);
    });
}

void JitX86CodeGen::dynamic_xaddr32e32_lock(DecodedOp* op) {
    dynamic_arithE32R32_lock(op, [this](RegPtr dest, RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.xadd(X86Asm::Mem32(R32(addressReg->hardwareReg()), R32(offsetReg->hardwareReg())), R32(dest->hardwareReg()));
    }, true);
}

void JitX86CodeGen::dynamic_xaddr16e16_lock(DecodedOp* op) {
    dynamic_arithE16R16_lock(op, [this](RegPtr dest, RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.xadd(X86Asm::Mem16(R32(addressReg->hardwareReg()), R32(offsetReg->hardwareReg())), R16(dest->hardwareReg()));
    }, true);
}
void JitX86CodeGen::dynamic_xaddr8e8_lock(DecodedOp* op) {
    dynamic_arithE8R8_lock(op, [this](RegPtr dest, RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.xadd(X86Asm::Mem8(R32(addressReg->hardwareReg()), R32(offsetReg->hardwareReg())), R8(get8bitReg(dest)));
    }, true);
}

void JitX86CodeGen::dynamic_adde32r32_lock(DecodedOp* op) {
    dynamic_arithE32R32_lock(op, [this](RegPtr dest, RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.add(X86Asm::Mem32(R32(addressReg->hardwareReg()), R32(offsetReg->hardwareReg())), R32(dest->hardwareReg()));
    });
}
void JitX86CodeGen::dynamic_adde16r16_lock(DecodedOp* op) {
    dynamic_arithE16R16_lock(op, [this](RegPtr dest, RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.add(X86Asm::Mem16(R32(addressReg->hardwareReg()), R32(offsetReg->hardwareReg())), R16(dest->hardwareReg()));
    });
}
void JitX86CodeGen::dynamic_adde8r8_lock(DecodedOp* op) {
    dynamic_arithE8R8_lock(op, [this](RegPtr dest, RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.add(X86Asm::Mem8(R32(addressReg->hardwareReg()), R32(offsetReg->hardwareReg())), R8(get8bitReg(dest)));
    });
}
void JitX86CodeGen::dynamic_add32_mem_lock(DecodedOp* op) {
    dynamic_arithE32_lock(op, [op, this](RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.add(X86Asm::Mem32(R32(addressReg->hardwareReg()), R32(offsetReg->hardwareReg())), op->imm);
    });
}
void JitX86CodeGen::dynamic_add16_mem_lock(DecodedOp* op) {
    dynamic_arithE16_lock(op, [op, this](RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.add(X86Asm::Mem16(R32(addressReg->hardwareReg()), R32(offsetReg->hardwareReg())), (U16)op->imm);
    });
}
void JitX86CodeGen::dynamic_add8_mem_lock(DecodedOp* op) {
    dynamic_arithE8_lock(op, [op, this](RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.add(X86Asm::Mem8(R32(addressReg->hardwareReg()), R32(offsetReg->hardwareReg())), op->imm);
    });
}

void JitX86CodeGen::dynamic_sube32r32_lock(DecodedOp* op) {
    dynamic_arithE32R32_lock(op, [this](RegPtr dest, RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.sub(X86Asm::Mem32(R32(addressReg->hardwareReg()), R32(offsetReg->hardwareReg())), R32(dest->hardwareReg()));
    });
}
void JitX86CodeGen::dynamic_sube16r16_lock(DecodedOp* op) {
    dynamic_arithE16R16_lock(op, [this](RegPtr dest, RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.sub(X86Asm::Mem16(R32(addressReg->hardwareReg()), R32(offsetReg->hardwareReg())), R16(dest->hardwareReg()));
    });
}
void JitX86CodeGen::dynamic_sube8r8_lock(DecodedOp* op) {
    dynamic_arithE8R8_lock(op, [this](RegPtr dest, RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.sub(X86Asm::Mem8(R32(addressReg->hardwareReg()), R32(offsetReg->hardwareReg())), R8(get8bitReg(dest)));
    });
}
void JitX86CodeGen::dynamic_sub32_mem_lock(DecodedOp* op) {
    dynamic_arithE32_lock(op, [op, this](RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.sub(X86Asm::Mem32(R32(addressReg->hardwareReg()), R32(offsetReg->hardwareReg())), op->imm);
    });
}
void JitX86CodeGen::dynamic_sub16_mem_lock(DecodedOp* op) {
    dynamic_arithE16_lock(op, [op, this](RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.sub(X86Asm::Mem16(R32(addressReg->hardwareReg()), R32(offsetReg->hardwareReg())), (U16)op->imm);
    });
}
void JitX86CodeGen::dynamic_sub8_mem_lock(DecodedOp* op) {
    dynamic_arithE8_lock(op, [op, this](RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.sub(X86Asm::Mem8(R32(addressReg->hardwareReg()), R32(offsetReg->hardwareReg())), (U8)op->imm);
    });
}

void JitX86CodeGen::dynamic_inc32_mem32_lock(DecodedOp* op) {
    dynamic_arithE32_lock(op, [op, this](RegPtr addressReg, RegPtr offsetReg) {
        if (op->getNeededFlagsAfter(CF)) {
            updateHardwareFlags(CF);
        }
        this->x86.lock();
        this->x86.inc(X86Asm::Mem32(R32(addressReg->hardwareReg()), R32(offsetReg->hardwareReg())));
    });
}
void JitX86CodeGen::dynamic_inc16_mem16_lock(DecodedOp* op) {
    dynamic_arithE16_lock(op, [op, this](RegPtr addressReg, RegPtr offsetReg) {
        if (currentOp->getNeededFlagsAfter(CF)) {
            updateHardwareFlags(CF);
        }
        this->x86.lock();
        this->x86.inc(X86Asm::Mem16(R32(addressReg->hardwareReg()), R32(offsetReg->hardwareReg())));
    });
}
void JitX86CodeGen::dynamic_inc8_mem8_lock(DecodedOp* op) {
    dynamic_arithE8_lock(op, [op, this](RegPtr addressReg, RegPtr offsetReg) {
        if (currentOp->getNeededFlagsAfter(CF)) {
            updateHardwareFlags(CF);
        }
        this->x86.lock();
        this->x86.inc(X86Asm::Mem8(R32(addressReg->hardwareReg()), R32(offsetReg->hardwareReg())));
    });
}
void JitX86CodeGen::dynamic_dec32_mem32_lock(DecodedOp* op) {
    dynamic_arithE32_lock(op, [op, this](RegPtr addressReg, RegPtr offsetReg) {
        if (currentOp->getNeededFlagsAfter(CF)) {
            updateHardwareFlags(CF);
        }
        this->x86.lock();
        this->x86.dec(X86Asm::Mem32(R32(addressReg->hardwareReg()), R32(offsetReg->hardwareReg())));
    });
}
void JitX86CodeGen::dynamic_dec16_mem16_lock(DecodedOp* op) {
    dynamic_arithE16_lock(op, [op, this](RegPtr addressReg, RegPtr offsetReg) {
        if (currentOp->getNeededFlagsAfter(CF)) {
            updateHardwareFlags(CF);
        }
        this->x86.lock();
        this->x86.dec(X86Asm::Mem16(R32(addressReg->hardwareReg()), R32(offsetReg->hardwareReg())));
    });
}
void JitX86CodeGen::dynamic_dec8_mem8_lock(DecodedOp* op) {
    dynamic_arithE8_lock(op, [op, this](RegPtr addressReg, RegPtr offsetReg) {
        if (currentOp->getNeededFlagsAfter(CF)) {
            updateHardwareFlags(CF);
        }
        this->x86.lock();
        this->x86.dec(X86Asm::Mem8(R32(addressReg->hardwareReg()), R32(offsetReg->hardwareReg())));
    });
}

void JitX86CodeGen::dynamic_note32_lock(DecodedOp* op) {
    dynamic_arithE32_lock(op, [op, this](RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.not_(X86Asm::Mem32(R32(addressReg->hardwareReg()), R32(offsetReg->hardwareReg())));
    });
}

void JitX86CodeGen::dynamic_note16_lock(DecodedOp* op) {
    dynamic_arithE16_lock(op, [op, this](RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.not_(X86Asm::Mem16(R32(addressReg->hardwareReg()), R32(offsetReg->hardwareReg())));
    });
}

void JitX86CodeGen::dynamic_note8_lock(DecodedOp* op) {
    dynamic_arithE8_lock(op, [op, this](RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.not_(X86Asm::Mem8(R32(addressReg->hardwareReg()), R32(offsetReg->hardwareReg())));
    });
}

void JitX86CodeGen::dynamic_nege32_lock(DecodedOp* op) {
    dynamic_arithE32_lock(op, [op, this](RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.neg(X86Asm::Mem32(R32(addressReg->hardwareReg()), R32(offsetReg->hardwareReg())));
    });
}

void JitX86CodeGen::dynamic_nege16_lock(DecodedOp* op) {
    dynamic_arithE16_lock(op, [op, this](RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.neg(X86Asm::Mem16(R32(addressReg->hardwareReg()), R32(offsetReg->hardwareReg())));
    });
}

void JitX86CodeGen::dynamic_nege8_lock(DecodedOp* op) {
    dynamic_arithE8_lock(op, [op, this](RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.neg(X86Asm::Mem8(R32(addressReg->hardwareReg()), R32(offsetReg->hardwareReg())));
    });
}

void JitX86CodeGen::dynamic_btse32_lock(DecodedOp* op) {
    JitCodeGen::write(JitWidth::b32, calculateEaa(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        // imm is the mask, not the bitshift
        U8 imm = std::countr_zero(op->imm);
        this->x86.lock();
        this->x86.bts(X86Asm::Mem32(R32(address->hardwareReg()), R32(offset->hardwareReg())), (U8)imm);
        updateFlagsIfNecessary();
        incrementEip(op->len);
    });
}

void JitX86CodeGen::dynamic_btse16_lock(DecodedOp* op) {
    JitCodeGen::write(JitWidth::b16, calculateEaa(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        // imm is the mask, not the bitshift
        U8 imm = std::countr_zero(op->imm);
        this->x86.lock();
        this->x86.bts(X86Asm::Mem16(R32(address->hardwareReg()), R32(offset->hardwareReg())), (U8)imm);
        updateFlagsIfNecessary();
        incrementEip(op->len);
    });
}

void JitX86CodeGen::dynamic_btse32r32_lock(DecodedOp* op) {
    JitCodeGen::write(JitWidth::b32, calculateEffectiveEaa32(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        RegPtr reg = getTmpReg(op->reg);
        andValue(JitWidth::b32, reg, 0x1f);
        this->x86.lock();
        this->x86.bts(X86Asm::Mem32(R32(address->hardwareReg()), R32(offset->hardwareReg())), R32(reg->hardwareReg()));
        updateFlagsIfNecessary();
        incrementEip(op->len);
    });
}

void JitX86CodeGen::dynamic_btse16r16_lock(DecodedOp* op) {
    JitCodeGen::write(JitWidth::b16, calculateEffectiveEaa16(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        RegPtr reg = getTmpReg(op->reg);
        andValue(JitWidth::b16, reg, 0xf);
        this->x86.lock();
        this->x86.bts(X86Asm::Mem16(R32(address->hardwareReg()), R32(offset->hardwareReg())), R16(reg->hardwareReg()));
        updateFlagsIfNecessary();
        incrementEip(op->len);
    });
}

void JitX86CodeGen::dynamic_btre32_lock(DecodedOp* op) {
    JitCodeGen::write(JitWidth::b32, calculateEaa(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        // imm is the mask, not the bitshift
        U8 imm = std::countr_zero(op->imm);
        this->x86.lock();
        this->x86.btr(X86Asm::Mem32(R32(address->hardwareReg()), R32(offset->hardwareReg())), (U8)imm);
        updateFlagsIfNecessary();
        incrementEip(op->len);
    });
}
void JitX86CodeGen::dynamic_btre16_lock(DecodedOp* op) {
    JitCodeGen::write(JitWidth::b16, calculateEaa(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        // imm is the mask, not the bitshift
        U8 imm = std::countr_zero(op->imm);
        this->x86.lock();
        this->x86.btr(X86Asm::Mem16(R32(address->hardwareReg()), R32(offset->hardwareReg())), (U8)imm);
        updateFlagsIfNecessary();
        incrementEip(op->len);
    });
}
void JitX86CodeGen::dynamic_btre32r32_lock(DecodedOp* op) {
    JitCodeGen::write(JitWidth::b32, calculateEffectiveEaa32(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        RegPtr reg = getTmpReg(op->reg);
        andValue(JitWidth::b32, reg, 0x1f);
        this->x86.lock();
        this->x86.btr(X86Asm::Mem32(R32(address->hardwareReg()), R32(offset->hardwareReg())), R32(reg->hardwareReg()));
        updateFlagsIfNecessary();
        incrementEip(op->len);
    });
}
void JitX86CodeGen::dynamic_btre16r16_lock(DecodedOp* op) {
    JitCodeGen::write(JitWidth::b16, calculateEffectiveEaa16(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        RegPtr reg = getTmpReg(op->reg);
        andValue(JitWidth::b16, reg, 0xf);
        this->x86.lock();
        this->x86.btr(X86Asm::Mem16(R32(address->hardwareReg()), R32(offset->hardwareReg())), R16(reg->hardwareReg()));
        updateFlagsIfNecessary();
        incrementEip(op->len);
    });
}

void JitX86CodeGen::dynamic_btce32_lock(DecodedOp* op) {
    JitCodeGen::write(JitWidth::b32, calculateEaa(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        // imm is the mask, not the bitshift
        U8 imm = std::countr_zero(op->imm);
        this->x86.lock();
        this->x86.btc(X86Asm::Mem32(R32(address->hardwareReg()), R32(offset->hardwareReg())), (U8)imm);
        updateFlagsIfNecessary();
        incrementEip(op->len);
    });
}
void JitX86CodeGen::dynamic_btce16_lock(DecodedOp* op) {
    JitCodeGen::write(JitWidth::b16, calculateEaa(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        // imm is the mask, not the bitshift
        U8 imm = std::countr_zero(op->imm);
        this->x86.lock();
        this->x86.btc(X86Asm::Mem16(R32(address->hardwareReg()), R32(offset->hardwareReg())), (U8)imm);
        updateFlagsIfNecessary();
        incrementEip(op->len);
    });
}
void JitX86CodeGen::dynamic_btce32r32_lock(DecodedOp* op) {
    JitCodeGen::write(JitWidth::b32, calculateEffectiveEaa32(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        RegPtr reg = getTmpReg(op->reg);
        andValue(JitWidth::b32, reg, 0x1f);
        this->x86.lock();
        this->x86.btc(X86Asm::Mem32(R32(address->hardwareReg()), R32(offset->hardwareReg())), R32(reg->hardwareReg()));
        updateFlagsIfNecessary();
        incrementEip(op->len);
    });
}
void JitX86CodeGen::dynamic_btce16r16_lock(DecodedOp* op) {
    JitCodeGen::write(JitWidth::b16, calculateEffectiveEaa16(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        RegPtr reg = getTmpReg(op->reg);
        andValue(JitWidth::b16, reg, 0xf);
        this->x86.lock();
        this->x86.btc(X86Asm::Mem16(R32(address->hardwareReg()), R32(offset->hardwareReg())), R16(reg->hardwareReg()));
        updateFlagsIfNecessary();
        incrementEip(op->len);
    });
}

U8* JitX86CodeGen::createStartJITCode() {
#ifdef BOXEDWINE_64
    // Integer arguments are passed in registers RCX, RDX, R8, and R9
    // The x64 ABI considers registers RBX, RBP, RDI, RSI, RSP, R12, R13, R14, R15, and XMM6 - XMM15 nonvolatile
    x86.push(x86.rbx);
    x86.push(x86.rbp);    
    x86.push(x86.rsi);
    x86.push(x86.rdi);
    x86.push(x86.r12);
    x86.push(x86.r13);
    x86.push(x86.r14);
    x86.push(x86.r15);

    //x86.mov(HOST_MMU, (U64)getMemData(KThread::currentThread()->memory)->mmu);
    x86.mov(HOST_RAM, (U64)ramPages);

    // on win32 ecx contains cpu
    x86.mov(HOST_CPU, x86.rcx);

    // before rdx is clobbered
    x86.mov(x86.r8, X86Asm::Mem64(x86.rdx, offsetof(DecodedOp, pfnJitCode)));

    loadCache();

    // jmp ((DecodedOp*)rdx)->pfn    
    x86.jmp(x86.r8);

#else
    x86.push(x86.ebx);
    x86.push(x86.edi);
    x86.push(x86.esi);
    x86.push(x86.ebp);
    // on win32 ecx contains cpu
    x86.mov(HOST_CPU, x86.ecx);

    loadCache();

    // :TODO: what about other x86 platforms that use a different calling convention
    // 
    // jmp ((DecodedOp*)edx)->pfn
    x86.mov(x86.eax, X86Asm::Mem32(x86.edx, offsetof(DecodedOp, pfnJitCode)));
    x86.jmp(x86.eax);
#endif
    return createDynamicExecutableMemory();
}

void JitX86CodeGen::preCommitJIT() {
    for (DynamicJump& jmp : x86.jumps) {
        U32 bufferIndex = 0;

        if (!eipToBufferPos.get(jmp.eip, bufferIndex)) {
            return;
        }
        *(U32*)&x86.buffer.data()[jmp.bufferPos] = bufferIndex - jmp.bufferPos - 4;
    }
}

void JitX86CodeGen::patch(U8* begin) {
#ifndef BOXEDWINE_64
    for (U32 i = 0; i < x86.patch.size(); i++) {
        U32 pos = x86.patch[i];
        U32* value = (U32*)(&begin[pos]);
        *value = *value - (U32)(begin + pos + 4);
    }
#endif
}

void JitX86CodeGen::loadCache() {
    for (int i = 0; i < 8; i++) {
        if (regCache[i] != INVALID_REG) {
            x86.mov(R32(regCache[i]), X86Asm::Mem32(HOST_CPU, offsetof(CPU, reg[i].u32)));
        }
    }
}

void JitX86CodeGen::writeCache() {
    for (int i = 0; i < 8; i++) {
        if (regCache[i] != INVALID_REG) {
            x86.mov(X86Asm::Mem32(HOST_CPU, offsetof(CPU, reg[i].u32)), R32(regCache[i]));
        }
    }    
}

U8* JitX86CodeGen::createSyncToHost() {
    writeCache();
    x86.ret();
    return createDynamicExecutableMemory();
}

U8* JitX86CodeGen::createSyncFromHost() {
    loadCache();
    x86.ret();
    return createDynamicExecutableMemory();
}

JitCodeGen* startNewJIT(CPU* cpu) {
    return new JitX86CodeGen(cpu);
}

void startNewJIT(CPU* cpu, U32 address, DecodedOp* op) {
    JitX86CodeGen data(cpu);
    data.doJIT(address, op);
}

#endif