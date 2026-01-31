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
#if defined(BOXEDWINE_JIT_ARMV8)

#include "jitArmV8CodeGen.h"
#include "../jit/jitSSE.h"
#include <array>

#undef u8
#undef h8
#undef h16

#include <asmjit/core.h>
#include <asmjit/a64.h>
#include <asmjit/arm/armutils.h>

#define NUMBER_OF_REGS 31
#define NUMBER_OF_VREGS 32
#define NUMBER_OF_TMPS 9
#define NUMBER_OF_VREG_TMPS 6

static bool isVolitile[] = { true,  true,  true,  true,  true,  true,  true,  true,
                             true,  true,  true,  true,  true,  true,  true,  true,
                             true,  true,  true,  false, false, false, false, false,
                             false, false, false, false, false, false, false, false };

static bool isTmp[] = { false, false, false, false, false, false, false, false,
                        false, false, false, false, true, true, true, false,
                        false,  false,  false,  false,  false,  true, true, true,
                        true, true, true, false, false, false, false, false };

static U8 regCache[] = { 0, 1, 2, 3, 4, 5, 6, 7 };
static U8 tmps[] = { 21, 22, 23, 24, 25, 26, 12, 13, 14 };
static U8 vtmps[] = { 16, 17, 18, 19, 20, 21 };

#define INVALID_REG 0xff

typedef asmjit::a64::Mem Mem;
typedef asmjit::a64::Shift Shift;
typedef asmjit::a64::ShiftOp ShiftOp;
typedef asmjit::Label Label;

asmjit::a64::Gp R64(U8 reg) {
    return asmjit::a64::gp64(reg);
}

asmjit::a64::Gp R32(U8 reg) {
    return asmjit::a64::gp32(reg);
}

#define xEAX 0
#define xECX 1
#define xEDX 2
#define xEBX 3
#define xESP 4
#define xEBP 5
#define xESI 6
#define xEDI 7
#define xFLAGS asmjit::a64::w8
#define regFlags 8
#define xFlagsType asmjit::a64::w11
#define regFlagsType 11

// x9 to x15 caller saved
#define xBranch asmjit::a64::x9 
#define xMemTmp asmjit::a64::x10

#define xTmp7 12
#define xTmp8 13
#define xTmp9 14
#define xBranchEip 17

// don't use x18

// x19 to x28 callee saved
// These should be one that won't need to be reloaded after a function call
#define regCPU 19
#define xCPU asmjit::a64::x19
#define xMMU asmjit::a64::x20
#define regMMU 20
#define xTmp1 21
#define xTmp2 22
#define xTmp3 23
#define xTmp4 24
#define xTmp5 25
#define xTmp6 26
#define xWriteCacheToCPU asmjit::a64::x27
#define xLoadCacheFromCPU asmjit::a64::x28
    
#define ZERO_EXTEND 1
#define SIGN_EXTEND 2

class JitArmV8CodeGen : public JitMMX, asmjit::ErrorHandler {
public:  
    void handle_error(asmjit::Error err, const char* message, asmjit::BaseEmitter* origin) override {
        kpanic(message);
    }

    JitArmV8CodeGen(CPU* cpu) : JitMMX(cpu) {
        code.init(rt.environment());
        code.attach(&compiler);
        code.set_error_handler(this);
        fpuRoundingMode = cpu->fpu.round;
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
    RegPtr readEip() override;
    void writeEip(RegPtr eip) override;
    void writeEip(U32 eip) override;
    void pushReg(RegPtr reg) override;
    void popReg(RegPtr reg) override;
    bool isTmpRegAvailable() override;    
    void forceSyncBackIfNotCached(RegPtr reg) override;
    
    U8 findTmpReg(bool allowInvalidReturn = false);
    void emulateSingleOp() override;

    void regReg(JitWidth regWidth, RegPtr reg, RegPtr rm, std::function<void(asmjit::a64::Gp dst, asmjit::a64::Gp reg, asmjit::a64::Gp rm)> fn, U32 extend = 0);
    void reg1(JitWidth regWidth, RegPtr reg, std::function<void(asmjit::a64::Gp dst, asmjit::a64::Gp src)> fn);
    void regValue(JitWidth regWidth, RegPtr reg, U32 value, std::function<void(asmjit::a64::Gp dst, asmjit::a64::Gp reg, U32 value)> fn, U32 extend = 0);

    void addReg(JitWidth regWidth, RegPtr reg, RegPtr rm) override;
    void addValue(JitWidth regWidth, RegPtr reg, U32 imm) override;
    void orReg(JitWidth regWidth, RegPtr reg, RegPtr rm) override;
    void orValue(JitWidth regWidth, RegPtr reg, U32 imm) override;
    void subReg(JitWidth regWidth, RegPtr reg, RegPtr rm) override;
    void subValue(JitWidth regWidth, RegPtr reg, U32 imm) override;
    void andReg(JitWidth regWidth, RegPtr reg, RegPtr rm) override;
    void andValue(JitWidth regWidth, RegPtr reg, U32 immm) override;
    void andValue64(RegPtr reg, U64 immm) override;
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
    void mulReg(JitWidth regWidth, RegPtr reg) override;
    void imulReg(JitWidth regWidth, RegPtr reg) override;
    void imulRRI(JitWidth regWidth, RegPtr dst, RegPtr src, U32 src2, RegPtr overflow = nullptr) override;
    void imulRR(JitWidth regWidth, RegPtr dst, RegPtr src, RegPtr overflow = nullptr) override;
    void divRegRegWithRemainder(JitWidth regWidth, RegPtr dest, RegPtr destHighAndRemainder, RegPtr src) override;
    void idivRegRegWithRemainder(JitWidth regWidth, RegPtr dest, RegPtr destHighAndRemainder, RegPtr src) override;
    void absReg(JitWidth regWidth, RegPtr reg) override;
    void clzReg(JitWidth regWidth, RegPtr result, RegPtr reg) override;

    void byteSwapReg32(RegPtr reg) override;
    RegPtr compareReg(JitWidth regWidth, RegPtr reg1, RegPtr reg2, JitEvaluate condition, RegPtr resultReg = nullptr) override;    
    RegPtr compareValue(JitWidth regWidth, RegPtr reg, U32 value, JitEvaluate condition, RegPtr resultReg = nullptr) override;
    RegPtr compare(JitEvaluate condition, RegPtr result);
    RegPtr testZeroReg(JitWidth regWidth, RegPtr reg, RegPtr result = nullptr) override;

    void readRamPage(RegPtr dest, RegPtr index) override;
    void readMMU(RegPtr dest, RegPtr index) override;
    void readMMU(RegPtr dest, U32 index) override;
    void read(JitWidth width, RegPtr dest, RegPtr reg, U32 disp) override;
    void read(JitWidth width, RegPtr dest, RegPtr reg, RegPtr sib, U8 lsl, U32 disp) override;
    void write(JitWidth width, RegPtr reg, U32 disp, RegPtr src) override;
    void write(JitWidth width, RegPtr reg, RegPtr sib, U8 lsl, U32 disp, RegPtr src) override;
    void write(JitWidth width, RegPtr reg, RegPtr sib, U8 lsl, U32 disp, U32 value) override;

    RegPtr readCPU(JitWidth width, U32 offset, RegPtr resultReg = nullptr) override;
    RegPtr readCPU(JitWidth width, RegPtr sib, U8 lsl, U32 offset, RegPtr resultReg = nullptr) override;
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
    void IfNotTestBit(JitWidth regWidth, RegPtr reg, U32 bitPos) override;
    void IfTestBit(JitWidth regWidth, RegPtr reg, U32 bitPos) override;
    void IfEqual(JitWidth regWidth, RegPtr reg, DYN_PTR_SIZE value) override;
    void IfEqual(JitWidth regWidth, RegPtr reg1, RegPtr reg2) override;
    void IfNotEqual(JitWidth regWidth, RegPtr reg, DYN_PTR_SIZE value) override;
    void IfNotEqual(JitWidth regWidth, RegPtr reg, RegPtr reg2) override;
    void IfLessThan2(JitWidth regWidth, RegPtr reg, U32 value) override;
    void IfLessThan2(JitWidth regWidth, RegPtr reg1, RegPtr reg2) override;
    void IfGreaterThanOrEqual(JitWidth regWidth, RegPtr reg1, RegPtr reg2) override;
    void IfGreaterThanOrEqual(JitWidth regWidth, RegPtr reg1, U32) override;
    void IfNot(JitWidth regWidth, RegPtr reg) override;
    void IfNotCPU(JitWidth regWidth, RegPtr sib, U8 lsl, U32 offset) override;    
    void JumpIfCondition(JitConditional condition, U32 address) override;
    void clearMMUPermissionIfSpansPage(JitWidth width, RegPtr offset, RegPtr reg) override;

    U32 MarkJumpLocation() override;
    void Goto(U32 location) override;
    void jmp(RegPtr reg) override;
    void updateFlagsIfNecessary();    
    RegPtr getReadOnlyFlags() override;
    void storeLazyFlagType(LazyFlagType flags);
    RegPtr getFlagsInTmp(RegPtr reg = nullptr) override;
    void setFlags(RegPtr flags, U32 mask) override;
    RegPtr getLazyFlagType() override;
    RegPtr getLazyFlagTypeInTmp() override;
    void writeFlags(RegPtr flags) override;

    void callHostFunction(void* address, const std::vector<DynParam>& params, bool restoreCache = true) override;
    void callHostFunctionWithResult(RegPtr result, void* address, const std::vector<DynParam>& params) override;
    void nakedCall(RegPtr reg) override;
    void nakedReturn() override;
    void pushParam(const DynParam& param, U32 index);

    std::array<bool, NUMBER_OF_REGS> regUsed{ 0 };
    std::array<bool, NUMBER_OF_VREGS> vregUsed{ 0 };

    void JumpInBlock(U32 address) override;
    void StartElse() override;
    void EndIf() override;
    void blockExit(bool syncCache = true) override;

    // FPU
    void dynamic_FNINIT(DecodedOp* op) override;

    U32 fpuRoundingMode = ROUND_Nearest;
    FPURegPtr getFPUTmp() override;
    bool shouldContinueCompilingAfterOp(DecodedOp* op) override;
    void storeCpuFpuReg(FPURegPtr reg, RegPtr index) override;
    void loadCpuFpuReg(FPURegPtr reg, RegPtr index) override;
    void loadCpuFpuRegConst(FPURegPtr reg, U32 offset) override;
    void loadFpuRegFromInt(FPURegPtr reg, RegPtr rm, RegPtr sib) override;
    void storeFpuReg(FPURegPtr reg, RegPtr rm, RegPtr sib, DynFpuWidth width = DYN_FPU_64_BIT) override;
    RegPtr fpuRegToInt32(FPURegPtr fpuRegSrc, bool truncate) override;
    void loadFpuReg(FPURegPtr reg, RegPtr rm, RegPtr sib, DynFpuWidth width = DYN_FPU_64_BIT) override;
    void fpuRegExtend32To64(FPURegPtr dst, FPURegPtr src) override;
    void fpuReg64To32(FPURegPtr dst, FPURegPtr src) override;
    void regToFpuReg(FPURegPtr dst, RegPtr src) override;
#ifdef BOXEDWINE_64
    void regToFpuReg64(FPURegPtr dst, RegPtr src) override;
#endif
    void updateFPURounding() override;
    void restoreFPURounding() override;
    void roundFPUToInt64(FPURegPtr src) override;
    void storeFPUToInt64(FPURegPtr src, RegPtr address, RegPtr offset, bool truncate) override;

    void fpuAdd(FPURegPtr dst, FPURegPtr src) override;
    void fpuMul(FPURegPtr dst, FPURegPtr src) override;
    void fpuSub(FPURegPtr dst, FPURegPtr src) override;
    void fpuDiv(FPURegPtr dst, FPURegPtr src) override;
    void fpuXor(FPURegPtr dst, FPURegPtr src) override;
    void fpuAnd(FPURegPtr dst, FPURegPtr src) override;
    void fpuSqrt(FPURegPtr dst, FPURegPtr src) override;
    void fcompare(FPURegPtr fpuReg1, FPURegPtr fpuReg2, RegPtr ordTags, const std::function<void()>& pfnEqual, const std::function<void()>& pfnLessThan, const std::function<void()>& pfnGreaterThan, const std::function<void()>& pfnInvalid) override;
   
    // MMX
    void pshuflwNotOverlappingDstSrc(MMXRegPtr dst, MMXRegPtr src, U8 mask);
    MMXRegPtr getTmpMMX() override;
    MMXRegPtr loadMMXFromReg(RegPtr reg) override;
    MMXRegPtr loadMMXConst(U8 index);
    void storeCpuMMXReg(MMXRegPtr reg, U32 index) override;
    void storeMMXToReg(MMXRegPtr mmx, RegPtr reg) override;
    MMXRegPtr loadCpuMMXReg(U8 index) override;
    MMXRegPtr loadMMXFromMem32(U8 index, RegPtr rm, RegPtr sib, U8 lsl, U32 disp) override;
    MMXRegPtr loadMMXFromMem64(U8 index, RegPtr rm, RegPtr sib, U8 lsl, U32 disp) override;
    void storeMMXToMem32(MMXRegPtr reg, RegPtr rm, RegPtr sib, U8 lsl, U32 disp) override;
    void storeMMXToMem64(MMXRegPtr reg, RegPtr rm, RegPtr sib, U8 lsl, U32 disp) override;
    void xorMmxMmx(MMXRegPtr dst, MMXRegPtr src) override;
    void orMmxMmx(MMXRegPtr dst, MMXRegPtr src) override;
    void andMmxMmx(MMXRegPtr dst, MMXRegPtr src) override;
    void andnMmxMmx(MMXRegPtr dst, MMXRegPtr src) override;
    void psllwMmxMmx(MMXRegPtr dst, MMXRegPtr src) override;
    void psrlwMmxMmx(MMXRegPtr dst, MMXRegPtr src) override;
    void psrawMmxMmx(MMXRegPtr dst, MMXRegPtr src) override;
    void psllwMmx(MMXRegPtr dst, U32 imm) override;
    void psrlwMmx(MMXRegPtr dst, U32 imm) override;
    void psrawMmx(MMXRegPtr dst, U32 imm) override;
    void pslldMmxMmx(MMXRegPtr dst, MMXRegPtr src) override;
    void psrldMmxMmx(MMXRegPtr dst, MMXRegPtr src) override;
    void psradMmxMmx(MMXRegPtr dst, MMXRegPtr src) override;
    void pslldMmx(MMXRegPtr dst, U32 imm) override;
    void psrldMmx(MMXRegPtr dst, U32 imm) override;
    void psradMmx(MMXRegPtr dst, U32 imm) override;
    void psllqMmxMmx(MMXRegPtr dst, MMXRegPtr src) override;
    void psrlqMmxMmx(MMXRegPtr dst, MMXRegPtr src) override;
    void psllqMmx(MMXRegPtr dst, U32 imm) override;
    void psrlqMmx(MMXRegPtr dst, U32 imm) override;
    void paddbMmxMmx(MMXRegPtr dst, MMXRegPtr src) override;
    void paddwMmxMmx(MMXRegPtr dst, MMXRegPtr src) override;
    void padddMmxMmx(MMXRegPtr dst, MMXRegPtr src) override;
    void paddsbMmxMmx(MMXRegPtr dst, MMXRegPtr src) override;
    void paddswMmxMmx(MMXRegPtr dst, MMXRegPtr src) override;
    void paddusbMmxMmx(MMXRegPtr dst, MMXRegPtr src) override;
    void padduswMmxMmx(MMXRegPtr dst, MMXRegPtr src) override;
    void psubbMmxMmx(MMXRegPtr dst, MMXRegPtr src) override;
    void psubwMmxMmx(MMXRegPtr dst, MMXRegPtr src) override;
    void psubdMmxMmx(MMXRegPtr dst, MMXRegPtr src) override;
    void psubsbMmxMmx(MMXRegPtr dst, MMXRegPtr src) override;
    void psubswMmxMmx(MMXRegPtr dst, MMXRegPtr src) override;
    void psubusbMmxMmx(MMXRegPtr dst, MMXRegPtr src) override;
    void psubuswMmxMmx(MMXRegPtr dst, MMXRegPtr src) override;
    void pmulhwMmxMmx(MMXRegPtr dst, MMXRegPtr src) override;
    void pmullwMmxMmx(MMXRegPtr dst, MMXRegPtr src) override;
    void pmaddwdMmxMmx(MMXRegPtr dst, MMXRegPtr src) override;
    void pcmpeqbMmxMmx(MMXRegPtr dst, MMXRegPtr src) override;
    void pcmpeqwMmxMmx(MMXRegPtr dst, MMXRegPtr src) override;
    void pcmpeqdMmxMmx(MMXRegPtr dst, MMXRegPtr src) override;
    void pcmpgtbMmxMmx(MMXRegPtr dst, MMXRegPtr src) override;
    void pcmpgtwMmxMmx(MMXRegPtr dst, MMXRegPtr src) override;
    void pcmpgtdMmxMmx(MMXRegPtr dst, MMXRegPtr src) override;
    void packsswbMmxMmx(MMXRegPtr dst, MMXRegPtr src) override;
    void packssdwMmxMmx(MMXRegPtr dst, MMXRegPtr src) override;
    void packuswbMmxMmx(MMXRegPtr dst, MMXRegPtr src) override;
    void punpckhbwMmxMmx(MMXRegPtr dst, MMXRegPtr src) override;
    void punpckhwdMmxMmx(MMXRegPtr dst, MMXRegPtr src) override;
    void punpckhdqMmxMmx(MMXRegPtr dst, MMXRegPtr src) override;
    void punpcklbwMmxMmx(MMXRegPtr dst, MMXRegPtr src) override;
    void punpcklwdMmxMmx(MMXRegPtr dst, MMXRegPtr src) override;
    void punpckldqMmxMmx(MMXRegPtr dst, MMXRegPtr src) override;

    void pavgbMmxMmx(MMXRegPtr dst, MMXRegPtr src) override;
    void pavgwMmxMmx(MMXRegPtr dst, MMXRegPtr src) override;
    void psadbwMmxMmx(MMXRegPtr dst, MMXRegPtr src) override;
    void pextrwRegMmx(RegPtr dst, MMXRegPtr src, U8 srcIndex) override;
    void pinsrwMmxReg(MMXRegPtr dest, RegPtr src, U8 dstIndex) override;
    void pmaxswMmxMmx(MMXRegPtr dst, MMXRegPtr src) override;
    void pmaxubMmxMmx(MMXRegPtr dst, MMXRegPtr src) override;
    void pminswMmxMmx(MMXRegPtr dst, MMXRegPtr src) override;
    void pminubMmxMmx(MMXRegPtr dst, MMXRegPtr src) override;
    void pmovmskbMmxMmx(RegPtr dst, MMXRegPtr src) override;
    void pmulhuwMmxMmx(MMXRegPtr dst, MMXRegPtr src) override;
    void pshufwMmxMmx(MMXRegPtr dst, MMXRegPtr src, U8 mask) override;
    void maskmovq(MMXRegPtr src, MMXRegPtr mask, RegPtr destAddress) override;

    void paddqMmxMmx(MMXRegPtr dst, MMXRegPtr src) override;
    void psubqMmxMmx(MMXRegPtr dst, MMXRegPtr src) override;
    void pmuludqMmxMmx(MMXRegPtr dst, MMXRegPtr src) override;

    /*
    // SSE    
    SSERegPtr getTmpSSE() override;
    void IfSseLessThan(SSERegPtr src1, SSERegPtr src2) override;
    bool isSseRegCached(U8 reg) override;
    void storeCpuXMMReg(SSERegPtr reg, U32 index) override;
    SSERegPtr loadCpuXMMReg(U8 index) override;
    SSERegPtr loadXMMFromMem128(U8 reg, RegPtr rm, RegPtr sib, U8 lsl, U32 disp) override;
    SSERegPtr loadXMMFromMem64(U8 reg, RegPtr rm, RegPtr sib, U8 lsl, U32 disp) override;
    SSERegPtr loadLowXMMFromMem64(U8 reg, RegPtr rm, RegPtr sib, U8 lsl, U32 disp) override;
    SSERegPtr loadHighXMMFromMem64(U8 reg, RegPtr rm, RegPtr sib, U8 lsl, U32 disp) override;
    SSERegPtr loadXMMFromMem32(U8 reg, RegPtr rm, RegPtr sib, U8 lsl, U32 disp) override;
    void storeXMMToMem128(SSERegPtr reg, RegPtr rm, RegPtr sib, U8 lsl, U32 disp) override;
    void storeXMMToMem64(SSERegPtr reg, RegPtr rm, RegPtr sib, U8 lsl, U32 disp) override;
    void storeXMMToMem32(SSERegPtr reg, RegPtr rm, RegPtr sib, U8 lsl, U32 disp) override;
    void storeHighXMMToMem64(SSERegPtr reg, RegPtr rm, RegPtr sib, U8 lsl, U32 disp) override;

    void addpsXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void addssXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void subpsXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void subssXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void mulpsXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void mulssXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void divpsXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void divssXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void rcppsXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void rcpssXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void sqrtpsXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void sqrtssXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void rsqrtpsXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void rsqrtssXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void maxpsXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void maxssXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void minpsXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void minssXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void andnpsXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void andpsXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void orpsXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void xorpsXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void cvtpi2psXmmMmx(SSERegPtr dst, MMXRegPtr src) override;
    void cvtps2piMmxXmm(MMXRegPtr dst, SSERegPtr src) override;
    void cvtsi2ssXmmR32(SSERegPtr dst, RegPtr src) override;
    void cvtss2siR32Xmm(RegPtr dst, SSERegPtr src) override;
    void cvttps2piMmxXmm(MMXRegPtr dst, SSERegPtr src) override;
    void cvttss2siR32Xmm(RegPtr dst, SSERegPtr src) override;
    void movhlpsXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void movlhpsXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void movmskpsR32Xmm(RegPtr dst, SSERegPtr src) override;
    void movssXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void shufpsXmmXmm(SSERegPtr dst, SSERegPtr src, U32 imm) override;
    void unpckhpsXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void unpcklpsXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void cmppsXmmXmm(SSERegPtr dst, SSERegPtr src, U32 imm) override;
    void cmpssXmmXmm(SSERegPtr dst, SSERegPtr src, U32 imm) override;
    void comissXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void ucomissXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void sfence() override;
    void stmxcsr(RegPtr address) override;
    void ldmxcsr(RegPtr address) override;

    // SSE2
#ifdef BOXEDWINE_64
    void cvtsi2sdXmmR64(SSERegPtr dst, RegPtr src) override;
#endif
    void addpdXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void addsdXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void subpdXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void subsdXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void mulpdXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void mulsdXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void divpdXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void divsdXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void maxpdXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void maxsdXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void minpdXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void minsdXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void paddbXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void paddwXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void padddXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void paddqXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void paddsbXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void paddswXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void paddusbXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void padduswXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void psubbXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void psubwXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void psubdXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void psubqXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void psubsbXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void psubswXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void psubusbXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void psubuswXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void pmaddwdXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void pmulhwXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void pmullwXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void pmuludqXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void sqrtpdXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void sqrtsdXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void andnpdXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void andpdXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void pandXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void pandnXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void porXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void pslldqXmm(SSERegPtr dst, U32 imm) override;
    void psllqXmm(SSERegPtr dst, U32 imm) override;
    void pslldXmm(SSERegPtr dst, U32 imm) override;
    void psllwXmm(SSERegPtr dst, U32 imm) override;
    void psradXmm(SSERegPtr dst, U32 imm) override;
    void psrawXmm(SSERegPtr dst, U32 imm) override;
    void psrldqXmm(SSERegPtr dst, U32 imm) override;
    void psrlqXmm(SSERegPtr dst, U32 imm) override;
    void psrldXmm(SSERegPtr dst, U32 imm) override;
    void psrlwXmm(SSERegPtr dst, U32 imm) override;
    void psllqXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void pslldXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void psllwXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void psradXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void psrawXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void psrlqXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void psrldXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void psrlwXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void pxorXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void orpdXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void xorpdXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void cmppdXmmXmm(SSERegPtr dst, SSERegPtr src, U32 imm) override;
    void cmpsdXmmXmm(SSERegPtr dst, SSERegPtr src, U32 imm) override;
    void comisdXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void ucomisdXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void pcmpgtbXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void pcmpgtwXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void pcmpgtdXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void pcmpeqbXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void pcmpeqwXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void pcmpeqdXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void cvtdq2pdXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void cvtdq2psXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void cvtpd2piMmxXmm(MMXRegPtr dst, SSERegPtr src) override;
    void cvtpi2pdXmmMmx(SSERegPtr dst, MMXRegPtr src) override;
    void cvtpd2dqXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void cvtpd2psXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void cvttpd2piMmxXmm(MMXRegPtr dst, SSERegPtr src) override;
    void cvtps2dqXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void cvtps2pdXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void cvtsd2siR32Xmm(RegPtr dst, SSERegPtr src) override;
    void cvtsd2ssXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void cvtsi2sdXmmR32(SSERegPtr dst, RegPtr src) override;
    void cvtss2sdXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void cvttpd2dqXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void cvttps2dqXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void cvttsd2siR32Xmm(RegPtr dst, SSERegPtr src) override;
    void movsdXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void movupdXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void movmskpd(RegPtr dst, SSERegPtr src) override;
    void movd(RegPtr dst, SSERegPtr src) override;
    void movd(SSERegPtr dst, RegPtr src) override;
    void movdq2q(MMXRegPtr dst, SSERegPtr src) override;
    void movq2dq(SSERegPtr dst, MMXRegPtr src) override;
    void movq(SSERegPtr dst, SSERegPtr src) override;

    void maskmovdqu(SSERegPtr dst, SSERegPtr src, RegPtr address) override;
    void pshufdXmmXmm(SSERegPtr dst, SSERegPtr src, U32 imm) override;
    void pshufhwXmmXmm(SSERegPtr dst, SSERegPtr src, U32 imm) override;
    void pshuflwXmmXmm(SSERegPtr dst, SSERegPtr src, U32 imm) override;
    void shufpdXmmXmm(SSERegPtr dst, SSERegPtr src, U32 imm) override;
    void unpckhpdXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void unpcklpdXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void punpckhbwXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void punpckhwdXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void punpckhdqXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void punpckhqdqXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void punpcklbwXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void punpcklwdXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void punpckldqXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void punpcklqdqXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void packssdwXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void packsswbXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void packuswbXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void pavgbXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void pavgwXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void psadbwXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void pmaxswXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void pmaxubXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void pminswXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void pminubXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void pmulhuwXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void lfence() override;
    void mfence() override;
    void clflush(RegPtr rm, RegPtr sib, U8 lsl, U32 disp) override;
    void pause() override;
    void pextrwR32Xmm(RegPtr dst, SSERegPtr src, U32 imm) override;
    void pinsrwXmmR32(SSERegPtr dst, RegPtr src, U32 imm) override;
    void pmovmskbR32Xmm(RegPtr dst, SSERegPtr src) override;


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
    void dynamic_ore32r32_lock(DecodedOp* op) override;
    void dynamic_ore16r16_lock(DecodedOp* op) override;
    void dynamic_ore8r8_lock(DecodedOp* op) override;
    void dynamic_or32_mem_lock(DecodedOp* op) override;
    void dynamic_or16_mem_lock(DecodedOp* op) override;
    void dynamic_or8_mem_lock(DecodedOp* op) override;
    void dynamic_ande32r32_lock(DecodedOp* op) override;
    void dynamic_ande16r16_lock(DecodedOp* op) override;
    void dynamic_ande8r8_lock(DecodedOp* op) override;
    void dynamic_and32_mem_lock(DecodedOp* op) override;
    void dynamic_and16_mem_lock(DecodedOp* op) override;
    void dynamic_and8_mem_lock(DecodedOp* op) override;
    void dynamic_xore32r32_lock(DecodedOp* op) override;
    void dynamic_xore16r16_lock(DecodedOp* op) override;
    void dynamic_xore8r8_lock(DecodedOp* op) override;
    void dynamic_xor32_mem_lock(DecodedOp* op) override;
    void dynamic_xor16_mem_lock(DecodedOp* op) override;
    void dynamic_xor8_mem_lock(DecodedOp* op) override;
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
    */
protected:
    friend void startNewJIT(CPU* cpu, U32 address, DecodedOp* op);

    U32 markBufferLocation() override;
    U32 getBufferLocation(U32 id) override;
    U32 getBufferSize() override;
    U32 getIfJumpSize() override;    
    void copyBuffer(U8* dst, U32 size) override;

    void preCommitJIT() override;
    void patch(U8* begin) override;
    U8* createStartJITCode() override;

    void loadCacheFromCPU();
    void writeCacheToCPU();
    void setParams(const std::vector<DynParam>& params);
    U8* createSyncToHost() override;
    U8* createSyncFromHost() override;
    U8 findTmpXMM();
    SSERegPtr getXMM(U8 index, bool load);
    void cmp(JitWidth width, RegPtr reg, RegPtr reg2);
    void cmp(JitWidth width, RegPtr reg, DYN_PTR_SIZE value);
    void IfEqual();
    void IfGreater();
    void IfLessThanOrEqual();
    void If_v();

    U8 getMMXReg(MMXRegPtr reg);
    RegPtr getReg8InLowByte(RegPtr reg);
    bool isRegHigh(RegPtr reg);
    Mem createMem(RegPtr reg, RegPtr sib, U8 lsl, U32 disp);
    Mem createMem(RegPtr reg, U32 disp);
    Mem createMem(U8 reg, U8 sib, U8 lsl, U32 disp);
    Mem createMem(U8 reg, U32 disp);
    RegPtr loadConst(U64 value);    
    void modValue32(RegPtr dst, RegPtr src, RegPtr value);

    asmjit::JitRuntime rt;
    asmjit::CodeHolder code;
    asmjit::a64::Assembler compiler;

    std::array<bool, NUMBER_OF_REGS> rUsed{ 0 };
    std::array<bool, NUMBER_OF_VREGS> vUsed{ 0 };
    std::vector<Label> labels;

    std::vector<Label> ifLabels;
    BHashTable<U32, Label> opLabels;
};

void JitArmV8CodeGen::preOp(DecodedOp* op) {
    rUsed.fill(false);
    vUsed.fill(false);
    currentOp = op;
    Label label;
    if (!opLabels.get(currentEip, label)) {
        label = compiler.new_label();
        opLabels.set(currentEip, label);
    }
    compiler.bind(label);
}

U8 JitArmV8CodeGen::findTmpXMM() {
    for (int i = 0; i < NUMBER_OF_VREG_TMPS; i++) {
        U8 tmp = vtmps[i];
        if (!vUsed[tmp]) {
            vUsed[tmp] = true;
            return tmp;
        }
    }
    kpanic("JitArmV8CodeGen::findTmpXMM");
    return INVALID_REG;
}

/*
bool JitArmV8CodeGen::isSseRegCached(U8 reg) {
    return true;
}
*/

/*
SSERegPtr JitArmV8CodeGen::getTmpSSE() {
    return std::shared_ptr<SSERegInternal>(new SSERegInternal(findTmpXMM(), 0xff), [this](SSERegInternal* p) {
        vUsed[p->hardwareReg()] = false;
        delete p;
    });
}
*/

U8 JitArmV8CodeGen::getMMXReg(MMXRegPtr reg) {
    return reg->hardwareReg();
}

MMXRegPtr JitArmV8CodeGen::getTmpMMX() {
    return std::shared_ptr<MMXRegInternal>(new MMXRegInternal(findTmpXMM(), 0xff), [this](MMXRegInternal* p) {
        vUsed[p->hardwareReg()] = false;
        delete p;
    });
}

FPURegPtr JitArmV8CodeGen::getFPUTmp() {
    return std::shared_ptr<FPURegInternal>(new FPURegInternal(findTmpXMM()), [this](FPURegInternal* p) {
        vUsed[p->hardwareReg()] = false;
        delete p;
    });
}

void JitArmV8CodeGen::emulateSingleOp() {
    writeCurrentEip(0);

    compiler.mov(xBranch, (DYN_PTR_SIZE)cpu->thread->process->emulateSingleOp);
    compiler.br(xBranch); // we won't return to here
}

U8 JitArmV8CodeGen::findTmpReg(bool allowInvalidReturn) {
    for (int i = 0; i < NUMBER_OF_TMPS; i++) {
        U8 tmp = tmps[i];
        if (!regUsed[tmp]) {
            regUsed[tmp] = true;
            return tmp;
        }
    }
    if (!allowInvalidReturn) {
        kpanic("JitArmV8CodeGen::getTmpReg ran out of tmp regs");
    }
    return 0xff;
}

RegPtr JitArmV8CodeGen::getTmpReg() {
    return std::shared_ptr<JitReg>(new JitReg(findTmpReg(), 0xff), [this](JitReg* p) {
        if (p->isLoaded()) {
            regUsed[p->hardwareReg()] = false;
        }
        delete p;
    });
}

RegPtr JitArmV8CodeGen::getTmpRegWithHint(S8 hint) {
    return getTmpReg();
}

RegPtr JitArmV8CodeGen::getTmpReg8() {
    return getTmpReg();
}

RegPtr JitArmV8CodeGen::getTmpReg(U8 reg, bool delayed, S8 hint) {
    if (delayed) {
        auto getTmp = [reg, hint, this]() {
            U8 hardwareReg = findTmpReg();
            if (regCache[reg] != INVALID_REG) {
                compiler.mov(R64(hardwareReg), R64(regCache[reg]));
            } else {
                compiler.ldr(R32(hardwareReg), Mem(xCPU, CPU::offsetofReg32(reg)));
            }
            return hardwareReg;
        };

        return std::shared_ptr<JitReg>(new JitReg(0xff, 0xff, getTmp), [this](JitReg* p) {
            if (p->isLoaded()) {
                regUsed[p->hardwareReg()] = false;
            }
            delete p;
        });
    } else {
        RegPtr result = std::shared_ptr<JitReg>(new JitReg(findTmpReg(), 0xff), [this](JitReg* p) {
            if (p->isLoaded()) {
                regUsed[p->hardwareReg()] = false;
            }
            delete p;
        });
        if (regCache[reg] != INVALID_REG) {
            compiler.mov(R64(result->hardwareReg()), R64(regCache[reg]));
        } else {
            compiler.ldr(R32(result->hardwareReg()), Mem(xCPU, CPU::offsetofReg32(reg)));
        }
        return result;
    }
}

RegPtr JitArmV8CodeGen::getTmpRegForCallResult() {
    return std::shared_ptr<JitReg>(new JitReg(findTmpReg(), 0xff), [this](JitReg* p) {
        if (p->isLoaded()) {
            regUsed[p->hardwareReg()] = false;
        }
        delete p;
    });
}

RegPtr JitArmV8CodeGen::getTmpSegAddress(U8 reg) {
    return getReadOnlySegAddress(reg);
}

RegPtr JitArmV8CodeGen::getReadOnlySegAddress(U8 reg) {
    RegPtr result = getTmpReg();
    compiler.ldr(R32(result->hardwareReg()), Mem(xCPU, CPU::offsetofSegAddress(reg)));
    return result;
}

RegPtr JitArmV8CodeGen::getReadOnlySegValue(U8 reg) {
    RegPtr result = getTmpReg();
    compiler.ldr(R32(result->hardwareReg()), Mem(xCPU, CPU::offsetofSegValue(reg)));
    return result;
}

RegPtr JitArmV8CodeGen::getTmpReg8(U8 reg, bool delayed, S8 hint) {
    U8 hardwareReg = reg;
    if (hardwareReg >= 4) {
        hardwareReg -= 4;
        RegPtr result = getTmpReg(hardwareReg, delayed, hint);
        result->emulatedReg = reg;
        result->isHigh = true;
        return result;
    }
    return getTmpReg(reg, delayed, hint);
}

RegPtr JitArmV8CodeGen::readEip() {
    RegPtr result = getTmpReg();
    compiler.ldr(R32(result->hardwareReg()), Mem(xCPU, offsetof(CPU, eip.u32)));
    return result;
}

void JitArmV8CodeGen::writeEip(RegPtr reg) {
    compiler.str(R32(reg->hardwareReg()), Mem(xCPU, offsetof(CPU, eip.u32)));
}

void JitArmV8CodeGen::writeEip(U32 eip) {
    RegPtr reg = getTmpReg();
    compiler.mov(R32(reg->hardwareReg()), eip);
    writeEip(reg);
}

void JitArmV8CodeGen::pushReg(RegPtr reg) {
    kpanic("JitArmV8CodeGen::pushReg");
}

void JitArmV8CodeGen::popReg(RegPtr reg) {
    kpanic("JitArmV8CodeGen::popReg");
}

bool JitArmV8CodeGen::isTmpRegAvailable() {
    U8 found = findTmpReg(true);
    if (found == 0xff) {
        return false;
    }
    regUsed[found] = false;
    return true;
}

void JitArmV8CodeGen::forceSyncBackIfNotCached(RegPtr reg) {
    if (reg->emulatedReg != 0xff && regCache[reg->emulatedReg] == INVALID_REG) {
        compiler.str(R32(reg->hardwareReg()), Mem(xCPU, CPU::offsetofReg32(reg->emulatedReg)));
    }
}

RegPtr JitArmV8CodeGen::getReg(U8 reg, S8 hint, bool load) {
    if (regCache[reg] == INVALID_REG) {
        RegPtr result = std::shared_ptr<JitReg>(new JitReg(findTmpReg(), reg), [this](JitReg* p) {
            compiler.str(R32(p->hardwareReg()), Mem(xCPU, CPU::offsetofReg32(p->emulatedReg)));
            regUsed[p->hardwareReg()] = false;
            delete p;
        });
        if (load) {
            compiler.ldr(R32(result->hardwareReg()), Mem(xCPU, CPU::offsetofReg32(reg)));
        }
        return result;
    } else {
        return std::make_shared<JitReg>(regCache[reg], reg);
    }
}

RegPtr JitArmV8CodeGen::getReg8(U8 reg) {
    U8 hardwareReg = reg;
    if (hardwareReg >= 4) {
        hardwareReg -= 4;
        RegPtr result = getReg(hardwareReg);
        result->emulatedReg = reg;
        result->isHigh = true;
        return result;
    }
    return getReg(reg);
}

RegPtr JitArmV8CodeGen::getReadOnlyReg(U8 reg, bool delayed, S8 hint) {
    if (regCache[reg] != INVALID_REG) {
        return std::make_shared<JitReg>(regCache[reg], reg);
    } else {
        return getTmpReg(reg, delayed, hint);
    }
}

RegPtr JitArmV8CodeGen::getReadOnlyReg8(U8 reg, bool delayed, S8 hint) {
    U8 hardwareReg = reg;
    if (hardwareReg >= 4) {
        hardwareReg -= 4;
    }
    if (regCache[hardwareReg] != INVALID_REG) {
        return std::make_shared<JitReg>(regCache[hardwareReg], reg, reg >= 4);
    }
    return getTmpReg8(reg, delayed, hint);
}

RegPtr JitArmV8CodeGen::getReg8InLowByte(RegPtr reg) {
    if (!isRegHigh(reg)) {
        return reg;
    }
    RegPtr tmp = getTmpReg();
    compiler.ubfx(R32(tmp->hardwareReg()), R32(reg->hardwareReg()), 8, 8);
    return tmp;
}

bool JitArmV8CodeGen::isRegHigh(RegPtr reg) {
    return reg->emulatedReg >= 4 && reg->emulatedReg <= 7;
}

void JitArmV8CodeGen::reg1(JitWidth regWidth, RegPtr reg, std::function<void(asmjit::a64::Gp dst, asmjit::a64::Gp src)> fn) {
    if (regWidth == JitWidth::b32) {
        fn(R32(reg->hardwareReg()), R32(reg->hardwareReg()));
    } else if (regWidth == JitWidth::b64) {
        fn(R64(reg->hardwareReg()), R64(reg->hardwareReg()));
    } else if (regWidth == JitWidth::b16) {
        RegPtr tmp = getTmpReg();
        fn(R32(tmp->hardwareReg()), R32(reg->hardwareReg()));
        compiler.bfxil(R32(reg->hardwareReg()), R32(tmp->hardwareReg()), 0, 16);
    } else if (regWidth == JitWidth::b8) {
        RegPtr tmp = getTmpReg();

        if (!isRegHigh(reg)) {
            fn(R32(tmp->hardwareReg()), R32(reg->hardwareReg()));
            compiler.bfi(R32(reg->hardwareReg()), R32(tmp->hardwareReg()), 0, 8);
        } else {
            compiler.ubfx(R32(tmp->hardwareReg()), R32(reg->hardwareReg()), 8, 8);
            fn(R32(tmp->hardwareReg()), R32(tmp->hardwareReg()));
            compiler.bfi(R32(reg->hardwareReg()), R32(tmp->hardwareReg()), 8, 8);
        }
    } else {
        kpanic("JitArmV8CodeGen::reg");
    }
}

void JitArmV8CodeGen::regReg(JitWidth regWidth, RegPtr reg, RegPtr rm, std::function<void(asmjit::a64::Gp dst, asmjit::a64::Gp reg, asmjit::a64::Gp rm)> fn, U32 extend) {    
    if (regWidth == JitWidth::b32) {
        fn(R32(reg->hardwareReg()), R32(reg->hardwareReg()), R32(rm->hardwareReg()));
    } else if (regWidth == JitWidth::b64) {
        fn(R64(reg->hardwareReg()), R64(reg->hardwareReg()), R64(rm->hardwareReg()));
    } else if (regWidth == JitWidth::b16) {
        RegPtr tmp = getTmpReg();
        if (extend == ZERO_EXTEND) {
            compiler.ubfx(R32(tmp->hardwareReg()), R32(reg->hardwareReg()), 0, 16);
            fn(R32(tmp->hardwareReg()), R32(tmp->hardwareReg()), R32(rm->hardwareReg()));
        } else if (extend == SIGN_EXTEND) {
            movsx(JitWidth::b32, tmp, JitWidth::b16, reg);
            fn(R32(tmp->hardwareReg()), R32(tmp->hardwareReg()), R32(rm->hardwareReg()));
        } else {
            fn(R32(tmp->hardwareReg()), R32(reg->hardwareReg()), R32(rm->hardwareReg()));
        }
        compiler.bfxil(R32(reg->hardwareReg()), R32(tmp->hardwareReg()), 0, 16);
    } else if (regWidth == JitWidth::b8) {
        rm = getReg8InLowByte(std::move(rm));
        RegPtr tmp = getTmpReg();

        if (!isRegHigh(reg)) {
            if (extend == ZERO_EXTEND) {
                compiler.ubfx(R32(tmp->hardwareReg()), R32(reg->hardwareReg()), 0, 8);
                fn(R32(tmp->hardwareReg()), R32(tmp->hardwareReg()), R32(rm->hardwareReg()));
            } else if (extend == SIGN_EXTEND) {
                movsx(JitWidth::b32, tmp, JitWidth::b8, reg);
                fn(R32(tmp->hardwareReg()), R32(tmp->hardwareReg()), R32(rm->hardwareReg()));
            } else {
                fn(R32(tmp->hardwareReg()), R32(reg->hardwareReg()), R32(rm->hardwareReg()));
            }
            compiler.bfxil(R32(reg->hardwareReg()), R32(tmp->hardwareReg()), 0, 8);
        } else {
            if (extend == SIGN_EXTEND) {
                movsx(JitWidth::b32, tmp, JitWidth::b8, reg);
            } else {
                compiler.ubfx(R32(tmp->hardwareReg()), R32(reg->hardwareReg()), 8, 8);
            }
            fn(R32(tmp->hardwareReg()), R32(tmp->hardwareReg()), R32(rm->hardwareReg()));
            compiler.bfi(R32(reg->hardwareReg()), R32(tmp->hardwareReg()), 8, 8);
        }
    } else {
        kpanic("JitArmV8CodeGen::regReg");
    }
}

void JitArmV8CodeGen::regValue(JitWidth regWidth, RegPtr reg, U32 value, std::function<void(asmjit::a64::Gp dst, asmjit::a64::Gp reg, U32 value)> fn, U32 extend) {
    if (regWidth == JitWidth::b32) {
        fn(R32(reg->hardwareReg()), R32(reg->hardwareReg()), value);
    } else if (regWidth == JitWidth::b64) {
        fn(R64(reg->hardwareReg()), R64(reg->hardwareReg()), value);
    } else if (regWidth == JitWidth::b16) {
        RegPtr tmp = getTmpReg();

        if (extend == ZERO_EXTEND) {
            compiler.ubfx(R32(tmp->hardwareReg()), R32(reg->hardwareReg()), 0, 16);
            fn(R32(tmp->hardwareReg()), R32(tmp->hardwareReg()), value);
        } else if (extend == SIGN_EXTEND) {
            movsx(JitWidth::b32, tmp, JitWidth::b16, reg);
            fn(R32(tmp->hardwareReg()), R32(tmp->hardwareReg()), value);
        } else {
            fn(R32(tmp->hardwareReg()), R32(reg->hardwareReg()), value);
        }
        compiler.bfxil(R32(reg->hardwareReg()), R32(tmp->hardwareReg()), 0, 16);
    } else if (regWidth == JitWidth::b8) {
        RegPtr tmp = getTmpReg();

        if (!isRegHigh(reg)) {
            if (extend == ZERO_EXTEND) {
                compiler.ubfx(R32(tmp->hardwareReg()), R32(reg->hardwareReg()), 0, 8);
                fn(R32(tmp->hardwareReg()), R32(tmp->hardwareReg()), value);
            } else if (extend == SIGN_EXTEND) {
                movsx(JitWidth::b32, tmp, JitWidth::b8, reg);
                fn(R32(tmp->hardwareReg()), R32(tmp->hardwareReg()), value);
            } else {
                fn(R32(tmp->hardwareReg()), R32(reg->hardwareReg()), value);
            }
            compiler.bfxil(R32(reg->hardwareReg()), R32(tmp->hardwareReg()), 0, 8);
        } else {
            if (extend == SIGN_EXTEND) {
                movsx(JitWidth::b32, tmp, JitWidth::b8, reg);
            } else {
                compiler.ubfx(R32(tmp->hardwareReg()), R32(reg->hardwareReg()), 8, 8);
            }
            fn(R32(tmp->hardwareReg()), R32(tmp->hardwareReg()), value);
            compiler.bfi(R32(reg->hardwareReg()), R32(tmp->hardwareReg()), 8, 8);
        }
    } else {
        kpanic("JitArmV8CodeGen::addValue");
    }
}

void JitArmV8CodeGen::addReg(JitWidth regWidth, RegPtr reg, RegPtr rm) {
    regReg(regWidth, reg, rm, [this](asmjit::a64::Gp dst, asmjit::a64::Gp reg, asmjit::a64::Gp rm) {
        compiler.add(dst, reg, rm);
    });
}

void JitArmV8CodeGen::addValue(JitWidth regWidth, RegPtr reg, U32 imm) {
    if (!asmjit::a64::Utils::is_add_sub_imm(imm)) {
        addReg(regWidth, reg, loadConst(imm));
        return;
    }
    regValue(regWidth, reg, imm, [this](asmjit::a64::Gp dst, asmjit::a64::Gp reg, U32 value) {
        compiler.add(dst, reg, value);
    });
}

void JitArmV8CodeGen::orReg(JitWidth regWidth, RegPtr reg, RegPtr rm) {
    regReg(regWidth, reg, rm, [this](asmjit::a64::Gp dst, asmjit::a64::Gp reg, asmjit::a64::Gp rm) {
        compiler.orr(dst, reg, rm);
    });
}

void JitArmV8CodeGen::orValue(JitWidth regWidth, RegPtr reg, U32 imm) {
    if (!asmjit::a64::Utils::is_logical_imm(imm, 32)) {
        orReg(regWidth, reg, loadConst(imm));
        return;
    }
    regValue(regWidth, reg, imm, [this](asmjit::a64::Gp dst, asmjit::a64::Gp reg, U32 value) {
        compiler.orr(dst, reg, value);
    });
}

void JitArmV8CodeGen::subReg(JitWidth regWidth, RegPtr reg, RegPtr rm) {
    regReg(regWidth, reg, rm, [this](asmjit::a64::Gp dst, asmjit::a64::Gp reg, asmjit::a64::Gp rm) {
        compiler.sub(dst, reg, rm);
    });
}

void JitArmV8CodeGen::subValue(JitWidth regWidth, RegPtr reg, U32 imm) {
    if (!asmjit::a64::Utils::is_add_sub_imm(imm)) {
        subReg(regWidth, reg, loadConst(imm));
        return;
    }
    regValue(regWidth, reg, imm, [this](asmjit::a64::Gp dst, asmjit::a64::Gp reg, U32 value) {
        compiler.sub(dst, reg, value);
    });
}

void JitArmV8CodeGen::andReg(JitWidth regWidth, RegPtr reg, RegPtr rm) {
    regReg(regWidth, reg, rm, [this](asmjit::a64::Gp dst, asmjit::a64::Gp reg, asmjit::a64::Gp rm) {
        compiler.and_(dst, reg, rm);
    });
}

void JitArmV8CodeGen::andValue(JitWidth regWidth, RegPtr reg, U32 imm) {
    if (!asmjit::a64::Utils::is_logical_imm(imm, 32)) {
        andReg(regWidth, reg, loadConst(imm));
        return;
    }
    regValue(regWidth, reg, imm, [regWidth, this](asmjit::a64::Gp dst, asmjit::a64::Gp reg, U32 value) {
        compiler.and_(dst, reg, value);
    });
}

void JitArmV8CodeGen::andValue64(RegPtr reg, U64 imm) {
    if (!asmjit::a64::Utils::is_logical_imm(imm, 64)) {
        RegPtr value = loadConst(imm);
        compiler.and_(R64(reg->hardwareReg()), R64(reg->hardwareReg()), R64(value->hardwareReg()));
        return;
    }
    compiler.and_(R64(reg->hardwareReg()), R64(reg->hardwareReg()), imm);
}

void JitArmV8CodeGen::xorReg(JitWidth regWidth, RegPtr reg, RegPtr rm) {
    regReg(regWidth, reg, rm, [this](asmjit::a64::Gp dst, asmjit::a64::Gp reg, asmjit::a64::Gp rm) {
        compiler.eor(dst, reg, rm);
    });
}

void JitArmV8CodeGen::xorValue(JitWidth regWidth, RegPtr reg, U32 imm) {
    if (!asmjit::a64::Utils::is_logical_imm(imm, 32)) {
        xorReg(regWidth, reg, loadConst(imm));
        return;
    }
    regValue(regWidth, reg, imm, [this](asmjit::a64::Gp dst, asmjit::a64::Gp reg, U32 value) {
        compiler.eor(dst, reg, value);
    });
}

void JitArmV8CodeGen::shrReg(JitWidth regWidth, RegPtr reg, RegPtr rm) {
    RegPtr cl = getTmpReg();
    compiler.and_(R32(cl->hardwareReg()), R32(rm->hardwareReg()), regWidth == JitWidth::b64 ? 0x3f : 0x1f);
    regReg(regWidth, reg, cl, [this](asmjit::a64::Gp dst, asmjit::a64::Gp reg, asmjit::a64::Gp rm) {
        compiler.lsr(dst, reg, rm);
    }, ZERO_EXTEND);
}

void JitArmV8CodeGen::shrValue(JitWidth regWidth, RegPtr reg, U32 imm) {
    regValue(regWidth, reg, imm, [this](asmjit::a64::Gp dst, asmjit::a64::Gp reg, U32 value) {
        compiler.lsr(dst, reg, value);
    }, ZERO_EXTEND);
}

void JitArmV8CodeGen::shlReg(JitWidth regWidth, RegPtr reg, RegPtr rm) {
    RegPtr cl = getTmpReg();
    compiler.and_(R32(cl->hardwareReg()), R32(rm->hardwareReg()), regWidth == JitWidth::b64 ? 0x3f : 0x1f);
    regReg(regWidth, reg, cl, [this](asmjit::a64::Gp dst, asmjit::a64::Gp reg, asmjit::a64::Gp rm) {
        compiler.lsl(dst, reg, rm);
    });
}

void JitArmV8CodeGen::shlValue(JitWidth regWidth, RegPtr reg, U32 imm) {
    regValue(regWidth, reg, imm, [this](asmjit::a64::Gp dst, asmjit::a64::Gp reg, U32 value) {
        compiler.lsl(dst, reg, value);
    });
}

void JitArmV8CodeGen::sarReg(JitWidth regWidth, RegPtr reg, RegPtr rm) {
    RegPtr cl = getTmpReg();
    compiler.and_(R32(cl->hardwareReg()), R32(rm->hardwareReg()), regWidth == JitWidth::b64 ? 0x3f : 0x1f);
    regReg(regWidth, reg, cl, [this](asmjit::a64::Gp dst, asmjit::a64::Gp reg, asmjit::a64::Gp rm) {
        compiler.asr(dst, reg, rm);
    }, SIGN_EXTEND);
}

void JitArmV8CodeGen::sarValue(JitWidth regWidth, RegPtr reg, U32 imm) {
    regValue(regWidth, reg, imm, [this](asmjit::a64::Gp dst, asmjit::a64::Gp reg, U32 value) {
        compiler.asr(dst, reg, value);
    }, SIGN_EXTEND);
}

void JitArmV8CodeGen::notReg2(JitWidth regWidth, RegPtr reg) {
    reg1(regWidth, reg, [this](asmjit::a64::Gp dst, asmjit::a64::Gp reg) {
        compiler.mvn_(dst, reg);
    });
}

void JitArmV8CodeGen::negReg2(JitWidth regWidth, RegPtr reg) {
    reg1(regWidth, reg, [this](asmjit::a64::Gp dst, asmjit::a64::Gp reg) {
        compiler.neg(dst, reg);
    });
}

void JitArmV8CodeGen::bsfReg(JitWidth regWidth, RegPtr reg, RegPtr rm) {
    // rm was already checked to not be 0 before getting here
    if (regWidth == JitWidth::b32) {
        compiler.rbit(R32(reg->hardwareReg()), R32(rm->hardwareReg()));
        compiler.clz(R32(reg->hardwareReg()), R32(reg->hardwareReg()));
    } else if (regWidth == JitWidth::b16) {
        RegPtr tmp = getTmpReg();
        movzx(JitWidth::b32, tmp, JitWidth::b16, rm);
        compiler.rbit(R32(tmp->hardwareReg()), R32(tmp->hardwareReg()));
        compiler.clz(R32(tmp->hardwareReg()), R32(tmp->hardwareReg()));
        compiler.bfxil(R32(reg->hardwareReg()), R32(tmp->hardwareReg()), 0, 16);        
    } else {
        kpanic("JitArmV8CodeGen::bsfReg");
    }
}

void JitArmV8CodeGen::bsrReg(JitWidth regWidth, RegPtr reg, RegPtr rm) {
    // rm was already checked to not be 0 before getting here
    if (regWidth == JitWidth::b32) {
        compiler.clz(R32(reg->hardwareReg()), R32(rm->hardwareReg()));
        compiler.sub(R32(reg->hardwareReg()), R32(reg->hardwareReg()), 31);
        compiler.neg(R32(reg->hardwareReg()), R32(reg->hardwareReg()));
    } else if (regWidth == JitWidth::b16) {
        RegPtr tmp = getTmpReg();
        movzx(JitWidth::b32, tmp, JitWidth::b16, rm);
        compiler.clz(R32(tmp->hardwareReg()), R32(tmp->hardwareReg()));
        compiler.sub(R32(tmp->hardwareReg()), R32(tmp->hardwareReg()), 31);
        compiler.neg(R32(tmp->hardwareReg()), R32(tmp->hardwareReg()));
        compiler.bfxil(R32(reg->hardwareReg()), R32(tmp->hardwareReg()), 0, 16);
    } else {
        kpanic("JitArmV8CodeGen::bsrReg");
    }
}

void JitArmV8CodeGen::rolReg(JitWidth regWidth, RegPtr reg, RegPtr rm) {
    RegPtr cl = getTmpReg();    
    if (regWidth == JitWidth::b32) {
        RegPtr tmp = loadConst(32);
        compiler.and_(R32(cl->hardwareReg()), R32(rm->hardwareReg()), 0x1f);
        compiler.sub(R32(cl->hardwareReg()), R32(tmp->hardwareReg()), R32(cl->hardwareReg()));
        compiler.ror(R32(reg->hardwareReg()), R32(reg->hardwareReg()), R32(cl->hardwareReg()));
    } else if (regWidth == JitWidth::b16) {
        RegPtr tmp = getTmpReg();
        RegPtr tmp2 = loadConst(16);
        RegPtr tmp3 = getTmpReg();
        compiler.and_(R32(cl->hardwareReg()), R32(rm->hardwareReg()), 0xf);
        movzx(JitWidth::b32, tmp3, JitWidth::b16, reg);
        compiler.lsl(R32(tmp->hardwareReg()), R32(tmp3->hardwareReg()), R32(cl->hardwareReg()));
        compiler.sub(R32(tmp2->hardwareReg()), R32(tmp2->hardwareReg()), R32(cl->hardwareReg()));
        compiler.lsr(R32(tmp2->hardwareReg()), R32(tmp3->hardwareReg()), R32(tmp2->hardwareReg()));
        compiler.orr(R32(tmp->hardwareReg()), R32(tmp->hardwareReg()), R32(tmp2->hardwareReg()));
        compiler.bfxil(R32(reg->hardwareReg()), R32(tmp->hardwareReg()), 0, 16);
    } else if (regWidth == JitWidth::b8) {
        RegPtr tmp = getTmpReg();
        RegPtr tmp2 = loadConst(8);
        RegPtr tmp3 = getTmpReg();
        compiler.and_(R32(cl->hardwareReg()), R32(rm->hardwareReg()), 0x7);
        movzx(JitWidth::b32, tmp3, JitWidth::b8, reg);
        compiler.lsl(R32(tmp->hardwareReg()), R32(tmp3->hardwareReg()), R32(cl->hardwareReg()));
        compiler.sub(R32(tmp2->hardwareReg()), R32(tmp2->hardwareReg()), R32(cl->hardwareReg()));
        compiler.lsr(R32(tmp2->hardwareReg()), R32(tmp3->hardwareReg()), R32(tmp2->hardwareReg()));
        compiler.orr(R32(tmp->hardwareReg()), R32(tmp->hardwareReg()), R32(tmp2->hardwareReg()));
        compiler.bfi(R32(reg->hardwareReg()), R32(tmp->hardwareReg()), reg->isHigh ? 8 : 0, 8);
    } else {
        kpanic("JitArmV8CodeGen::rolReg");
    }
}

void JitArmV8CodeGen::rolValue(JitWidth regWidth, RegPtr reg, U32 imm) {
    if (regWidth == JitWidth::b32) {
        imm = imm % 32;
        if (imm == 0) {
            return;
        }
        compiler.ror(R32(reg->hardwareReg()), R32(reg->hardwareReg()), 32-imm);
    } else if (regWidth == JitWidth::b16) {
        RegPtr tmp = getTmpReg();
        imm = imm % 16;
        if (imm == 0) {
            return;
        }
        compiler.lsl(R32(tmp->hardwareReg()), R32(reg->hardwareReg()), imm);
        compiler.bfxil(R32(tmp->hardwareReg()), R32(reg->hardwareReg()), 16 - imm, imm);
        compiler.bfxil(R32(reg->hardwareReg()), R32(tmp->hardwareReg()), 0, 16);
    } else if (regWidth == JitWidth::b8) {
        imm = imm % 8;
        if (imm == 0) {
            return;
        }
        RegPtr tmp = getTmpReg();
        if (reg->isHigh) {
            compiler.lsr(R32(tmp->hardwareReg()), R32(reg->hardwareReg()), 8 - imm);
            compiler.bfxil(R32(tmp->hardwareReg()), R32(reg->hardwareReg()), 16 - imm, imm);
            compiler.bfi(R32(reg->hardwareReg()), R32(tmp->hardwareReg()), 8, 8);
        } else {
            compiler.lsl(R32(tmp->hardwareReg()), R32(reg->hardwareReg()), imm);
            compiler.bfxil(R32(tmp->hardwareReg()), R32(reg->hardwareReg()), 8 - imm, imm);
            compiler.bfxil(R32(reg->hardwareReg()), R32(tmp->hardwareReg()), 0, 8);
        }
    } else {
        kpanic("JitArmV8CodeGen::rolValue");
    }
}

void JitArmV8CodeGen::rorReg(JitWidth regWidth, RegPtr reg, RegPtr rm) {
    RegPtr cl = getTmpReg();
    if (regWidth == JitWidth::b32) {
        compiler.and_(R32(cl->hardwareReg()), R32(rm->hardwareReg()), 0x1f);
        compiler.ror(R32(reg->hardwareReg()), R32(reg->hardwareReg()), R32(cl->hardwareReg()));
    } else if (regWidth == JitWidth::b16) {
        RegPtr tmp = getTmpReg();
        RegPtr tmp2 = loadConst(16);
        RegPtr tmp3 = getTmpReg();
        compiler.and_(R32(cl->hardwareReg()), R32(rm->hardwareReg()), 0xf);
        movzx(JitWidth::b32, tmp3, JitWidth::b16, reg);
        compiler.lsr(R32(tmp->hardwareReg()), R32(tmp3->hardwareReg()), R32(cl->hardwareReg()));
        compiler.sub(R32(tmp2->hardwareReg()), R32(tmp2->hardwareReg()), R32(cl->hardwareReg()));
        compiler.lsl(R32(tmp2->hardwareReg()), R32(tmp3->hardwareReg()), R32(tmp2->hardwareReg()));
        compiler.orr(R32(tmp->hardwareReg()), R32(tmp->hardwareReg()), R32(tmp2->hardwareReg()));
        compiler.bfxil(R32(reg->hardwareReg()), R32(tmp->hardwareReg()), 0, 16);
    } else if (regWidth == JitWidth::b8) {
        RegPtr tmp = getTmpReg();
        RegPtr tmp2 = loadConst(8);
        RegPtr tmp3 = getTmpReg();
        compiler.and_(R32(cl->hardwareReg()), R32(rm->hardwareReg()), 0x7);
        movzx(JitWidth::b32, tmp3, JitWidth::b8, reg);
        compiler.lsr(R32(tmp->hardwareReg()), R32(tmp3->hardwareReg()), R32(cl->hardwareReg()));
        compiler.sub(R32(tmp2->hardwareReg()), R32(tmp2->hardwareReg()), R32(cl->hardwareReg()));
        compiler.lsl(R32(tmp2->hardwareReg()), R32(tmp3->hardwareReg()), R32(tmp2->hardwareReg()));
        compiler.orr(R32(tmp->hardwareReg()), R32(tmp->hardwareReg()), R32(tmp2->hardwareReg()));
        compiler.bfi(R32(reg->hardwareReg()), R32(tmp->hardwareReg()), reg->isHigh ? 8 : 0, 8);
    } else {
        kpanic("JitArmV8CodeGen::rorReg");
    }
}

void JitArmV8CodeGen::rorValue(JitWidth regWidth, RegPtr reg, U32 imm) {
    if (regWidth == JitWidth::b32) {
        imm = imm % 32;
        if (imm == 0) {
            return;
        }
        compiler.ror(R32(reg->hardwareReg()), R32(reg->hardwareReg()), imm);
    }
    else if (regWidth == JitWidth::b16) {
        RegPtr tmp = getTmpReg();
        imm = imm % 16;
        if (imm == 0) {
            return;
        }
        compiler.lsr(R32(tmp->hardwareReg()), R32(reg->hardwareReg()), imm);
        compiler.bfi(R32(tmp->hardwareReg()), R32(reg->hardwareReg()), 16 - imm, imm);
        compiler.bfxil(R32(reg->hardwareReg()), R32(tmp->hardwareReg()), 0, 16);
    }
    else if (regWidth == JitWidth::b8) {
        imm = imm % 8;
        if (imm == 0) {
            return;
        }
        RegPtr tmp = getTmpReg();
        if (reg->isHigh) {
            RegPtr low8 = getReg8InLowByte(reg);
            compiler.lsr(R32(tmp->hardwareReg()), R32(low8->hardwareReg()), imm);
            compiler.bfi(R32(tmp->hardwareReg()), R32(low8->hardwareReg()), 8 - imm, imm);
            compiler.bfi(R32(reg->hardwareReg()), R32(tmp->hardwareReg()), 8, 8);
        }
        else {
            compiler.lsr(R32(tmp->hardwareReg()), R32(reg->hardwareReg()), imm);
            compiler.bfi(R32(tmp->hardwareReg()), R32(reg->hardwareReg()), 8 - imm, imm);
            compiler.bfxil(R32(reg->hardwareReg()), R32(tmp->hardwareReg()), 0, 8);
        }
    }
    else {
        kpanic("JitArmV8CodeGen::rorValue");
    }
}

void JitArmV8CodeGen::modValue32(RegPtr dst, RegPtr src, RegPtr value) {
    RegPtr tmp = getTmpReg(); // in case dst == src
    compiler.udiv(R32(tmp->hardwareReg()), R32(src->hardwareReg()), R32(value->hardwareReg()));
    compiler.msub(R32(dst->hardwareReg()), R32(tmp->hardwareReg()), R32(value->hardwareReg()), R32(src->hardwareReg()));
}

void JitArmV8CodeGen::rclReg(JitWidth regWidth, RegPtr reg, RegPtr rm) {
    RegPtr cf = getCF();

    // result = (var1 << var2) | (cf << (var2 - 1)) | (var1 >> (33 - var2));
    if (regWidth == JitWidth::b32) {        
        RegPtr cl = getTmpReg();
        RegPtr tmp1 = getTmpReg();

        // (var1 << var2)
        compiler.and_(R32(cl->hardwareReg()), R32(rm->hardwareReg()), 0x1f);
        compiler.lsl(R32(tmp1->hardwareReg()), R32(reg->hardwareReg()), R32(cl->hardwareReg()));

        // (var1 >> (33 - var2))
        RegPtr tmp2 = loadConst(33);
        compiler.sub(R32(tmp2->hardwareReg()), R32(tmp2->hardwareReg()), R32(cl->hardwareReg()));
        compiler.lsr(R64(tmp2->hardwareReg()), R64(reg->hardwareReg()), R64(tmp2->hardwareReg())); // R64 so that we shift in 0's in the case of cl = 1
        
        compiler.orr(R32(tmp1->hardwareReg()), R32(tmp2->hardwareReg()), R32(tmp1->hardwareReg()));

        // (cf << (var2 - 1))
        compiler.sub(R32(tmp2->hardwareReg()), R32(cl->hardwareReg()), 1);
        compiler.lsl(R32(tmp2->hardwareReg()), R32(cf->hardwareReg()), R32(tmp2->hardwareReg()));
        
        compiler.orr(R32(reg->hardwareReg()), R32(tmp2->hardwareReg()), R32(tmp1->hardwareReg()));        
    } else if (regWidth == JitWidth::b16) {
        RegPtr cl = getTmpReg();
        RegPtr tmp1 = getTmpReg();
        RegPtr tmp2 = loadConst(17);
        RegPtr src = getTmpReg();

        movzx(JitWidth::b32, src, JitWidth::b16, reg);
        // (var1 << var2)
        compiler.and_(R32(cl->hardwareReg()), R32(rm->hardwareReg()), 0x1f);
        modValue32(cl, cl, tmp2);
        compiler.lsl(R32(tmp1->hardwareReg()), R32(src->hardwareReg()), R32(cl->hardwareReg()));

        // (var1 >> (17 - var2))        
        compiler.sub(R32(tmp2->hardwareReg()), R32(tmp2->hardwareReg()), R32(cl->hardwareReg()));
        compiler.lsr(R32(tmp2->hardwareReg()), R32(src->hardwareReg()), R32(tmp2->hardwareReg()));

        compiler.orr(R32(tmp1->hardwareReg()), R32(tmp2->hardwareReg()), R32(tmp1->hardwareReg()));

        // (cf << (var2 - 1))
        compiler.sub(R32(tmp2->hardwareReg()), R32(cl->hardwareReg()), 1);
        compiler.lsl(R32(tmp2->hardwareReg()), R32(cf->hardwareReg()), R32(tmp2->hardwareReg()));

        compiler.orr(R32(tmp1->hardwareReg()), R32(tmp2->hardwareReg()), R32(tmp1->hardwareReg()));
        compiler.bfxil(R32(reg->hardwareReg()), R32(tmp1->hardwareReg()), 0, 16);
    } else if (regWidth == JitWidth::b8) {
        RegPtr cl = getTmpReg();
        RegPtr tmp1 = getTmpReg();
        RegPtr tmp2 = loadConst(9);
        RegPtr src = getTmpReg();

        movzx(JitWidth::b32, src, JitWidth::b8, reg);
        // (var1 << var2)
        compiler.and_(R32(cl->hardwareReg()), R32(rm->hardwareReg()), 0x1f);
        modValue32(cl, cl, tmp2);
        compiler.lsl(R32(tmp1->hardwareReg()), R32(src->hardwareReg()), R32(cl->hardwareReg()));

        // (var1 >> (9 - var2))        
        compiler.sub(R32(tmp2->hardwareReg()), R32(tmp2->hardwareReg()), R32(cl->hardwareReg()));
        compiler.lsr(R32(tmp2->hardwareReg()), R32(src->hardwareReg()), R32(tmp2->hardwareReg()));

        compiler.orr(R32(tmp1->hardwareReg()), R32(tmp2->hardwareReg()), R32(tmp1->hardwareReg()));

        // (cf << (var2 - 1))
        compiler.sub(R32(tmp2->hardwareReg()), R32(cl->hardwareReg()), 1);
        compiler.lsl(R32(tmp2->hardwareReg()), R32(cf->hardwareReg()), R32(tmp2->hardwareReg()));

        compiler.orr(R32(tmp1->hardwareReg()), R32(tmp2->hardwareReg()), R32(tmp1->hardwareReg()));
        compiler.bfi(R32(reg->hardwareReg()), R32(tmp1->hardwareReg()), reg->isHigh ? 8 : 0, 8);
    } else {
        kpanic("JitArmV8CodeGen::rclValue");
    }
}

void JitArmV8CodeGen::rclValue(JitWidth regWidth, RegPtr reg, U32 imm) {
    RegPtr cf = getCF();

    // result = (var1 << var2) | ((cpu->flags & CF) << (var2 - 1)) | (var1 >> (9 - var2));
    if (regWidth == JitWidth::b32) {
        RegPtr tmp = getTmpReg();
        imm = imm % 33;
        if (imm == 0) {
            return;
        }
        if (imm == 1) {
            compiler.lsl(R32(reg->hardwareReg()), R32(reg->hardwareReg()), imm);
            compiler.bfi(R32(reg->hardwareReg()), R32(cf->hardwareReg()), imm - 1, 1);
        } else {
            compiler.lsl(R32(tmp->hardwareReg()), R32(reg->hardwareReg()), imm);
            compiler.bfxil(R32(tmp->hardwareReg()), R32(reg->hardwareReg()), 33 - imm, imm - 1);
            compiler.bfi(R32(tmp->hardwareReg()), R32(cf->hardwareReg()), imm - 1, 1);
            compiler.mov(R32(reg->hardwareReg()), R32(tmp->hardwareReg()));
        }
    } else if (regWidth == JitWidth::b16) {
        RegPtr tmp = getTmpReg();
        imm = imm % 17;
        if (imm == 0) {
            return;
        }
        compiler.lsl(R32(tmp->hardwareReg()), R32(reg->hardwareReg()), imm);
        if (imm != 1) {
            compiler.bfxil(R32(tmp->hardwareReg()), R32(reg->hardwareReg()), 17 - imm, imm - 1);
        }
        compiler.bfi(R32(tmp->hardwareReg()), R32(cf->hardwareReg()), imm - 1, 1);
        compiler.bfxil(R32(reg->hardwareReg()), R32(tmp->hardwareReg()), 0, 16);
    } else if (regWidth == JitWidth::b8) {
        imm = imm % 9;
        if (imm == 0) {
            return;
        }
        RegPtr tmp = getTmpReg();
        if (reg->isHigh) {
            compiler.lsr(R32(tmp->hardwareReg()), R32(reg->hardwareReg()), 8 - imm);
            if (imm != 1) {
                compiler.bfxil(R32(tmp->hardwareReg()), R32(reg->hardwareReg()), 17 - imm, imm - 1);
            }
            compiler.bfi(R32(tmp->hardwareReg()), R32(cf->hardwareReg()), imm - 1, 1);
            compiler.bfi(R32(reg->hardwareReg()), R32(tmp->hardwareReg()), 8, 8);
        } else {
            compiler.lsl(R32(tmp->hardwareReg()), R32(reg->hardwareReg()), imm);
            if (imm != 1) {
                compiler.bfxil(R32(tmp->hardwareReg()), R32(reg->hardwareReg()), 9 - imm, imm - 1);
            }
            compiler.bfi(R32(tmp->hardwareReg()), R32(cf->hardwareReg()), imm - 1, 1);
            compiler.bfxil(R32(reg->hardwareReg()), R32(tmp->hardwareReg()), 0, 8);
        }
    } else {
        kpanic("JitArmV8CodeGen::rclValue");
    }
}

void JitArmV8CodeGen::rcrReg(JitWidth regWidth, RegPtr reg, RegPtr rm) {
    RegPtr cf = getCF();

    // result = (var1 >> var2) | (cf << (32 - var2)) | (var1 << (33 - var2));
    if (regWidth == JitWidth::b32) {
        RegPtr cl = getTmpReg();
        RegPtr tmp1 = getTmpReg();

        // (var1 >> var2)
        compiler.and_(R32(cl->hardwareReg()), R32(rm->hardwareReg()), 0x1f);
        compiler.lsr(R32(tmp1->hardwareReg()), R32(reg->hardwareReg()), R32(cl->hardwareReg()));

        // (var1 << (33 - var2))
        RegPtr tmp2 = loadConst(33);
        compiler.sub(R32(tmp2->hardwareReg()), R32(tmp2->hardwareReg()), R32(cl->hardwareReg()));
        compiler.lsl(R64(tmp2->hardwareReg()), R64(reg->hardwareReg()), R64(tmp2->hardwareReg())); // R64, otherwise in the case of cl = 1, 33 - 1 = 32, which does nothing on a 32-bit shift

        compiler.orr(R32(tmp1->hardwareReg()), R32(tmp2->hardwareReg()), R32(tmp1->hardwareReg()));

        // (cf << (32 - var2))
        RegPtr tmp3 = loadConst(32);
        compiler.sub(R32(tmp2->hardwareReg()), R32(tmp3->hardwareReg()), R32(cl->hardwareReg()));
        compiler.lsl(R32(tmp2->hardwareReg()), R32(cf->hardwareReg()), R32(tmp2->hardwareReg()));

        compiler.orr(R32(reg->hardwareReg()), R32(tmp2->hardwareReg()), R32(tmp1->hardwareReg()));
    } else if (regWidth == JitWidth::b16) {
        RegPtr cl = getTmpReg();
        RegPtr tmp1 = getTmpReg();
        RegPtr tmp2 = loadConst(17);
        RegPtr src = getTmpReg();

        movzx(JitWidth::b32, src, JitWidth::b16, reg);
        // (var1 >> var2)
        compiler.and_(R32(cl->hardwareReg()), R32(rm->hardwareReg()), 0x1f);
        modValue32(cl, cl, tmp2);
        compiler.lsr(R32(tmp1->hardwareReg()), R32(src->hardwareReg()), R32(cl->hardwareReg()));

        // (var1 << (17 - var2))       
        compiler.sub(R32(tmp2->hardwareReg()), R32(tmp2->hardwareReg()), R32(cl->hardwareReg()));
        compiler.lsl(R32(tmp2->hardwareReg()), R32(src->hardwareReg()), R32(tmp2->hardwareReg()));

        compiler.orr(R32(tmp1->hardwareReg()), R32(tmp2->hardwareReg()), R32(tmp1->hardwareReg()));

        // (cf << (16 - var2))
        compiler.sub(R32(tmp2->hardwareReg()), R32(cl->hardwareReg()), 16);
        compiler.neg(R32(tmp2->hardwareReg()), R32(tmp2->hardwareReg())); // instead of load const, saves a tmp reg
        compiler.lsl(R32(tmp2->hardwareReg()), R32(cf->hardwareReg()), R32(tmp2->hardwareReg()));

        compiler.orr(R32(tmp1->hardwareReg()), R32(tmp2->hardwareReg()), R32(tmp1->hardwareReg()));
        compiler.bfxil(R32(reg->hardwareReg()), R32(tmp1->hardwareReg()), 0, 16);
    } else if (regWidth == JitWidth::b8) {
        RegPtr cl = getTmpReg();
        RegPtr tmp1 = getTmpReg();
        RegPtr tmp2 = loadConst(9);
        RegPtr src = getTmpReg();

        movzx(JitWidth::b32, src, JitWidth::b8, reg);
        // (var1 >> var2)
        compiler.and_(R32(cl->hardwareReg()), R32(rm->hardwareReg()), 0x1f);
        modValue32(cl, cl, tmp2);
        compiler.lsl(R32(tmp1->hardwareReg()), R32(src->hardwareReg()), R32(cl->hardwareReg()));

        // (var1 << (9 - var2))       
        compiler.sub(R32(tmp2->hardwareReg()), R32(tmp2->hardwareReg()), R32(cl->hardwareReg()));
        compiler.lsr(R32(tmp2->hardwareReg()), R32(src->hardwareReg()), R32(tmp2->hardwareReg()));

        compiler.orr(R32(tmp1->hardwareReg()), R32(tmp2->hardwareReg()), R32(tmp1->hardwareReg()));

        // (cf << (8 - var2))
        compiler.sub(R32(tmp2->hardwareReg()), R32(cl->hardwareReg()), 1);
        compiler.lsl(R32(tmp2->hardwareReg()), R32(cf->hardwareReg()), R32(tmp2->hardwareReg()));

        compiler.orr(R32(tmp1->hardwareReg()), R32(tmp2->hardwareReg()), R32(tmp1->hardwareReg()));
        compiler.bfi(R32(reg->hardwareReg()), R32(tmp1->hardwareReg()), reg->isHigh ? 8 : 0, 8);
    } else {
        kpanic("JitArmV8CodeGen::rclValue");
    }
}

void JitArmV8CodeGen::rcrValue(JitWidth regWidth, RegPtr reg, U32 imm) {
    RegPtr cf = getCF();

    // result = (var1 << var2) | ((cpu->flags & CF) << (var2 - 1)) | (var1 >> (9 - var2));
    if (regWidth == JitWidth::b32) {
        RegPtr tmp = getTmpReg();
        imm = imm % 33;
        if (imm == 0) {
            return;
        }
        if (imm == 1) {
            compiler.lsr(R32(reg->hardwareReg()), R32(reg->hardwareReg()), imm);
            compiler.bfi(R32(reg->hardwareReg()), R32(cf->hardwareReg()), 32 - imm, 1);
        } else {
            compiler.lsr(R32(tmp->hardwareReg()), R32(reg->hardwareReg()), imm);
            compiler.bfi(R32(tmp->hardwareReg()), R32(reg->hardwareReg()), 33 - imm, imm - 1);
            compiler.bfi(R32(tmp->hardwareReg()), R32(cf->hardwareReg()), 32 - imm, 1);
            compiler.mov(R32(reg->hardwareReg()), R32(tmp->hardwareReg()));
        }
    } else if (regWidth == JitWidth::b16) {
        RegPtr tmp = getTmpReg();
        imm = imm % 17;
        if (imm == 0) {
            return;
        }
        compiler.lsr(R32(tmp->hardwareReg()), R32(reg->hardwareReg()), imm);
        if (imm != 1) {
            compiler.bfi(R32(tmp->hardwareReg()), R32(reg->hardwareReg()), 17 - imm, imm - 1);
        }
        compiler.bfi(R32(tmp->hardwareReg()), R32(cf->hardwareReg()), 16 - imm, 1);
        compiler.bfxil(R32(reg->hardwareReg()), R32(tmp->hardwareReg()), 0, 16);
    } else if (regWidth == JitWidth::b8) {
        imm = imm % 9;
        if (imm == 0) {
            return;
        }
        RegPtr tmp = getTmpReg();
        if (reg->isHigh) {            
            if (imm == 1) {
                compiler.lsr(R32(tmp->hardwareReg()), R32(reg->hardwareReg()), 8 + imm);
            } else {
                compiler.lsr(R32(tmp->hardwareReg()), R32(reg->hardwareReg()), imm - 1);
                compiler.bfxil(R32(tmp->hardwareReg()), R32(reg->hardwareReg()), imm + 8, 8 - imm);
            }
            compiler.bfi(R32(tmp->hardwareReg()), R32(cf->hardwareReg()), 8 - imm, 1);
            compiler.bfi(R32(reg->hardwareReg()), R32(tmp->hardwareReg()), 8, 8);
        } else {
            compiler.lsr(R32(tmp->hardwareReg()), R32(reg->hardwareReg()), imm);
            if (imm != 1) {
                compiler.bfi(R32(tmp->hardwareReg()), R32(reg->hardwareReg()), 9 - imm, imm - 1);
            }
            compiler.bfi(R32(tmp->hardwareReg()), R32(cf->hardwareReg()), 8 - imm, 1);
            compiler.bfxil(R32(reg->hardwareReg()), R32(tmp->hardwareReg()), 0, 8);
        }
    } else {
        kpanic("JitArmV8CodeGen::rcrValue");
    }
}

void JitArmV8CodeGen::shldReg(JitWidth regWidth, RegPtr reg, RegPtr rm, RegPtr cl) {
    RegPtr tmp = getTmpReg();

    if (regWidth == JitWidth::b32) {
        // cl already masked by 0x1f
        // cpu->result.u32=(cpu->reg[reg].u32 << cl) | (cpu->reg[rm].u32 >> (32-cl));        
        compiler.lsl(R32(tmp->hardwareReg()), R32(reg->hardwareReg()), R32(cl->hardwareReg()));
        RegPtr tmp1 = loadConst(32);
        compiler.sub(R32(tmp1->hardwareReg()), R32(tmp1->hardwareReg()), R32(cl->hardwareReg()));
        compiler.lsr(R32(reg->hardwareReg()), R32(rm->hardwareReg()), R32(tmp1->hardwareReg()));
        compiler.orr(R32(reg->hardwareReg()), R32(reg->hardwareReg()), R32(tmp->hardwareReg()));
    } else if (regWidth == JitWidth::b16) {
        RegPtr tmp = getTmpReg();
        RegPtr reg16 = getTmpReg();
        RegPtr rm16 = getTmpReg();

        movzx(JitWidth::b32, reg16, JitWidth::b16, reg);
        movzx(JitWidth::b32, rm16, JitWidth::b16, rm);
        IfGreaterThanOrEqual(JitWidth::b32, cl, 0x10); {
            subValue(JitWidth::b32, cl, 16);
            mov(JitWidth::b32, reg16, rm16);
        } EndIf();
        compiler.lsl(R32(tmp->hardwareReg()), R32(reg16->hardwareReg()), R32(cl->hardwareReg()));
        RegPtr tmp1 = loadConst(16);
        compiler.sub(R32(tmp1->hardwareReg()), R32(tmp1->hardwareReg()), R32(cl->hardwareReg()));
        compiler.lsr(R32(reg16->hardwareReg()), R32(rm16->hardwareReg()), R32(tmp1->hardwareReg()));
        compiler.orr(R32(reg16->hardwareReg()), R32(reg16->hardwareReg()), R32(tmp->hardwareReg()));
        compiler.bfxil(R32(reg->hardwareReg()), R32(reg16->hardwareReg()), 0, 16);
    } else {
        kpanic("JitArmV8CodeGen::shldValue");
    }
}

void JitArmV8CodeGen::shldValue(JitWidth regWidth, RegPtr reg, RegPtr rm, U32 imm) {
    // don't need to check if imm is 0, that was handled in the decoder, if it was 0, the decoder will replace shld with nop

    if (regWidth == JitWidth::b32) {
        // cpu->result.u32=(cpu->reg[reg].u32 << imm) | (cpu->reg[rm].u32 >> (32-imm));
        RegPtr tmp = getTmpReg();
        compiler.bfi(R32(tmp->hardwareReg()), R32(reg->hardwareReg()), imm, 32 - imm);
        compiler.bfxil(R32(tmp->hardwareReg()), R32(rm->hardwareReg()), 32 - imm, imm);
        compiler.mov(R32(reg->hardwareReg()), R32(tmp->hardwareReg()));
    } else if (regWidth == JitWidth::b16) {
        RegPtr tmp = getTmpReg();
        if (imm >= 16) {
            imm -= 16;
            mov(JitWidth::b16, reg, rm);
        }
        if (imm != 0) {
            compiler.bfi(R32(tmp->hardwareReg()), R32(reg->hardwareReg()), imm, 16 - imm);
            compiler.bfxil(R32(tmp->hardwareReg()), R32(rm->hardwareReg()), 16 - imm, imm);
            compiler.bfxil(R32(reg->hardwareReg()), R32(tmp->hardwareReg()), 0, 16);
        }
    } else {
        kpanic("JitArmV8CodeGen::shldValue");
    }
}

void JitArmV8CodeGen::shrdReg(JitWidth regWidth, RegPtr reg, RegPtr rm, RegPtr cl) {
    RegPtr tmp = getTmpReg();

    if (regWidth == JitWidth::b32) {
        // cl already masked by 0x1f
        // cpu->result.u32 = (cpu->reg[reg].u32 >> cl) | (cpu->reg[rm].u32 << (32 - cl));
        compiler.lsr(R32(tmp->hardwareReg()), R32(reg->hardwareReg()), R32(cl->hardwareReg()));
        RegPtr tmp1 = loadConst(32);
        compiler.sub(R32(tmp1->hardwareReg()), R32(tmp1->hardwareReg()), R32(cl->hardwareReg()));
        compiler.lsl(R32(reg->hardwareReg()), R32(rm->hardwareReg()), R32(tmp1->hardwareReg()));
        compiler.orr(R32(reg->hardwareReg()), R32(reg->hardwareReg()), R32(tmp->hardwareReg()));
    } else if (regWidth == JitWidth::b16) {
        RegPtr tmp = getTmpReg();
        RegPtr reg16 = getTmpReg();
        RegPtr rm16 = getTmpReg();

        movzx(JitWidth::b32, reg16, JitWidth::b16, reg);
        movzx(JitWidth::b32, rm16, JitWidth::b16, rm);
        IfGreaterThanOrEqual(JitWidth::b32, cl, 0x10); {
            subValue(JitWidth::b32, cl, 16);
            mov(JitWidth::b32, reg16, rm16);
        } EndIf();
        compiler.lsr(R32(tmp->hardwareReg()), R32(reg16->hardwareReg()), R32(cl->hardwareReg()));
        RegPtr tmp1 = loadConst(16);
        compiler.sub(R32(tmp1->hardwareReg()), R32(tmp1->hardwareReg()), R32(cl->hardwareReg()));
        compiler.lsl(R32(reg16->hardwareReg()), R32(rm16->hardwareReg()), R32(tmp1->hardwareReg()));
        compiler.orr(R32(reg16->hardwareReg()), R32(reg16->hardwareReg()), R32(tmp->hardwareReg()));
        compiler.bfxil(R32(reg->hardwareReg()), R32(reg16->hardwareReg()), 0, 16);
    } else {
        kpanic("JitArmV8CodeGen::shrdReg");
    }
}

void JitArmV8CodeGen::shrdValue(JitWidth regWidth, RegPtr reg, RegPtr rm, U32 imm) {
    if (regWidth == JitWidth::b32) {
        // cpu->result.u32 = (cpu->reg[reg].u32 >> imm) | (cpu->reg[rm].u32 << (32 - imm));
        RegPtr tmp = getTmpReg();
        compiler.lsr(R32(tmp->hardwareReg()), R32(reg->hardwareReg()), imm);
        compiler.lsl(R32(reg->hardwareReg()), R32(rm->hardwareReg()), 32 - imm);
        compiler.orr(R32(reg->hardwareReg()), R32(reg->hardwareReg()), R32(tmp->hardwareReg()));
    } else if (regWidth == JitWidth::b16) {
        RegPtr tmp = getTmpReg();
        if (imm >= 16) {
            imm -= 16;
            mov(JitWidth::b16, reg, rm);
        }
        if (imm != 0) {
            compiler.lsr(R32(tmp->hardwareReg()), R32(reg->hardwareReg()), imm);
            compiler.bfi(R32(tmp->hardwareReg()), R32(rm->hardwareReg()), 16 - imm, imm);
            compiler.bfxil(R32(reg->hardwareReg()), R32(tmp->hardwareReg()), 0, 16);
        }
    } else {
        kpanic("JitArmV8CodeGen::shrdValue");
    }
}

void JitArmV8CodeGen::xchgReg(JitWidth regWidth, RegPtr dest, RegPtr src) {
    RegPtr tmp = getTmpReg();
    mov(regWidth, tmp, dest);
    mov(regWidth, dest, src);
    mov(regWidth, src, tmp);
}

void JitArmV8CodeGen::byteSwapReg32(RegPtr reg) {
    compiler.rev32(R32(reg->hardwareReg()), R32(reg->hardwareReg()));
}

RegPtr JitArmV8CodeGen::compare(JitEvaluate condition, RegPtr result) {
    if (!result) {
        result = getTmpReg();
    }
    switch (condition) {
    case JitEvaluate::EQUALS:
        compiler.csinc(R32(result->hardwareReg()), asmjit::a64::wzr, asmjit::a64::wzr, asmjit::a64::CondCode::kNE);
        break;
    case JitEvaluate::NOT_EQUALS:
        compiler.csinc(R32(result->hardwareReg()), asmjit::a64::wzr, asmjit::a64::wzr, asmjit::a64::CondCode::kEQ);
        break;
    case JitEvaluate::GREATER_THAN_UNSIGNED:
        compiler.csinc(R32(result->hardwareReg()), asmjit::a64::wzr, asmjit::a64::wzr, asmjit::a64::CondCode::kLS);
        break;
    case JitEvaluate::GREATER_THAN_EQUAL_UNSIGNED:
        compiler.csinc(R32(result->hardwareReg()), asmjit::a64::wzr, asmjit::a64::wzr, asmjit::a64::CondCode::kLO);
        break;
    case JitEvaluate::LESS_THAN_UNSIGNED:
        compiler.csinc(R32(result->hardwareReg()), asmjit::a64::wzr, asmjit::a64::wzr, asmjit::a64::CondCode::kHS);
        break;
    case JitEvaluate::LESS_THAN_EQUAL_UNSIGNED:
        compiler.csinc(R32(result->hardwareReg()), asmjit::a64::wzr, asmjit::a64::wzr, asmjit::a64::CondCode::kHI);
        break;
    case JitEvaluate::GREATER_THAN_SIGNED:
        compiler.csinc(R32(result->hardwareReg()), asmjit::a64::wzr, asmjit::a64::wzr, asmjit::a64::CondCode::kLE);
        break;
    case JitEvaluate::GREATER_THAN_EQUAL_SIGNED:
        compiler.csinc(R32(result->hardwareReg()), asmjit::a64::wzr, asmjit::a64::wzr, asmjit::a64::CondCode::kLT);
        break;
    case JitEvaluate::LESS_THAN_SIGNED:
        compiler.csinc(R32(result->hardwareReg()), asmjit::a64::wzr, asmjit::a64::wzr, asmjit::a64::CondCode::kGE);
        break;
    case JitEvaluate::LESS_THAN_EQUAL_SIGNED:
        compiler.csinc(R32(result->hardwareReg()), asmjit::a64::wzr, asmjit::a64::wzr, asmjit::a64::CondCode::kGT);
        break;
    default:
        kpanic_fmt("JitArmV8CodeGen::compareReg unknown condition %d", condition);
    }
    return result;
}

RegPtr JitArmV8CodeGen::compareReg(JitWidth regWidth, RegPtr reg1, RegPtr reg2, JitEvaluate condition, RegPtr result) {
    cmp(regWidth, reg1, reg2);
    return compare(condition, result);
}

RegPtr JitArmV8CodeGen::compareValue(JitWidth regWidth, RegPtr reg, U32 value, JitEvaluate condition, RegPtr result) {
    cmp(regWidth, reg, value);
    return compare(condition, result);
}

RegPtr JitArmV8CodeGen::testZeroReg(JitWidth regWidth, RegPtr reg, RegPtr result) {
    kpanic("JitArmV8CodeGen::testZeroReg");
    /*
    if (regWidth == JitWidth::b32) {
        x86.test(R32(reg->hardwareReg()), R32(reg->hardwareReg()));
    } else if (regWidth == JitWidth::b16) {
        x86.test(R16(reg->hardwareReg()), R16(reg->hardwareReg()));
    } else if (regWidth == JitWidth::b8) {
        x86.test(R8(get8bitReg(reg)), R8(get8bitReg(reg)));
    } else {
        kpanic_fmt("JitArmV8CodeGen::compareReg reg width %d", regWidth);
    }
    if (!result) {
        result = getTmpReg8();
    }
    x86.setz(R8(result->hardwareReg()));
    x86.movzx(R32(result->hardwareReg()), R8(get8bitReg(result)));
    */
    return result;
}

void JitArmV8CodeGen::mulReg(JitWidth regWidth, RegPtr reg) {
    if (regWidth == JitWidth::b32) {
        // EDX:EAX = (U64)EAX * src;
        compiler.mul(R64(0), R64(0), R64(reg->hardwareReg()));
        compiler.ubfx(R64(2), R64(0), 32, 32);
        compiler.mov(R32(0), R32(0)); // clear top 32-bits
    } else if (regWidth == JitWidth::b16) {
        // DX:AX = AX * src;
        RegPtr tmp = getTmpReg();
        RegPtr tmp2 = getTmpReg();
        compiler.ubfx(R32(tmp->hardwareReg()), R32(0), 0, 16);
        compiler.ubfx(R32(tmp2->hardwareReg()), R32(reg->hardwareReg()), 0, 16);
        compiler.mul(R32(tmp->hardwareReg()), R32(tmp->hardwareReg()), R32(tmp2->hardwareReg()));
        compiler.bfxil(R32(2), R32(tmp->hardwareReg()), 16, 16);
        compiler.bfxil(R32(0), R32(tmp->hardwareReg()), 0, 16);
    } else if (regWidth == JitWidth::b8) {
        // AX = AL * src;
        RegPtr tmp = getTmpReg();
        RegPtr tmp2 = getTmpReg();
        compiler.ubfx(R32(tmp->hardwareReg()), R32(0), 0, 8);
        compiler.ubfx(R32(tmp2->hardwareReg()), R32(reg->hardwareReg()), reg->isHigh ? 8 : 0, 8);
        compiler.mul(R32(tmp->hardwareReg()), R32(tmp->hardwareReg()), R32(tmp2->hardwareReg()));
        compiler.bfxil(R32(0), R32(tmp->hardwareReg()), 0, 16);
    } else {
        kpanic("JitArmV8CodeGen::mulReg");
    }
}

void JitArmV8CodeGen::imulRR(JitWidth regWidth, RegPtr dst, RegPtr src, RegPtr overflow) {
    if (regWidth == JitWidth::b32) {
        compiler.smull(R64(dst->hardwareReg()), R32(dst->hardwareReg()), R32(src->hardwareReg()));

        if (overflow) {
            compiler.lsr(R64(overflow->hardwareReg()), R64(dst->hardwareReg()), 32);
        }
        compiler.mov(R32(dst->hardwareReg()), R32(dst->hardwareReg())); // clear out top 32-bits
    } else if (regWidth == JitWidth::b16) {
        if (overflow) {
            kpanic("JitArmV8CodeGen::imulRR overflow");
        }
        RegPtr dst16 = getTmpReg();
        RegPtr src16 = getTmpReg();

        movsx(JitWidth::b32, dst16, JitWidth::b16, dst);
        movsx(JitWidth::b32, src16, JitWidth::b16, src);
        compiler.smull(R64(dst16->hardwareReg()), R32(dst16->hardwareReg()), R32(src16->hardwareReg()));
        compiler.bfxil(R32(dst->hardwareReg()), R32(dst16->hardwareReg()), 0, 16);
    } else {
        kpanic("JitArmV8CodeGen::imulRR");
    }
}

void JitArmV8CodeGen::imulRRI(JitWidth regWidth, RegPtr dst, RegPtr src, U32 src2, RegPtr overflow) {    
    RegPtr value = loadConst(src2);
    if (regWidth == JitWidth::b32) {
        RegPtr value = loadConst(src2);
        compiler.smull(R64(dst->hardwareReg()), R32(src->hardwareReg()), R32(value->hardwareReg()));
        if (overflow) {
            compiler.ubfx(R64(overflow->hardwareReg()), R64(dst->hardwareReg()), 32, 32);
        }
    } else if (regWidth == JitWidth::b16) {
        RegPtr value = loadConst((S32)((S16)src2));
        RegPtr src16 = getTmpReg();
        RegPtr dst16 = getTmpReg();
        movsx(JitWidth::b32, src16, JitWidth::b16, src);
        compiler.smull(R64(dst16->hardwareReg()), R32(src16->hardwareReg()), R32(value->hardwareReg()));
        mov(JitWidth::b16, dst, dst16);
        if (overflow) {
            compiler.ubfx(R32(overflow->hardwareReg()), R32(dst16->hardwareReg()), 16, 16);
        }
    } else {
        kpanic("JitArmV8CodeGen::imulRRI");
    }
}

void JitArmV8CodeGen::imulReg(JitWidth regWidth, RegPtr reg) {
    if (regWidth == JitWidth::b32) {
        // EDX:EAX = (S64)EAX * src;
        compiler.smull(R64(0), R32(0), R32(reg->hardwareReg()));
        compiler.ubfx(R64(2), R64(0), 32, 32);
        compiler.mov(R32(0), R32(0)); // clear top 32-bits
    } else if (regWidth == JitWidth::b16) {
        // DX:AX = AX * src;
        RegPtr tmp = getTmpReg();
        RegPtr tmp2 = getTmpReg();
        compiler.sxth(R32(tmp->hardwareReg()), R32(0));
        compiler.sxth(R32(tmp2->hardwareReg()), R32(reg->hardwareReg()));
        compiler.smull(R64(tmp->hardwareReg()), R32(tmp->hardwareReg()), R32(tmp2->hardwareReg()));
        compiler.bfxil(R32(2), R32(tmp->hardwareReg()), 16, 16);
        compiler.bfxil(R32(0), R32(tmp->hardwareReg()), 0, 16);
    } else if (regWidth == JitWidth::b8) {
        // AX = AL * src;
        RegPtr tmp = getTmpReg();
        RegPtr tmp2 = getTmpReg();
        compiler.sxtb(R32(tmp->hardwareReg()), R32(0));
        compiler.sbfx(R32(tmp2->hardwareReg()), R32(reg->hardwareReg()), reg->isHigh ? 8 : 0, 8);
        compiler.smull(R64(tmp->hardwareReg()), R32(tmp->hardwareReg()), R32(tmp2->hardwareReg()));
        compiler.bfxil(R32(0), R32(tmp->hardwareReg()), 0, 16);
    } else {
        kpanic("JitArmV8CodeGen::imulReg");
    }
}

void JitArmV8CodeGen::absReg(JitWidth regWidth, RegPtr reg) {
    RegPtr tmp = getTmpReg();

    if (regWidth == JitWidth::b16) {
        movsx(JitWidth::b32, reg, JitWidth::b16, reg);
    } else if (regWidth == JitWidth::b8) {
        movsx(JitWidth::b32, reg, JitWidth::b8, reg);
    } else if (regWidth == JitWidth::b64) {
        compiler.add(R64(tmp->hardwareReg()), R64(reg->hardwareReg()), R64(reg->hardwareReg()), asmjit::Imm(asmjit::a64::Shift(asmjit::a64::ShiftOp::kASR, 63)));
        compiler.eor(R64(reg->hardwareReg()), R64(tmp->hardwareReg()), R64(reg->hardwareReg()), asmjit::Imm(asmjit::a64::Shift(asmjit::a64::ShiftOp::kASR, 63)));
        return;
    } else if (regWidth != JitWidth::b32) {
        kpanic("JitArmV8CodeGen::absReg");
    }

    compiler.add(R32(tmp->hardwareReg()), R32(reg->hardwareReg()), R32(reg->hardwareReg()), asmjit::Imm(asmjit::a64::Shift(asmjit::a64::ShiftOp::kASR, 31)));
    compiler.eor(R32(reg->hardwareReg()), R32(tmp->hardwareReg()), R32(reg->hardwareReg()), asmjit::Imm(asmjit::a64::Shift(asmjit::a64::ShiftOp::kASR, 31)));
}

void JitArmV8CodeGen::clzReg(JitWidth regWidth, RegPtr result, RegPtr reg) {
   if (regWidth == JitWidth::b32) {
        compiler.clz(R32(result->hardwareReg()), R32(reg->hardwareReg()));
#ifdef BOXEDWINE_64
    } else if (regWidth == JitWidth::b64) {
        compiler.clz(R64(result->hardwareReg()), R64(reg->hardwareReg()));
#endif
    } else {
        kpanic("JitArmV8CodeGen::clzReg");
    }
}

void JitArmV8CodeGen::divRegRegWithRemainder(JitWidth regWidth, RegPtr dest, RegPtr destHighAndRemainder, RegPtr src) {
    if (regWidth == JitWidth::b32) {
        // U64 num = ((U64)EDX << 32) | EAX;
        RegPtr tmp = getTmpReg();
        compiler.orr(R64(tmp->hardwareReg()), R64(dest->hardwareReg()), R64(destHighAndRemainder->hardwareReg()), asmjit::Imm(asmjit::a64::Shift(asmjit::a64::ShiftOp::kLSL, 32)));
        compiler.udiv(R64(dest->hardwareReg()), R64(tmp->hardwareReg()), R64(src->hardwareReg()));
        compiler.msub(R64(destHighAndRemainder->hardwareReg()), R64(dest->hardwareReg()), R64(src->hardwareReg()), R64(tmp->hardwareReg()));
        compiler.mov(R32(dest->hardwareReg()), R32(dest->hardwareReg())); // clear top 32-bits
    } else if (regWidth == JitWidth::b16) {
        // U32 num = ((U32)DX << 16) | AX;
        RegPtr tmp = getTmpReg();
        RegPtr result = getTmpReg();
        RegPtr src16 = getTmpReg();

        compiler.ubfx(R32(src16->hardwareReg()), R32(src->hardwareReg()), 0, 16);
        compiler.ubfx(R32(tmp->hardwareReg()), R32(dest->hardwareReg()), 0, 16);
        compiler.orr(R32(tmp->hardwareReg()), R32(tmp->hardwareReg()), R32(destHighAndRemainder->hardwareReg()), asmjit::Imm(asmjit::a64::Shift(asmjit::a64::ShiftOp::kLSL, 16)));
        compiler.udiv(R32(result->hardwareReg()), R32(tmp->hardwareReg()), R32(src16->hardwareReg()));
        compiler.bfxil(R32(dest->hardwareReg()), R32(result->hardwareReg()), 0, 16);
        compiler.msub(R32(tmp->hardwareReg()), R32(result->hardwareReg()), R32(src16->hardwareReg()), R32(tmp->hardwareReg()));
        compiler.bfxil(R32(destHighAndRemainder->hardwareReg()), R32(tmp->hardwareReg()), 0, 16);
    } else if (regWidth == JitWidth::b8) {
        // quo = AX / src;
        RegPtr tmp = getTmpReg();
        RegPtr result = getTmpReg();
        RegPtr src8 = getTmpReg();

        compiler.ubfx(R32(src8->hardwareReg()), R32(src->hardwareReg()), src->isHigh ? 8 : 0, 8);
        compiler.ubfx(R32(tmp->hardwareReg()), R32(dest->hardwareReg()), 0, 16);
        compiler.udiv(R32(result->hardwareReg()), R32(tmp->hardwareReg()), R32(src8->hardwareReg()));
        compiler.bfxil(R32(dest->hardwareReg()), R32(result->hardwareReg()), 0, 8);
        compiler.msub(R32(tmp->hardwareReg()), R32(result->hardwareReg()), R32(src8->hardwareReg()), R32(tmp->hardwareReg()));
        compiler.bfi(R32(dest->hardwareReg()), R32(tmp->hardwareReg()), 8, 8);
    } else {
        kpanic("JitArmV8CodeGen::divRegRegWithRemainder");
    }
}

void JitArmV8CodeGen::idivRegRegWithRemainder(JitWidth regWidth, RegPtr dest, RegPtr destHighAndRemainder, RegPtr src) {
    if (regWidth == JitWidth::b32) {
        // S64 num = ((S64)EDX << 32) | EAX;
        RegPtr tmp = getTmpReg();
        RegPtr src32 = getTmpReg();

        compiler.sbfx(R64(src32->hardwareReg()), R64(src->hardwareReg()), 0, 32);
        compiler.orr(R64(tmp->hardwareReg()), R64(dest->hardwareReg()), R64(destHighAndRemainder->hardwareReg()), asmjit::Imm(asmjit::a64::Shift(asmjit::a64::ShiftOp::kLSL, 32)));
        compiler.sdiv(R64(dest->hardwareReg()), R64(tmp->hardwareReg()), R64(src32->hardwareReg()));
        compiler.msub(R64(destHighAndRemainder->hardwareReg()), R64(dest->hardwareReg()), R64(src32->hardwareReg()), R64(tmp->hardwareReg()));
        compiler.mov(R32(dest->hardwareReg()), R32(dest->hardwareReg())); // clear top 32-bits
    } else if (regWidth == JitWidth::b16) {
        // S32 num = ((S32)DX << 16) | AX;
        RegPtr tmp = getTmpReg();
        RegPtr result = getTmpReg();
        RegPtr src16 = getTmpReg();

        compiler.sbfx(R32(src16->hardwareReg()), R32(src->hardwareReg()), 0, 16);
        compiler.ubfx(R32(tmp->hardwareReg()), R32(dest->hardwareReg()), 0, 16);
        compiler.orr(R32(tmp->hardwareReg()), R32(tmp->hardwareReg()), R32(destHighAndRemainder->hardwareReg()), asmjit::Imm(asmjit::a64::Shift(asmjit::a64::ShiftOp::kLSL, 16)));
        compiler.sdiv(R32(result->hardwareReg()), R32(tmp->hardwareReg()), R32(src16->hardwareReg()));
        compiler.bfxil(R32(dest->hardwareReg()), R32(result->hardwareReg()), 0, 16);
        compiler.msub(R32(tmp->hardwareReg()), R32(result->hardwareReg()), R32(src16->hardwareReg()), R32(tmp->hardwareReg()));
        compiler.bfxil(R32(destHighAndRemainder->hardwareReg()), R32(tmp->hardwareReg()), 0, 16);
    } else if (regWidth == JitWidth::b8) {
        // quo = AX / src;
        RegPtr tmp = getTmpReg();
        RegPtr result = getTmpReg();
        RegPtr src8 = getTmpReg();

        compiler.sbfx(R32(src8->hardwareReg()), R32(src->hardwareReg()), src->isHigh ? 8 : 0, 8);
        compiler.sbfx(R32(tmp->hardwareReg()), R32(dest->hardwareReg()), 0, 16);
        compiler.sdiv(R32(result->hardwareReg()), R32(tmp->hardwareReg()), R32(src8->hardwareReg()));
        compiler.bfxil(R32(dest->hardwareReg()), R32(result->hardwareReg()), 0, 8);
        compiler.msub(R32(tmp->hardwareReg()), R32(result->hardwareReg()), R32(src8->hardwareReg()), R32(tmp->hardwareReg()));
        compiler.bfi(R32(dest->hardwareReg()), R32(tmp->hardwareReg()), 8, 8);
    } else {
        kpanic("JitArmV8CodeGen::idivRegRegWithRemainder");
    }
}

void JitArmV8CodeGen::readRamPage(RegPtr dest, RegPtr index) {
    // :TODO: remove
}

#include "../../softmmu/kmemory_soft.h"
void JitArmV8CodeGen::readMMU(RegPtr dest, RegPtr index) {
    compiler.ldr(R64(dest->hardwareReg()), Mem(xMMU, R64(index->hardwareReg()), Shift(ShiftOp::kLSL, 3)));
}

void JitArmV8CodeGen::readMMU(RegPtr dest, U32 index) {
    compiler.ldr(R64(dest->hardwareReg()), createMem(regMMU, index * 8));
}

Mem JitArmV8CodeGen::createMem(RegPtr reg, RegPtr sib, U8 lsl, U32 disp) {
    return createMem(reg->hardwareReg(), sib->hardwareReg(), lsl, disp);
}

Mem JitArmV8CodeGen::createMem(U8 reg, U8 sib, U8 lsl, U32 disp) {
    if (lsl) {
        if (disp) {
            compiler.add(xMemTmp, R64(reg), disp);
            return Mem(xMemTmp, R64(sib), Shift(ShiftOp::kLSL, lsl));
        } else {
            return Mem(R64(reg), R64(sib), Shift(ShiftOp::kLSL, lsl));
        }
    } else {
        if (disp) {
            compiler.add(xMemTmp, R64(sib), disp);
            return Mem(R64(reg), xMemTmp);
        } else {
            return Mem(R64(reg), R64(sib));
        }
    }
}

Mem JitArmV8CodeGen::createMem(RegPtr reg, U32 disp) {
    return createMem(reg->hardwareReg(), disp);
}

Mem JitArmV8CodeGen::createMem(U8 reg, U32 disp) {
    if (disp <= 0xfff && (disp & 3) == 0) {
        return Mem(R64(reg), disp);
    }
    RegPtr dispReg = loadConst(disp);
    return Mem(R64(reg), R64(dispReg->hardwareReg()));
}

RegPtr JitArmV8CodeGen::loadConst(U64 value) {
    RegPtr reg = getTmpReg();
    compiler.mov(R64(reg->hardwareReg()), value);
    return reg;
}

MMXRegPtr JitArmV8CodeGen::loadMMXConst(U8 index) {
    MMXRegPtr result = getTmpMMX();
    compiler.ldr(asmjit::a64::Vec::make_v128(result->hardwareReg()), createMem(regCPU, offsetof(CPU, sseConstants) + index * 16));
    return result;
}

void JitArmV8CodeGen::read(JitWidth width, RegPtr dest, RegPtr reg, U32 disp) {
    if (!isTmp[dest->hardwareReg()]) {
        kpanic("JitArmV8CodeGen::read");
    }
    if (width == JitWidth::b32) {
        compiler.ldr(R32(dest->hardwareReg()), createMem(reg, disp));
    } else if (width == JitWidth::b16) {
        compiler.ldrh(R32(dest->hardwareReg()), createMem(reg, disp));
    } else if (width == JitWidth::b8) {
        if (!isRegHigh(dest)) {
            compiler.ldrb(R32(dest->hardwareReg()), createMem(reg, disp));
        } else {
            RegPtr tmp = getTmpReg();
            compiler.ldrb(R32(tmp->hardwareReg()), createMem(reg, disp));
            compiler.bfi(R32(reg->hardwareReg()), R32(tmp->hardwareReg()), 8, 8);
        }
#ifdef BOXEDWINE_64
    } else if (width == JitWidth::b64) {
        compiler.ldr(R64(dest->hardwareReg()), createMem(reg, disp));
#endif
    } else {
        kpanic_fmt("JitArmV8CodeGen::readMem unexpected width: %d", (U32)width);
    }
}

void JitArmV8CodeGen::read(JitWidth width, RegPtr dest, RegPtr reg, RegPtr sib, U8 lsl, U32 disp) {
    // arm zero extends reads
    if (!isTmp[dest->hardwareReg()] && width == JitWidth::b8 || width == JitWidth::b16) {
        RegPtr tmp = getTmpReg();
        read(JitWidth::b32, tmp, reg, sib, lsl, disp);
        mov(width, dest, tmp);
        return;
    }
    if (width == JitWidth::b32) {
        compiler.ldr(R32(dest->hardwareReg()), createMem(reg, sib, lsl, disp));
    } else if (width == JitWidth::b16) {
        compiler.ldrh(R32(dest->hardwareReg()), createMem(reg, sib, lsl, disp));
    } else if (width == JitWidth::b8) {
        if (!isRegHigh(dest)) {
            compiler.ldrb(R32(dest->hardwareReg()), createMem(reg, sib, lsl, disp));
        } else {
            RegPtr tmp = getTmpReg();
            compiler.ldrb(R32(tmp->hardwareReg()), createMem(reg, sib, lsl, disp));
            compiler.bfi(R32(reg->hardwareReg()), R32(tmp->hardwareReg()), 8, 8);
        }
#ifdef BOXEDWINE_64
    } else if (width == JitWidth::b64) {
        compiler.ldr(R64(dest->hardwareReg()), createMem(reg, sib, lsl, disp));
#endif
    } else {
        kpanic_fmt("JitArmV8CodeGen::readMem unexpected width: %d", (U32)width);
    }
}

void JitArmV8CodeGen::write(JitWidth width, RegPtr reg, U32 disp, RegPtr src) {
    if(width == JitWidth::b32) {
        compiler.str(R32(src->hardwareReg()), createMem(reg->hardwareReg(), disp));
    } else if (width == JitWidth::b16) {
        compiler.strh(R32(src->hardwareReg()), createMem(reg->hardwareReg(), disp));
    } else if (width == JitWidth::b8) {
        compiler.strb(R32(getReg8InLowByte(src)->hardwareReg()), createMem(reg->hardwareReg(), disp));
#ifdef BOXEDWINE_64
    }
    else if (width == JitWidth::b64) {
        compiler.str(R64(src->hardwareReg()), createMem(reg->hardwareReg(), disp));
#endif
    } else {
        kpanic_fmt("JitArmV8CodeGen::write unexpected width: %d", (U32)width);
    }
}

void JitArmV8CodeGen::write(JitWidth width, RegPtr reg, RegPtr sib, U8 lsl, U32 disp, U32 value) {
    if (width == JitWidth::b32) {
        RegPtr tmp = loadConst(value);
        compiler.str(R32(tmp->hardwareReg()), createMem(reg->hardwareReg(), sib->hardwareReg(), lsl, disp));
    } else if (width == JitWidth::b16) {
        RegPtr tmp = loadConst(value & 0xffff);
        compiler.strh(R32(tmp->hardwareReg()), createMem(reg->hardwareReg(), sib->hardwareReg(), lsl, disp));
    } else if (width == JitWidth::b8) {
        RegPtr tmp = loadConst(value & 0xff);
        compiler.strb(R32(tmp->hardwareReg()), createMem(reg->hardwareReg(), sib->hardwareReg(), lsl, disp));
    } else {
        kpanic_fmt("JitArmV8CodeGen::write unexpected width: %d", (U32)width);
    }
}

void JitArmV8CodeGen::write(JitWidth width, RegPtr reg, RegPtr sib, U8 lsl, U32 disp, RegPtr src) {
    if (width == JitWidth::b32) {
        compiler.str(R32(src->hardwareReg()), createMem(reg->hardwareReg(), sib->hardwareReg(), lsl, disp));
    } else if (width == JitWidth::b16) {
        compiler.strh(R32(src->hardwareReg()), createMem(reg->hardwareReg(), sib->hardwareReg(), lsl, disp));
    } else if (width == JitWidth::b8) {
        compiler.strb(R32(getReg8InLowByte(src)->hardwareReg()), createMem(reg->hardwareReg(), sib->hardwareReg(), lsl, disp));
#ifdef BOXEDWINE_64
    } else if (width == JitWidth::b64) {
        compiler.str(R64(src->hardwareReg()), createMem(reg->hardwareReg(), sib->hardwareReg(), lsl, disp));
#endif
    } else {
        kpanic_fmt("JitArmV8CodeGen::write unexpected width: %d", (U32)width);
    }
}

RegPtr JitArmV8CodeGen::readCPU(JitWidth width, U32 offset, RegPtr reg) {
    if (!reg) {
        reg = getTmpReg();
    }
    if (!isTmp[reg->hardwareReg()]) {
        kpanic("JitArmV8CodeGen::readCPU");
    }
    // mov reg, [edi+srcOffset]    
    if (width == JitWidth::b32) {
        compiler.ldr(R32(reg->hardwareReg()), Mem(xCPU, offset));
    } else if (width == JitWidth::b16) {
        compiler.ldrh(R32(reg->hardwareReg()), Mem(xCPU, offset));
    } else if (width == JitWidth::b8) {
        compiler.ldrb(R32(reg->hardwareReg()), Mem(xCPU, offset));
#ifdef BOXEDWINE_64
    } else if (width == JitWidth::b64) {
        compiler.ldr(R64(reg->hardwareReg()), Mem(xCPU, offset));
#endif
    } else {
        kpanic_fmt("unknown dstWidth in JitArmV8CodeGen::readCPU %d", width);
    }
    return reg;
}

RegPtr JitArmV8CodeGen::readCPU(JitWidth width, RegPtr sib, U8 lsl, U32 offset, RegPtr reg) {
    if (!reg) {
        reg = getTmpReg();
    }
    if (!isTmp[reg->hardwareReg()]) {
        kpanic("JitArmV8CodeGen::readCPU");
    }
    if (width == JitWidth::b32) {
        compiler.ldr(R32(reg->hardwareReg()), createMem(regCPU, sib->hardwareReg(), lsl, offset));
    } else if (width == JitWidth::b16) {
        compiler.ldrh(R32(reg->hardwareReg()), createMem(regCPU, sib->hardwareReg(), lsl, offset));
    } else if (width == JitWidth::b8) {
        compiler.ldrb(R32(reg->hardwareReg()), createMem(regCPU, sib->hardwareReg(), lsl, offset));
#ifdef BOXEDWINE_64
    } else if (width == JitWidth::b64) {
        compiler.ldr(R64(reg->hardwareReg()), createMem(regCPU, sib->hardwareReg(), lsl, offset));
#endif
    } else {
        kpanic_fmt("unknown dstWidth in JitArmV8CodeGen::readCPU %d", width);
    }
    return reg;
}

void JitArmV8CodeGen::writeCPUValue(JitWidth width, RegPtr sib, U8 lsl, U32 offset, DYN_PTR_SIZE src) {
    if (width == JitWidth::b32) {
        RegPtr reg = loadConst(src & 0xffffffff);
        compiler.str(R32(reg->hardwareReg()), createMem(regCPU, sib->hardwareReg(), lsl, offset));
    } else if (width == JitWidth::b16) {
        RegPtr reg = loadConst(src & 0xffff);
        compiler.strh(R32(reg->hardwareReg()), createMem(regCPU, sib->hardwareReg(), lsl, offset));
    } else if (width == JitWidth::b8) {
        RegPtr reg = loadConst(src & 0xff);
        compiler.strb(R32(reg->hardwareReg()), createMem(regCPU, sib->hardwareReg(), lsl, offset));
#ifdef BOXEDWINE_64
    } else if (width == JitWidth::b64) {
        RegPtr reg = loadConst(src);
        compiler.str(R64(reg->hardwareReg()), createMem(regCPU, sib->hardwareReg(), lsl, offset));
#endif
    } else {
        kpanic_fmt("unknown dstWidth in JitArmV8CodeGen::writeCPU %d", width);
    }
}

void JitArmV8CodeGen::writeCPUValue(JitWidth width, U32 offset, DYN_PTR_SIZE src) {
    if (width == JitWidth::b32) {
        RegPtr reg = loadConst(src & 0xffffffff);
        compiler.str(R32(reg->hardwareReg()), createMem(regCPU, offset));
    } else if (width == JitWidth::b16) {
        RegPtr reg = loadConst(src & 0xffff);
        compiler.strh(R32(reg->hardwareReg()), createMem(regCPU, offset));
    } else if (width == JitWidth::b8) {
        RegPtr reg = loadConst(src & 0xff);
        compiler.strb(R32(reg->hardwareReg()), createMem(regCPU, offset));
#ifdef BOXEDWINE_64
    } else if (width == JitWidth::b64) {
        RegPtr reg = loadConst(src);
        compiler.str(R64(reg->hardwareReg()), createMem(regCPU, offset));
#endif
    } else {
        kpanic_fmt("unknown dstWidth in JitArmV8CodeGen::writeCPU %d", width);
    }
}

void JitArmV8CodeGen::mov(JitWidth regWidth, RegPtr dest, RegPtr src) {
    if (regWidth == JitWidth::b32) {
        compiler.mov(R32(dest->hardwareReg()), R32(src->hardwareReg()));
    } else if (regWidth == JitWidth::b16) {
        compiler.bfxil(R32(dest->hardwareReg()), R32(src->hardwareReg()), 0, 16);
    } else if (regWidth == JitWidth::b8) {
        if (isRegHigh(dest) && isRegHigh(src)) {
            RegPtr tmp = getTmpReg();
            mov(regWidth, tmp, src);
            mov(regWidth, dest, tmp);
        } else if (isRegHigh(dest)) {
            compiler.bfi(R32(dest->hardwareReg()), R32(src->hardwareReg()), 8, 8);
        } else if (isRegHigh(src)) {
            compiler.bfxil(R32(dest->hardwareReg()), R32(src->hardwareReg()), 8, 8);
        } else {
            compiler.bfxil(R32(dest->hardwareReg()), R32(src->hardwareReg()), 0, 8);
        }        
#ifdef BOXEDWINE_64
    } else if (regWidth == JitWidth::b64) {
        compiler.mov(R64(dest->hardwareReg()), R64(src->hardwareReg()));
#endif
    } else {
        kpanic_fmt("JitArmV8CodeGen::mov unexpected width: %d", (U32)regWidth);
    }
}

void JitArmV8CodeGen::movValue(JitWidth regWidth, RegPtr dst, DYN_PTR_SIZE imm) {
    if (regWidth == JitWidth::b32) {
        compiler.mov(R32(dst->hardwareReg()), (U32)imm);
    } else if (regWidth == JitWidth::b16) {
        compiler.movk(R32(dst->hardwareReg()), (U16)imm, 0);
    } else if (regWidth == JitWidth::b8) {
        mov(regWidth, dst, loadConst(imm));
#ifdef BOXEDWINE_64
    } else if (regWidth == JitWidth::b64) {
        compiler.mov(R64(dst->hardwareReg()), imm);
#endif
    } else {
        kpanic_fmt("JitArmV8CodeGen::mov unexpected width: %d", (U32)regWidth);
    }
}

void JitArmV8CodeGen::movzx(JitWidth dstWidth, RegPtr dest, JitWidth srcWidth, RegPtr src) {
    if (dstWidth == JitWidth::b32) {
        if (srcWidth == JitWidth::b16) {
            compiler.ubfx(R32(dest->hardwareReg()), R32(src->hardwareReg()), 0, 16);
        } else if (srcWidth == JitWidth::b8) {
            if (isRegHigh(src)) {
                compiler.ubfx(R32(dest->hardwareReg()), R32(src->hardwareReg()), 8, 8);
            } else {
                compiler.ubfx(R32(dest->hardwareReg()), R32(src->hardwareReg()), 0, 8);
            }
            dest->isHigh = false; // for when src == dest
        } else {
            kpanic_fmt("unknown width in JitArmV8CodeGen::movzx %d <= %d", dstWidth, srcWidth);
        }
    } else if (dstWidth == JitWidth::b16) {
        if (srcWidth == JitWidth::b8) {
            RegPtr tmp = getTmpReg();
            if (isRegHigh(src)) {
                compiler.ubfx(R32(tmp->hardwareReg()), R32(src->hardwareReg()), 8, 8);
            } else {
                compiler.ubfx(R32(tmp->hardwareReg()), R32(src->hardwareReg()), 0, 8);
            }
            compiler.bfxil(R32(dest->hardwareReg()), R32(tmp->hardwareReg()), 0, 16);
            dest->isHigh = false; // for when src == dest
        } else {
            kpanic_fmt("unknown width in JitArmV8CodeGen::movzx %d <= %d", dstWidth, srcWidth);
        }
    } else {
        kpanic_fmt("unknown width in JitArmV8CodeGen::movzx %d <= %d", dstWidth, srcWidth);
    }
}

void JitArmV8CodeGen::movsx(JitWidth dstWidth, RegPtr dest, JitWidth srcWidth, RegPtr src) {
    if (dstWidth == JitWidth::b32) {
        if (srcWidth == JitWidth::b16) {
            compiler.sxth(R32(dest->hardwareReg()), R32(src->hardwareReg()));
        } else if (srcWidth == JitWidth::b8) {
            if (isRegHigh(src)) {
                compiler.sbfx(R32(dest->hardwareReg()), R32(src->hardwareReg()), 8, 8);
            } else {
                compiler.sxtb(R32(dest->hardwareReg()), R32(src->hardwareReg()));
            }
            dest->isHigh = false; // for when src == dest
        } else {
            kpanic_fmt("unknown width in JitArmV8CodeGen::movsx %d <= %d", dstWidth, srcWidth);
        }
    } else if (dstWidth == JitWidth::b16) {
        if (srcWidth == JitWidth::b8) {
            RegPtr tmp = getTmpReg();
            if (isRegHigh(src)) {
                compiler.sbfx(R32(tmp->hardwareReg()), R32(src->hardwareReg()), 8, 8);
            } else {
                compiler.sxtb(R32(tmp->hardwareReg()), R32(src->hardwareReg()));
            }
            compiler.bfxil(R32(dest->hardwareReg()), R32(tmp->hardwareReg()), 0, 16);
            dest->isHigh = false; // for when src == dest
        } else {
            kpanic_fmt("unknown width in JitArmV8CodeGen::movsx %d <= %d", dstWidth, srcWidth);
        }
    } else {
        kpanic_fmt("unknown width in JitArmV8CodeGen::movsx %d <= %d", dstWidth, srcWidth);
    }
}

void JitArmV8CodeGen::pushParam(const DynParam& param, U32 index) {
    switch (param.type) {
    case JitCallParamType::REG_8:
        compiler.uxtb(R64(index), R64(param.reg->hardwareReg()));
        break;
    case JitCallParamType::REG_16:
        compiler.uxth(R64(index), R64(param.reg->hardwareReg()));
        break;
    case JitCallParamType::REG_32:
        compiler.mov(R64(index), R64(param.reg->hardwareReg()));
        break;
    case JitCallParamType::CPU:
        compiler.mov(R64(index), xCPU);
        break;
    case JitCallParamType::CONST_8:
        compiler.mov(R64(index), param.value & 0xFF);
        break;
    case JitCallParamType::CONST_16:
        compiler.mov(R64(index), param.value & 0xFFFF);
        break;
    case JitCallParamType::CONST_32:
        compiler.mov(R64(index), param.value & 0xFFFFFFFF);
        break;
    case JitCallParamType::CONST_PTR:
        compiler.mov(R64(index), param.value);
        break;
    default:
        kpanic_fmt("JitArmV8CodeGen::pushParam: unknown argType: %d", param.type);
        break;
    }
}

void JitArmV8CodeGen::setParams(const std::vector<DynParam>& params) {
    static_assert(MAX_NUMBER_OF_CALL_PARAMS == 2);
    if (params.size() == 2 && params[1].usesReg() && params[1].reg->hardwareReg() == 0) {
        pushParam(params[1], 1);
        pushParam(params[0], 0);
    } else {
        for (int i = 0; i < params.size(); i++) {
            pushParam(params[i], i);
        }
    }
}

void JitArmV8CodeGen::callHostFunction(void* address, const std::vector<DynParam>& params, bool restoreCache) {
    compiler.blr(xWriteCacheToCPU);

    setParams(params);    

    compiler.mov(xBranch, (U64)address);
    compiler.blr(xBranch);

    if (restoreCache) {
        compiler.blr(xLoadCacheFromCPU);
    }
}

void JitArmV8CodeGen::nakedCall(RegPtr reg) {
    compiler.blr(R64(reg->hardwareReg()));
}

void JitArmV8CodeGen::nakedReturn() {
    compiler.ret(asmjit::a64::x30);
}

void JitArmV8CodeGen::callHostFunctionWithResult(RegPtr result, void* address, const std::vector<DynParam>& params) {
    compiler.blr(xWriteCacheToCPU);

    setParams(params);

    compiler.mov(xBranch, (U64)address);
    compiler.blr(xBranch);
    compiler.mov(R64(result->hardwareReg()), R64(0));
    compiler.blr(xLoadCacheFromCPU);
}

void JitArmV8CodeGen::If(JitWidth regWidth, RegPtr reg) {
    Label label = compiler.new_label();
    ifLabels.push_back(label);

    if (regWidth == JitWidth::b32) {
        compiler.cbz(R32(reg->hardwareReg()), label);
    } else if (regWidth == JitWidth::b16) {
        RegPtr tmp = getTmpReg();
        compiler.ubfx(R32(tmp->hardwareReg()), R32(reg->hardwareReg()), 0, 16);
        compiler.cbz(R32(tmp->hardwareReg()), label);
    } else if (regWidth == JitWidth::b8) {
        RegPtr tmp = getTmpReg();
        if (isRegHigh(reg)) {
            compiler.ubfx(R32(tmp->hardwareReg()), R32(reg->hardwareReg()), 8, 8);
        } else {
            compiler.ubfx(R32(tmp->hardwareReg()), R32(reg->hardwareReg()), 0, 8);
        }
        compiler.cbz(R32(tmp->hardwareReg()), label);
#ifdef BOXEDWINE_64
    } else if (regWidth == JitWidth::b64) {
        compiler.cbz(R64(reg->hardwareReg()), label);
#endif
    } else {
        kpanic_fmt("JitArmV8CodeGen::If unexpected width: %d", (U32)regWidth);
    }
}

void JitArmV8CodeGen::IfTest(JitWidth regWidth, RegPtr reg, RegPtr mask) {
    if (regWidth == JitWidth::b32) {
        compiler.tst(R32(reg->hardwareReg()), R32(mask->hardwareReg()));
    } else if (regWidth == JitWidth::b16 || regWidth == JitWidth::b8) {
        RegPtr left = getTmpReg();
        RegPtr right = getTmpReg();
        movzx(JitWidth::b32, left, regWidth, reg);
        movzx(JitWidth::b32, right, regWidth, mask);
        compiler.tst(R32(left->hardwareReg()), R32(right->hardwareReg()));
    } else {
        kpanic("JitArmV8CodeGen::IfTest");
    }

    Label label = compiler.new_label();
    ifLabels.push_back(label);
    compiler.b_eq(label);
}

void JitArmV8CodeGen::IfTest(JitWidth regWidth, RegPtr reg, U32 value) {
    if (regWidth == JitWidth::b8 && reg->isHigh) {
        value <<= 8;
    }
    if (regWidth != JitWidth::b8 && regWidth != JitWidth::b16 && regWidth != JitWidth::b32) {
        kpanic("JitArmV8CodeGen::IfTest");
    }
    if (!asmjit::a64::Utils::is_logical_imm(value, 32)) {
        RegPtr imm = loadConst(value);
        compiler.tst(R32(reg->hardwareReg()), R32(imm->hardwareReg()));
    } else {
        compiler.tst(R32(reg->hardwareReg()), value);
    }
    Label label = compiler.new_label();
    ifLabels.push_back(label);
    compiler.b_eq(label);
}

void JitArmV8CodeGen::IfNotTestBit(JitWidth regWidth, RegPtr reg, U32 bitPos) {
    Label label = compiler.new_label();
    ifLabels.push_back(label);

    if (regWidth == JitWidth::b8 && isRegHigh(reg)) {
        kpanic("JitArmV8CodeGen::IfNotTest");
    }
    if (bitPos > 63) {
        kpanic("JitArmV8CodeGen::IfNotTest");
    }
    compiler.tbnz(R32(reg->hardwareReg()), bitPos, label);
}

void JitArmV8CodeGen::IfTestBit(JitWidth regWidth, RegPtr reg, U32 bitPos) {
    Label label = compiler.new_label();
    ifLabels.push_back(label);

    if (regWidth == JitWidth::b8 && isRegHigh(reg)) {
        kpanic("JitArmV8CodeGen::IfNotTest");
    }
    if (bitPos > 63) {
        kpanic("JitArmV8CodeGen::IfNotTest");
    }
    compiler.tbz(R32(reg->hardwareReg()), bitPos, label);
}

void JitArmV8CodeGen::cmp(JitWidth regWidth, RegPtr reg1, RegPtr reg2) {
    if (regWidth == JitWidth::b32) {
        compiler.cmp(R32(reg1->hardwareReg()), R32(reg2->hardwareReg()));
    }
    else if (regWidth == JitWidth::b16) {
        RegPtr left = getTmpReg();
        RegPtr right = getTmpReg();
        movsx(JitWidth::b32, left, JitWidth::b16, reg1);
        movsx(JitWidth::b32, right, JitWidth::b16, reg2);
        compiler.cmp(R32(left->hardwareReg()), R32(right->hardwareReg()));
    }
    else if (regWidth == JitWidth::b8) {
        RegPtr left = getTmpReg();
        RegPtr right = getTmpReg();
        movsx(JitWidth::b32, left, JitWidth::b8, reg1);
        movsx(JitWidth::b32, right, JitWidth::b8, reg2);
        compiler.cmp(R32(left->hardwareReg()), R32(right->hardwareReg()));
    }
    else {
        kpanic_fmt("JitArmV8CodeGen::cmp unexpected width: %d", (U32)regWidth);
    }
}

void JitArmV8CodeGen::cmp(JitWidth regWidth, RegPtr reg, DYN_PTR_SIZE value) {
    if (!asmjit::a64::Utils::is_add_sub_imm(value)) {
        cmp(regWidth, reg, loadConst(value));
        return;
    }
    if (regWidth == JitWidth::b32) {
        compiler.cmp(R32(reg->hardwareReg()), value);
    }
    else if (regWidth == JitWidth::b16) {
        RegPtr left = getTmpReg();
        movzx(JitWidth::b32, left, JitWidth::b16, reg);
        compiler.cmp(R32(left->hardwareReg()), value);
    }
    else if (regWidth == JitWidth::b8) {
        RegPtr left = getTmpReg();
        compiler.ubfx(R32(left->hardwareReg()), R32(reg->hardwareReg()), reg->isHigh ? 8 : 0, 8);
        compiler.cmp(R32(left->hardwareReg()), value);
    }
    else {
        kpanic_fmt("JitArmV8CodeGen::IfEqual unexpected width: %d", (U32)regWidth);
    }
}

void JitArmV8CodeGen::IfEqual(JitWidth regWidth, RegPtr reg1, RegPtr reg2) {
    cmp(regWidth, reg1, reg2);

    Label label = compiler.new_label();
    ifLabels.push_back(label);
    compiler.b_ne(label);
}

void JitArmV8CodeGen::IfEqual(JitWidth regWidth, RegPtr reg, DYN_PTR_SIZE value) {
    cmp(regWidth, reg, value);

    Label label = compiler.new_label();
    ifLabels.push_back(label);
    compiler.b_ne(label);
}

void JitArmV8CodeGen::IfNotEqual(JitWidth regWidth, RegPtr reg, DYN_PTR_SIZE value) {
    cmp(regWidth, reg, value);

    Label label = compiler.new_label();
    ifLabels.push_back(label);
    compiler.b_eq(label);
}

void JitArmV8CodeGen::IfNotEqual(JitWidth regWidth, RegPtr reg, RegPtr reg2) {
    cmp(regWidth, reg, reg2);

    Label label = compiler.new_label();
    ifLabels.push_back(label);
    compiler.b_eq(label);
}

void JitArmV8CodeGen::IfLessThan2(JitWidth regWidth, RegPtr reg, U32 value) {
    cmp(regWidth, reg, value);

    Label label = compiler.new_label();
    ifLabels.push_back(label);
    compiler.b_ge(label);
}

void JitArmV8CodeGen::IfLessThan2(JitWidth regWidth, RegPtr reg1, RegPtr reg2) {
    cmp(regWidth, reg1, reg2);

    Label label = compiler.new_label();
    ifLabels.push_back(label);
    compiler.b_ge(label);
}

void JitArmV8CodeGen::IfGreaterThanOrEqual(JitWidth regWidth, RegPtr reg1, RegPtr reg2) {
    cmp(regWidth, reg1, reg2);

    Label label = compiler.new_label();
    ifLabels.push_back(label);
    compiler.b_lt(label);
}

void JitArmV8CodeGen::IfGreaterThanOrEqual(JitWidth regWidth, RegPtr reg1, U32 value) {
    cmp(regWidth, reg1, value);

    Label label = compiler.new_label();
    ifLabels.push_back(label);
    compiler.b_lt(label);
}


void JitArmV8CodeGen::IfNot(JitWidth regWidth, RegPtr reg) {
    Label label = compiler.new_label();
    ifLabels.push_back(label);

    if (regWidth == JitWidth::b32) {
        compiler.cbnz(R32(reg->hardwareReg()), label);
    } else if (regWidth == JitWidth::b16) {
        RegPtr tmp = getTmpReg();
        compiler.ubfx(R32(tmp->hardwareReg()), R32(reg->hardwareReg()), 0, 16);
        compiler.cbnz(R32(tmp->hardwareReg()), label);
    } else if (regWidth == JitWidth::b8) {
        RegPtr tmp = getTmpReg();
        if (isRegHigh(reg)) {
            compiler.ubfx(R32(tmp->hardwareReg()), R32(reg->hardwareReg()), 8, 8);
        } else {
            compiler.ubfx(R32(tmp->hardwareReg()), R32(reg->hardwareReg()), 0, 8);
        }
        compiler.cbnz(R32(tmp->hardwareReg()), label);
#ifdef BOXEDWINE_64
    } else if (regWidth == JitWidth::b64) {
        compiler.cbnz(R64(reg->hardwareReg()), label);
#endif
    } else {
        kpanic_fmt("JitArmV8CodeGen::If unexpected width: %d", (U32)regWidth);
    }
}

void JitArmV8CodeGen::IfNotCPU(JitWidth regWidth, RegPtr sib, U8 lsl, U32 offset) {
    RegPtr tmp = readCPU(regWidth, sib, lsl, offset);
    IfNot(regWidth, tmp);
}

void JitArmV8CodeGen::clearMMUPermissionIfSpansPage(JitWidth width, RegPtr offset, RegPtr reg) {
    if (width == JitWidth::b16) {
        compiler.cmp(R32(offset->hardwareReg()), 0xFFF);
    } else if (width == JitWidth::b32) {
        compiler.cmp(R32(offset->hardwareReg()), 0xFFD);
    } else if (width == JitWidth::b64) {
        compiler.cmp(R32(offset->hardwareReg()), 0xFF9);
    } else if (width == JitWidth::b128) {
        compiler.cmp(R32(offset->hardwareReg()), 0xFF1);
    } else if (width == JitWidth::b256) {
        compiler.cmp(R32(offset->hardwareReg()), 0xFE1);
    } else {
        kpanic_fmt("JitArmV8CodeGen::clearRegIfSpansPage unknown width %d", (U32)width);
    }
    // :TODO: experiment with cset
    Label label = compiler.new_label();
    compiler.b_lt(label);
    compiler.mov(R64(reg->hardwareReg()), 0);
    compiler.bind(label);
}

void JitArmV8CodeGen::JumpIfCondition(JitConditional condition, U32 address) {
    IfCondition(condition);
    Label label;

    if (!opLabels.get(address, label)) {
        label = compiler.new_label();
        opLabels.set(address, label);
    }
    compiler.b(label);
    EndIf();
}

U32 JitArmV8CodeGen::MarkJumpLocation() {
    Label label = compiler.new_label();
    labels.push_back(label);
    compiler.bind(label);
    return (U32)labels.size();
}

void JitArmV8CodeGen::Goto(U32 location) {
    compiler.b(labels[location - 1]);
}

void JitArmV8CodeGen::jmp(RegPtr reg) {
    compiler.br(R64(reg->hardwareReg()));
}

void JitArmV8CodeGen::writeCPU(JitWidth width, U32 offset, RegPtr src) {
    if (width == JitWidth::b32) {
        compiler.str(R32(src->hardwareReg()), createMem(regCPU, offset));
    } else if (width == JitWidth::b16) {
        compiler.strh(R32(src->hardwareReg()), createMem(regCPU, offset));
    } else if (width == JitWidth::b8) {
        compiler.strb(R32(getReg8InLowByte(src)->hardwareReg()), createMem(regCPU, offset));
#ifdef BOXEDWINE_64
    } else if (width == JitWidth::b64) {
        compiler.str(R64(src->hardwareReg()), createMem(regCPU, offset));
#endif
    } else {
        kpanic_fmt("unknown dstWidth in JitArmV8CodeGen::writeCPU %d", width);
    }
}

void JitArmV8CodeGen::writeCPU(JitWidth width, RegPtr sib, U8 lsl, U32 offset, RegPtr src) {
    if (width == JitWidth::b32) {
        compiler.str(R32(src->hardwareReg()), createMem(regCPU, sib->hardwareReg(), lsl, offset));
    } else if (width == JitWidth::b16) {
        compiler.strh(R32(src->hardwareReg()), createMem(regCPU, sib->hardwareReg(), lsl, offset));
    } else if (width == JitWidth::b8) {
        compiler.strb(R32(getReg8InLowByte(src)->hardwareReg()), createMem(regCPU, sib->hardwareReg(), lsl, offset));
#ifdef BOXEDWINE_64
    } else if (width == JitWidth::b64) {
        compiler.str(R64(src->hardwareReg()), createMem(regCPU, sib->hardwareReg(), lsl, offset));
#endif
    } else {
        kpanic_fmt("unknown dstWidth in JitArmV8CodeGen::writeCPU %d", width);
    }
}
/*
void JitArmV8CodeGen::IfSseLessThan(SSERegPtr src1, SSERegPtr src2) {
    x86.ucomiss(X86Asm::XMM(src1->hardwareReg()), X86Asm::XMM(src2->hardwareReg()));
    x86.IfCF();
}

void JitArmV8CodeGen::storeCpuXMMReg(SSERegPtr reg, U32 index) {
    if (index >= 8) {
        kpanic("JitArmV8CodeGen::storeCpuXMMReg");
        return;
    }
    if (xmmCache[index] == INVALID_REG) {
        x86.movaps(X86Asm::Mem128(HOST_CPU, index * 16 + offsetof(CPU, xmm)), X86Asm::XMM(reg->hardwareReg()));
    } else if (xmmCache[index] != reg->hardwareReg()) {
        x86.movaps(X86Asm::XMM(xmmCache[index]), X86Asm::XMM(reg->hardwareReg()));
    }    
}

void JitArmV8CodeGen::storeXMMToMem128(SSERegPtr reg, RegPtr rm, RegPtr sib, U8 lsl, U32 disp) {
    x86.movups(X86Asm::Mem128(R(rm->hardwareReg()), R(sib->hardwareReg()), lsl, disp), X86Asm::XMM(reg->hardwareReg()));
}

void JitArmV8CodeGen::storeXMMToMem64(SSERegPtr reg, RegPtr rm, RegPtr sib, U8 lsl, U32 disp) {
    x86.movlps(X86Asm::Mem64(R(rm->hardwareReg()), R(sib->hardwareReg()), lsl, disp), X86Asm::XMM(reg->hardwareReg()));
}

void JitArmV8CodeGen::storeXMMToMem32(SSERegPtr reg, RegPtr rm, RegPtr sib, U8 lsl, U32 disp) {
    x86.movss(X86Asm::Mem32(R(rm->hardwareReg()), R(sib->hardwareReg()), lsl, disp), X86Asm::XMM(reg->hardwareReg()));
}

void JitArmV8CodeGen::storeHighXMMToMem64(SSERegPtr reg, RegPtr rm, RegPtr sib, U8 lsl, U32 disp) {
    x86.movhps(X86Asm::Mem64(R(rm->hardwareReg()), R(sib->hardwareReg()), lsl, disp), X86Asm::XMM(reg->hardwareReg()));
}

SSERegPtr JitArmV8CodeGen::getXMM(U8 index, bool load) {
    if (index != INVALID_REG && index >= 8) {
        kpanic("JitArmV8CodeGen::getXMM");
        return nullptr;
    }
    if (index != INVALID_REG && xmmCache[index] != INVALID_REG) {
        return std::make_shared<SSERegInternal>(xmmCache[index], index);
    }
    SSERegPtr result = std::shared_ptr<SSERegInternal>(new SSERegInternal(findTmpXMM(), index), [this](SSERegInternal* p) {
        xmmUsed2[p->hardwareReg()] = false;
        delete p;
    });
    if (load && index != INVALID_REG) {
        x86.movaps(X86Asm::XMM(result->hardwareReg()), X86Asm::Mem128(HOST_CPU, index * 16 + offsetof(CPU, xmm)));
    }
    return result;
}

SSERegPtr JitArmV8CodeGen::loadCpuXMMReg(U8 index) {        
    return getXMM(index, true);
}

SSERegPtr JitArmV8CodeGen::loadXMMFromMem128(U8 index, RegPtr rm, RegPtr sib, U8 lsl, U32 disp) {
    SSERegPtr reg = getXMM(index, false);
    x86.movups(X86Asm::XMM(reg->hardwareReg()), X86Asm::Mem128(R(rm->hardwareReg()), R(sib->hardwareReg()), lsl, disp));
    return reg;
}

SSERegPtr JitArmV8CodeGen::loadXMMFromMem64(U8 index, RegPtr rm, RegPtr sib, U8 lsl, U32 disp) {
    SSERegPtr reg = getXMM(index, true);
    x86.movq(X86Asm::XMM(reg->hardwareReg()), X86Asm::Mem64(R(rm->hardwareReg()), R(sib->hardwareReg()), lsl, disp));
    return reg;
}

SSERegPtr JitArmV8CodeGen::loadLowXMMFromMem64(U8 index, RegPtr rm, RegPtr sib, U8 lsl, U32 disp) {
    SSERegPtr reg = getXMM(index, true);
    x86.movlps(X86Asm::XMM(reg->hardwareReg()), X86Asm::Mem64(R(rm->hardwareReg()), R(sib->hardwareReg()), lsl, disp));
    return reg;
}

SSERegPtr JitArmV8CodeGen::loadHighXMMFromMem64(U8 index, RegPtr rm, RegPtr sib, U8 lsl, U32 disp) {
    SSERegPtr reg = getXMM(index, true);
    x86.movhps(X86Asm::XMM(reg->hardwareReg()), X86Asm::Mem64(R(rm->hardwareReg()), R(sib->hardwareReg()), lsl, disp));
    return reg;
}

SSERegPtr JitArmV8CodeGen::loadXMMFromMem32(U8 index, RegPtr rm, RegPtr sib, U8 lsl, U32 disp) {
    SSERegPtr reg = getXMM(index, true);
    x86.movss(X86Asm::XMM(reg->hardwareReg()), X86Asm::Mem32(R(rm->hardwareReg()), R(sib->hardwareReg()), lsl, disp));
    return reg;
}

void JitArmV8CodeGen::addpsXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.addps(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::addssXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.addss(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::subpsXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.subps(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::subssXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.subss(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::mulpsXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.mulps(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::mulssXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.mulss(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::divpsXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.divps(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::divssXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.divss(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::rcppsXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.rcpps(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::rcpssXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.rcpss(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::sqrtpsXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.sqrtps(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::sqrtssXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.sqrtss(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::rsqrtpsXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.rsqrtps(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::rsqrtssXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.rsqrtss(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::maxpsXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.maxps(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::maxssXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.maxss(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::minpsXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.minps(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::minssXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.minss(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

*/

asmjit::a64::Vec toMmxB(MMXRegPtr reg) {
    return asmjit::a64::Vec::make_b(reg->hardwareReg());
}

asmjit::a64::Vec toMmxB8(MMXRegPtr reg) {
    return asmjit::a64::Vec::make_b(reg->hardwareReg()).b8();
}

asmjit::a64::Vec toMmxB8(MMXRegPtr reg, U8 index) {
    asmjit::a64::Vec result = asmjit::a64::Vec::make_b(reg->hardwareReg()).b8();
    result.set_element_index(index);
    return result;
}

asmjit::a64::Vec toMmxB16(MMXRegPtr reg) {
    return asmjit::a64::Vec::make_b(reg->hardwareReg()).b16();
}

asmjit::a64::Vec toMmxH(MMXRegPtr reg) {
    return asmjit::a64::Vec::make_h(reg->hardwareReg());
}

asmjit::a64::Vec toMmxH4(MMXRegPtr reg) {
    return asmjit::a64::Vec::make_h(reg->hardwareReg()).h4();
}

asmjit::a64::Vec toMmxH4(MMXRegPtr reg, U8 index) {
    asmjit::a64::Vec result = asmjit::a64::Vec::make_h(reg->hardwareReg()).h4();
    result.set_element_index(index);
    return result;
}

asmjit::a64::Vec toMmxH8(MMXRegPtr reg) {
    return asmjit::a64::Vec::make_h(reg->hardwareReg()).h8();
}

asmjit::a64::Vec toMmxH8(MMXRegPtr reg, U8 index) {
    asmjit::a64::Vec result = asmjit::a64::Vec::make_h(reg->hardwareReg()).h8();
    result.set_element_index(index);
    return result;
}

asmjit::a64::Vec toMmxS(MMXRegPtr reg) {
    return asmjit::a64::Vec::make_s(reg->hardwareReg());
}

asmjit::a64::Vec toMmxS2(MMXRegPtr reg) {
    return asmjit::a64::Vec::make_s(reg->hardwareReg()).s2();
}

asmjit::a64::Vec toMmxS2(MMXRegPtr reg, U8 index) {
    asmjit::a64::Vec result = asmjit::a64::Vec::make_h(reg->hardwareReg()).s2();
    result.set_element_index(index);
    return result;
}

asmjit::a64::Vec toMmxS4(MMXRegPtr reg) {
    return asmjit::a64::Vec::make_s(reg->hardwareReg()).s4();
}

asmjit::a64::Vec toMmxS4(MMXRegPtr reg, U8 index) {
    asmjit::a64::Vec result = asmjit::a64::Vec::make_s(reg->hardwareReg()).s4();
    result.set_element_index(index);
    return result;
}

asmjit::a64::Vec toMmxD1(MMXRegPtr reg) {
    return asmjit::a64::Vec::make_d(reg->hardwareReg()).d();
}

void JitArmV8CodeGen::pavgbMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    compiler.urhadd(toMmxB8(dst), toMmxB8(dst), toMmxB8(src));
}

void JitArmV8CodeGen::pavgwMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    compiler.urhadd(toMmxH4(dst), toMmxH4(dst), toMmxH4(src));
}

void JitArmV8CodeGen::psadbwMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    compiler.uabd(toMmxB8(dst), toMmxB8(dst), toMmxB8(src));
    compiler.uaddlv(toMmxH(dst), toMmxB8(dst));
}

void JitArmV8CodeGen::pextrwRegMmx(RegPtr dst, MMXRegPtr src, U8 srcIndex) {
    compiler.umov(R32(dst->hardwareReg()), toMmxH4(src, srcIndex));
}

void JitArmV8CodeGen::pinsrwMmxReg(MMXRegPtr dst, RegPtr src, U8 dstIndex) {
    compiler.ins(toMmxH8(dst, dstIndex & 3), R32(src->hardwareReg()));
}

void JitArmV8CodeGen::pmaxswMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    compiler.smax(toMmxH4(dst), toMmxH4(dst), toMmxH4(src));
}

void JitArmV8CodeGen::pmaxubMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    compiler.umax(toMmxB8(dst), toMmxB8(dst), toMmxB8(src));
}

void JitArmV8CodeGen::pminswMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    compiler.smin(toMmxH4(dst), toMmxH4(dst), toMmxH4(src));
}

void JitArmV8CodeGen::pminubMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    compiler.umin(toMmxB8(dst), toMmxB8(dst), toMmxB8(src));
}

void JitArmV8CodeGen::pmovmskbMmxMmx(RegPtr dst, MMXRegPtr src) {
    MMXRegPtr tmp = getTmpMMX();
    // turn all the bits to 1 if signed
    compiler.sshr(toMmxB8(tmp), toMmxB8(src), 7);
    // mask out the bit that should be set, so index 0 will set bit 0, index 1 will set bit 1, etc
    compiler.and_(toMmxB8(tmp), toMmxB8(tmp), toMmxB8(loadMMXConst(SSE_BYTE8_BIT_MASK)));
    // add bits 0-7 for indexes 0-7 to end up with the mask
    compiler.addv(toMmxB(tmp), toMmxB8(tmp));

    compiler.umov(R32(dst->hardwareReg()), toMmxB8(tmp, 0));
}

void JitArmV8CodeGen::pmulhuwMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    MMXRegPtr tmp = getTmpMMX();
    compiler.umull(toMmxS2(tmp), toMmxH4(dst), toMmxH4(src));
    compiler.shrn(toMmxH4(dst), toMmxS2(tmp), 16);
}

void JitArmV8CodeGen::pshuflwNotOverlappingDstSrc(MMXRegPtr dst, MMXRegPtr src, U8 mask) {
    compiler.ins(toMmxH8(dst, 0), toMmxH8(src, mask & 3));
    compiler.ins(toMmxH8(dst, 1), toMmxH8(src, (mask >> 2) & 3));
    compiler.ins(toMmxH8(dst, 2), toMmxH8(src, (mask >> 4) & 3));
    compiler.ins(toMmxH8(dst, 3), toMmxH8(src, (mask >> 6) & 3));
}

void JitArmV8CodeGen::pshufwMmxMmx(MMXRegPtr dst, MMXRegPtr src, U8 order) {
    if (dst->hardwareReg() == src->hardwareReg()) {
        MMXRegPtr tmp = getTmpMMX();
        pshuflwNotOverlappingDstSrc(tmp, src, order);
        compiler.mov(toMmxD1(dst), toMmxD1(tmp));
    } else {
        pshuflwNotOverlappingDstSrc(dst, src, order);
    }
}

void JitArmV8CodeGen::maskmovq(MMXRegPtr src, MMXRegPtr mask, RegPtr destAddress) {
    MMXRegPtr tmp = getTmpMMX();
    MMXRegPtr tmpMask = getTmpMMX();

    compiler.ldr(toMmxD1(tmp), createMem(destAddress, 0));
    // spec: The most significant bit in each byte of the mask operand determines whether the corresponding byte in the source operand is written to the corresponding byte location in memory: 0 indicates no write and 1 indicates write.
    compiler.sshr(toMmxB8(tmpMask), toMmxB8(mask), 7);
    compiler.bsl(toMmxB8(tmpMask), toMmxB8(src), toMmxB8(tmp)); // if tmpMask[i] == 1 then tmpMask[i] = src[i] else tmpMaskd[i] = tmp[i]
    compiler.str(toMmxD1(tmpMask), createMem(destAddress, 0));
}

void JitArmV8CodeGen::paddqMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    compiler.add(toMmxD1(dst), toMmxD1(dst), toMmxD1(src));
}

void JitArmV8CodeGen::psubqMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    compiler.sub(toMmxD1(dst), toMmxD1(dst), toMmxD1(src));
}

void JitArmV8CodeGen::pmuludqMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    compiler.umull(toMmxD1(dst), toMmxS2(dst), toMmxS2(src));
}
/*
void JitArmV8CodeGen::andnpsXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.andnps(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::andpsXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.andps(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::orpsXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.orps(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::xorpsXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.xorps(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::cvtpi2psXmmMmx(SSERegPtr dst, MMXRegPtr src) {
    // cvtpi2ps need to keep top 64-bits of the xmm dst
    SSERegPtr tmp = getTmpSSE();
    x86.cvtdq2ps(X86Asm::XMM(tmp->hardwareReg()), getMMXReg(src));
    x86.movsd(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(tmp->hardwareReg()));
}

void JitArmV8CodeGen::cvtps2piMmxXmm(MMXRegPtr dst, SSERegPtr src) {
    x86.cvtps2dq(getMMXReg(dst), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::cvtsi2ssXmmR32(SSERegPtr dst, RegPtr src) {
    x86.cvtsi2ss(X86Asm::XMM(dst->hardwareReg()), R32(src->hardwareReg()));
}

void JitArmV8CodeGen::cvtss2siR32Xmm(RegPtr dst, SSERegPtr src) {
    x86.cvtss2si(R32(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::cvttps2piMmxXmm(MMXRegPtr dst, SSERegPtr src) {
    x86.cvttps2dq(getMMXReg(dst), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::cvttss2siR32Xmm(RegPtr dst, SSERegPtr src) {
    x86.cvttss2si(R32(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::movhlpsXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.movhlps(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::movlhpsXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.movlhps(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::movssXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.movss(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::shufpsXmmXmm(SSERegPtr dst, SSERegPtr src, U32 imm) {
    x86.shufps(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()), (U8)imm);
}

void JitArmV8CodeGen::cmppsXmmXmm(SSERegPtr dst, SSERegPtr src, U32 imm) {
    x86.cmpps(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()), (U8)imm);
}

void JitArmV8CodeGen::cmpssXmmXmm(SSERegPtr dst, SSERegPtr src, U32 imm) {
    x86.cmpss(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()), (U8)imm);
}
*/
void JitArmV8CodeGen::setFlags(RegPtr flags, U32 mask) {
    RegPtr maskedFlags = getTmpReg();
    if (asmjit::a64::Utils::is_logical_imm(mask, 32)) {
        compiler.and_(R32(maskedFlags->hardwareReg()), R32(flags->hardwareReg()), mask);
        compiler.and_(xFLAGS, xFLAGS, ~mask);
    } else {
        RegPtr maskReg = loadConst(mask);
        compiler.and_(R32(maskedFlags->hardwareReg()), R32(flags->hardwareReg()), R32(maskReg->hardwareReg()));
        compiler.mvn_(R32(maskReg->hardwareReg()), R32(maskReg->hardwareReg()));
        compiler.and_(xFLAGS, xFLAGS, R32(maskReg->hardwareReg()));
    } 
    compiler.orr(xFLAGS, xFLAGS, R32(maskedFlags->hardwareReg()));
    currentLazyFlags = FLAGS_NONE;
    storeLazyFlagType(FLAGS_NONE);
}

void JitArmV8CodeGen::writeFlags(RegPtr flags) {
    compiler.mov(xFLAGS, R32(flags->hardwareReg()));
}

RegPtr JitArmV8CodeGen::getFlagsInTmp(RegPtr tmp) {
    if (!tmp) {
        tmp = getTmpReg();
    }
    compiler.mov(R32(tmp->hardwareReg()), xFLAGS);
    return tmp;
}

void JitArmV8CodeGen::storeLazyFlagType(LazyFlagType flags) {
    if (flags == FLAGS_CFOF) {
        RegPtr lazyFlags = getLazyFlagType();
        IfNotEqual(JitWidth::b32, lazyFlags, FLAGS_CFOF); {
            writeCPU(JitWidth::b8, offsetof(CPU, lazyFlagTypePrev), lazyFlags);
            compiler.mov(xFlagsType, flags);
        } EndIf();
    } else {
        compiler.mov(xFlagsType, flags);
    }
}

RegPtr JitArmV8CodeGen::getLazyFlagType() {
    return std::make_shared<JitReg>(regFlagsType, 0xff);
}

RegPtr JitArmV8CodeGen::getLazyFlagTypeInTmp() {
    RegPtr reg = getTmpReg();
    compiler.mov(R32(reg->hardwareReg()), xFlagsType);
    return reg;
}

RegPtr JitArmV8CodeGen::getReadOnlyFlags() {
    return std::make_shared<JitReg>(regFlags, 0xff);
}
/*
void JitArmV8CodeGen::updateFlagsIfNecessary() {
    U32 neededFlags = currentOp->needsToSetFlags(cpu);
    if (neededFlags) {        
        RegPtr flags = readCPU(JitWidth::b32, offsetof(CPU, flags));
        RegPtr tmp = getTmpReg8();
        
        if (neededFlags == CF) {
            movValue(JitWidth::b32, tmp, 0); // not xor, we don't want to affect flags
            x86.setb(R8(tmp->hardwareReg()));
            andValue(JitWidth::b32, flags, ~CF);
            orReg(JitWidth::b32, flags, tmp);
        } else if (neededFlags == ZF) {
            movValue(JitWidth::b32, tmp, 0); // not xor, we don't want to affect flags
            x86.setz(R8(tmp->hardwareReg()));
            shlValue(JitWidth::b32, tmp, 6);
            andValue(JitWidth::b32, flags, ~ZF);
            orReg(JitWidth::b32, flags, tmp);
        } else if (neededFlags == SF) {
            movValue(JitWidth::b32, tmp, 0); // not xor, we don't want to affect flags
            x86.sets(R8(tmp->hardwareReg()));
            shlValue(JitWidth::b32, tmp, 7);
            andValue(JitWidth::b32, flags, ~SF);
            orReg(JitWidth::b32, flags, tmp);
        } else if (neededFlags == OF) {
            movValue(JitWidth::b32, tmp, 0); // not xor, we don't want to affect flags
            x86.seto(R8(tmp->hardwareReg()));
            shlValue(JitWidth::b32, tmp, 11);
            andValue(JitWidth::b32, flags, ~OF);
            orReg(JitWidth::b32, flags, tmp);
        } else if (neededFlags == PF) {
            movValue(JitWidth::b32, tmp, 0); // not xor, we don't want to affect flags
            x86.setp(R8(tmp->hardwareReg()));
            shlValue(JitWidth::b32, tmp, 2);
            andValue(JitWidth::b32, flags, ~PF);
            orReg(JitWidth::b32, flags, tmp);
        } else {
            bool savedEAX = false;

            if (regUsed2[0] || regCache[0] == 0) {
                x86.xchg(RN(0), RN(tmp->hardwareReg()));
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
            andValue(JitWidth::b32, flags, ~FMASK_TEST);
            x86.or_(R16(flags->hardwareReg()), x86.ax);
            if (savedEAX) {
                x86.xchg(RN(0), RN(tmp->hardwareReg()));
            }
        }
        writeCPU(JitWidth::b32, offsetof(CPU, flags), flags);
        storeLazyFlags(FLAGS_NONE);
        currentLazyFlags = FLAGS_NONE;
    }
}

void JitArmV8CodeGen::comissXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.comiss(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
    updateFlagsIfNecessary();
}

void JitArmV8CodeGen::ucomissXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.ucomiss(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
    updateFlagsIfNecessary();
}

void JitArmV8CodeGen::stmxcsr(RegPtr address) {
    x86.stmxcsr(X86Asm::Mem32(RN(address->hardwareReg()), 0));
}

void JitArmV8CodeGen::ldmxcsr(RegPtr address) {
    x86.ldmxcsr(X86Asm::Mem32(RN(address->hardwareReg()), 0));
}

void JitArmV8CodeGen::sfence() {
    x86.sfence();
}

void JitArmV8CodeGen::unpckhpsXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.unpckhps(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::unpcklpsXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.unpcklps(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::movmskpsR32Xmm(RegPtr dst, SSERegPtr src) {
    x86.movmskps(R32(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

*/

MMXRegPtr JitArmV8CodeGen::loadMMXFromReg(RegPtr src) {
    MMXRegPtr tmp = getTmpMMX();
    compiler.fmov(toMmxS(tmp), R32(src->hardwareReg()));
    return tmp;
}

void JitArmV8CodeGen::storeCpuMMXReg(MMXRegPtr reg, U32 index) {
    compiler.str(toMmxD1(reg), createMem(regCPU, index * cpu->fpu.sizeofRegInRegsArray() + offsetof(CPU, fpu.regs[0].signif)));
}

void JitArmV8CodeGen::storeMMXToReg(MMXRegPtr src, RegPtr dst) {
    compiler.umov(R32(dst->hardwareReg()), toMmxS4(src, 0));
}

MMXRegPtr JitArmV8CodeGen::loadCpuMMXReg(U8 index) {
    MMXRegPtr tmp = getTmpMMX();
    compiler.ldr(toMmxD1(tmp), createMem(regCPU, index * cpu->fpu.sizeofRegInRegsArray() + offsetof(CPU, fpu.regs[0].signif)));
    return tmp;
}

MMXRegPtr JitArmV8CodeGen::loadMMXFromMem32(U8 index, RegPtr rm, RegPtr sib, U8 lsl, U32 disp) {
    MMXRegPtr tmp = getTmpMMX();
    compiler.ldr(toMmxS2(tmp), createMem(rm, sib, 0, 0));
    return tmp;
}

MMXRegPtr JitArmV8CodeGen::loadMMXFromMem64(U8 index, RegPtr rm, RegPtr sib, U8 lsl, U32 disp) {
    MMXRegPtr tmp = getTmpMMX();
    compiler.ldr(toMmxD1(tmp), createMem(rm, sib, 0, 0));
    return tmp;
}

void JitArmV8CodeGen::storeMMXToMem32(MMXRegPtr reg, RegPtr rm, RegPtr sib, U8 lsl, U32 disp) {
    compiler.str(toMmxS2(reg), createMem(rm, sib, 0, 0));
}

void JitArmV8CodeGen::storeMMXToMem64(MMXRegPtr reg, RegPtr rm, RegPtr sib, U8 lsl, U32 disp) {
    compiler.str(toMmxD1(reg), createMem(rm, sib, 0, 0));
}

void JitArmV8CodeGen::xorMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    compiler.eor(toMmxD1(dst), toMmxD1(dst), toMmxD1(src));
}

void JitArmV8CodeGen::orMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    compiler.orr(toMmxD1(dst), toMmxD1(dst), toMmxD1(src));
}

void JitArmV8CodeGen::andMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    compiler.and_(toMmxD1(dst), toMmxD1(dst), toMmxD1(src));
}

void JitArmV8CodeGen::andnMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    compiler.bic(toMmxD1(dst), toMmxD1(src), toMmxD1(dst));
}

void JitArmV8CodeGen::psllwMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    MMXRegPtr tmp = getTmpMMX();
    compiler.dup(toMmxH4(tmp), toMmxH4(src, 0));
    compiler.ushl(toMmxH4(dst), toMmxH4(dst), toMmxH4(tmp));
}

void JitArmV8CodeGen::psrlwMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    MMXRegPtr tmp = getTmpMMX();
    // negative values in src means shift right for sshl
    compiler.dup(toMmxH4(tmp), toMmxH4(src, 0));
    compiler.neg(toMmxH4(tmp), toMmxH4(tmp));
    compiler.ushl(toMmxH4(dst), toMmxH4(dst), toMmxH4(tmp));
}

void JitArmV8CodeGen::psrawMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    MMXRegPtr tmp = getTmpMMX();
    // negative values in src means shift right for sshl
    compiler.dup(toMmxH4(tmp), toMmxH4(src, 0));
    compiler.neg(toMmxH4(tmp), toMmxH4(tmp));
    compiler.sshl(toMmxH4(dst), toMmxH4(dst), toMmxH4(tmp));
}

void JitArmV8CodeGen::psllwMmx(MMXRegPtr dst, U32 imm) {
    compiler.shl(toMmxH4(dst), toMmxH4(dst), imm);
}

void JitArmV8CodeGen::psrlwMmx(MMXRegPtr dst, U32 imm) {
    compiler.ushr(toMmxH4(dst), toMmxH4(dst), imm);
}

void JitArmV8CodeGen::psrawMmx(MMXRegPtr dst, U32 imm) {
    compiler.sshr(toMmxH4(dst), toMmxH4(dst), imm);
}

void JitArmV8CodeGen::pslldMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    MMXRegPtr tmp = getTmpMMX();
    compiler.dup(toMmxS2(tmp), toMmxS2(src, 0));
    compiler.ushl(toMmxS2(dst), toMmxS2(dst), toMmxS2(tmp));
}

void JitArmV8CodeGen::psrldMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    MMXRegPtr tmp = getTmpMMX();
    // negative values in src means shift right for sshl
    compiler.dup(toMmxS2(tmp), toMmxS2(src, 0));
    compiler.neg(toMmxS2(tmp), toMmxS2(tmp));
    compiler.ushl(toMmxS2(dst), toMmxS2(dst), toMmxS2(tmp));
}

void JitArmV8CodeGen::psradMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    MMXRegPtr tmp = getTmpMMX();
    // negative values in src means shift right for sshl
    compiler.dup(toMmxS2(tmp), toMmxS2(src, 0));
    compiler.neg(toMmxS2(tmp), toMmxS2(tmp));
    compiler.sshl(toMmxS2(dst), toMmxS2(dst), toMmxS2(tmp));
}

void JitArmV8CodeGen::pslldMmx(MMXRegPtr dst, U32 imm) {
    compiler.shl(toMmxS2(dst), toMmxS2(dst), imm);
}

void JitArmV8CodeGen::psrldMmx(MMXRegPtr dst, U32 imm) {
    compiler.ushr(toMmxS2(dst), toMmxS2(dst), imm);
}

void JitArmV8CodeGen::psradMmx(MMXRegPtr dst, U32 imm) {
    compiler.sshr(toMmxS2(dst), toMmxS2(dst), imm);
}

void JitArmV8CodeGen::psllqMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    compiler.ushl(toMmxD1(dst), toMmxD1(dst), toMmxD1(src));
}

void JitArmV8CodeGen::psrlqMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    MMXRegPtr tmp = getTmpMMX();
    // negative values in src means shift right for sshl
    compiler.neg(toMmxD1(tmp), toMmxD1(src));
    compiler.ushl(toMmxD1(dst), toMmxD1(dst), toMmxD1(tmp));
}

void JitArmV8CodeGen::psllqMmx(MMXRegPtr dst, U32 imm) {
    compiler.shl(toMmxD1(dst), toMmxD1(dst), imm);
}

void JitArmV8CodeGen::psrlqMmx(MMXRegPtr dst, U32 imm) {
    compiler.ushr(toMmxD1(dst), toMmxD1(dst), imm);
}

void JitArmV8CodeGen::paddbMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    compiler.add(toMmxB8(dst), toMmxB8(dst), toMmxB8(src));
}

void JitArmV8CodeGen::paddwMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    compiler.add(toMmxH4(dst), toMmxH4(dst), toMmxH4(src));
}

void JitArmV8CodeGen::padddMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    compiler.add(toMmxS2(dst), toMmxS2(dst), toMmxS2(src));
}

void JitArmV8CodeGen::paddsbMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    compiler.sqadd(toMmxB8(dst), toMmxB8(dst), toMmxB8(src));
}

void JitArmV8CodeGen::paddswMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    compiler.sqadd(toMmxH4(dst), toMmxH4(dst), toMmxH4(src));
}

void JitArmV8CodeGen::paddusbMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    compiler.uqadd(toMmxB8(dst), toMmxB8(dst), toMmxB8(src));
}

void JitArmV8CodeGen::padduswMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    compiler.uqadd(toMmxH4(dst), toMmxH4(dst), toMmxH4(src));
}

void JitArmV8CodeGen::psubbMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    compiler.sub(toMmxB8(dst), toMmxB8(dst), toMmxB8(src));
}

void JitArmV8CodeGen::psubwMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    compiler.sub(toMmxH4(dst), toMmxH4(dst), toMmxH4(src));
}

void JitArmV8CodeGen::psubdMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    compiler.sub(toMmxS2(dst), toMmxS2(dst), toMmxS2(src));
}

void JitArmV8CodeGen::psubsbMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    compiler.sqsub(toMmxB8(dst), toMmxB8(dst), toMmxB8(src));
}

void JitArmV8CodeGen::psubswMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    compiler.sqsub(toMmxH4(dst), toMmxH4(dst), toMmxH4(src));
}

void JitArmV8CodeGen::psubusbMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    compiler.uqsub(toMmxB8(dst), toMmxB8(dst), toMmxB8(src));
}

void JitArmV8CodeGen::psubuswMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    compiler.uqsub(toMmxH4(dst), toMmxH4(dst), toMmxH4(src));
}

void JitArmV8CodeGen::pmulhwMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    MMXRegPtr tmp = getTmpMMX();
    compiler.smull(toMmxS2(tmp), toMmxH4(dst), toMmxH4(src));
    compiler.shrn(toMmxH4(dst), toMmxS2(tmp), 16);
}

void JitArmV8CodeGen::pmullwMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    compiler.mul(toMmxH4(dst), toMmxH4(dst), toMmxH4(src));
}

void JitArmV8CodeGen::pmaddwdMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    MMXRegPtr tmp = getTmpMMX();

    compiler.smull(toMmxS4(tmp), toMmxH4(dst), toMmxH4(src));
    compiler.addp(toMmxS4(dst), toMmxS4(tmp), toMmxS4(tmp));
}

void JitArmV8CodeGen::pcmpeqbMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    compiler.cmeq(toMmxB8(dst), toMmxB8(dst), toMmxB8(src));
}

void JitArmV8CodeGen::pcmpeqwMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    compiler.cmeq(toMmxH4(dst), toMmxH4(dst), toMmxH4(src));
}

void JitArmV8CodeGen::pcmpeqdMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    compiler.cmeq(toMmxS2(dst), toMmxS2(dst), toMmxS2(src));
}

void JitArmV8CodeGen::pcmpgtbMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    compiler.cmgt(toMmxB8(dst), toMmxB8(dst), toMmxB8(src));
}

void JitArmV8CodeGen::pcmpgtwMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    compiler.cmgt(toMmxH4(dst), toMmxH4(dst), toMmxH4(src));
}

void JitArmV8CodeGen::pcmpgtdMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    compiler.cmgt(toMmxS2(dst), toMmxS2(dst), toMmxS2(src));
}

void JitArmV8CodeGen::packsswbMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    compiler.sqxtn(toMmxB8(dst), toMmxH4(dst));
    compiler.sqxtn2(toMmxB16(dst), toMmxH4(src));
    compiler.ins(toMmxS4(dst, 1), toMmxS4(dst, 2));
}

void JitArmV8CodeGen::packssdwMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    compiler.sqxtn(toMmxH4(dst), toMmxS2(dst));
    compiler.sqxtn2(toMmxH8(dst), toMmxS2(src));
    compiler.ins(toMmxS4(dst, 1), toMmxS4(dst, 2));
}

void JitArmV8CodeGen::packuswbMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    compiler.sqxtun(toMmxB8(dst), toMmxH4(dst));
    compiler.sqxtun2(toMmxB16(dst), toMmxH4(src));
    compiler.ins(toMmxS4(dst, 1), toMmxS4(dst, 2));
}

void JitArmV8CodeGen::punpckhbwMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    // dest->ub.b0 = dest->ub.b4;
    // dest->ub.b1 = src->ub.b4;
    // dest->ub.b2 = dest->ub.b5;
    // dest->ub.b3 = src->ub.b5;
    // dest->ub.b4 = dest->ub.b6;
    // dest->ub.b5 = src->ub.b6;
    // dest->ub.b6 = dest->ub.b7;
    // dest->ub.b7 = src->ub.b7;
    compiler.zip2(toMmxB8(dst), toMmxB8(dst), toMmxB8(src));
}

void JitArmV8CodeGen::punpckhwdMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    compiler.zip2(toMmxH4(dst), toMmxH4(dst), toMmxH4(src));
}

void JitArmV8CodeGen::punpckhdqMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    compiler.zip2(toMmxS2(dst), toMmxS2(dst), toMmxS2(src));
}

void JitArmV8CodeGen::punpcklbwMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    // dest->ub.b7 = src->ub.b3;
    // dest->ub.b6 = dest->ub.b3;
    // dest->ub.b5 = src->ub.b2;
    // dest->ub.b4 = dest->ub.b2;
    // dest->ub.b3 = src->ub.b1;
    // dest->ub.b2 = dest->ub.b1;
    // dest->ub.b1 = src->ub.b0;
    // dest->ub.b0 = dest->ub.b0;
    compiler.zip1(toMmxB8(dst), toMmxB8(dst), toMmxB8(src));
}

void JitArmV8CodeGen::punpcklwdMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    compiler.zip1(toMmxH4(dst), toMmxH4(dst), toMmxH4(src));
}

void JitArmV8CodeGen::punpckldqMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    compiler.zip1(toMmxS2(dst), toMmxS2(dst), toMmxS2(src));
}

/*
#ifdef BOXEDWINE_64
void JitArmV8CodeGen::cvtsi2sdXmmR64(SSERegPtr dst, RegPtr src) {
    x86.cvtsi2sd(X86Asm::XMM(dst->hardwareReg()), R64(src->hardwareReg()));
}
#endif

void JitArmV8CodeGen::addpdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.addpd(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::addsdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.addsd(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::subpdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.subpd(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::subsdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.subsd(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::mulpdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.mulpd(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::mulsdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.mulsd(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::divpdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.divpd(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::divsdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.divsd(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::maxpdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.maxpd(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::maxsdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.maxsd(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::minpdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.minpd(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::minsdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.minsd(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::paddbXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.paddb(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::paddwXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.paddw(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::padddXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.paddd(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::paddqXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.paddq(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::paddsbXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.paddsb(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::paddswXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.paddsw(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::paddusbXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.paddusb(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::padduswXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.paddusw(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::psubbXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.psubb(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::psubwXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.psubw(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::psubdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.psubd(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::psubqXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.psubq(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::psubsbXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.psubsb(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::psubswXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.psubsw(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::psubusbXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.psubusb(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::psubuswXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.psubusw(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::pmaddwdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.pmaddwd(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::pmulhwXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.pmulhw(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::pmullwXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.pmullw(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::pmuludqXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.pmuludq(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::sqrtpdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.sqrtpd(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::sqrtsdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.sqrtsd(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::andnpdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.andnpd(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::andpdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.andpd(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::pandXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.pand(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::pandnXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.pandn(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::porXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.por(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::pslldqXmm(SSERegPtr dst, U32 imm) {
    x86.pslldq(X86Asm::XMM(dst->hardwareReg()), imm);
}

void JitArmV8CodeGen::psllqXmm(SSERegPtr dst, U32 imm) {
    x86.psllq(X86Asm::XMM(dst->hardwareReg()), imm);
}

void JitArmV8CodeGen::pslldXmm(SSERegPtr dst, U32 imm) {
    x86.pslld(X86Asm::XMM(dst->hardwareReg()), imm);
}

void JitArmV8CodeGen::psllwXmm(SSERegPtr dst, U32 imm) {
    x86.psllw(X86Asm::XMM(dst->hardwareReg()), imm);
}

void JitArmV8CodeGen::psradXmm(SSERegPtr dst, U32 imm) {
    x86.psrad(X86Asm::XMM(dst->hardwareReg()), imm);
}

void JitArmV8CodeGen::psrawXmm(SSERegPtr dst, U32 imm) {
    x86.psraw(X86Asm::XMM(dst->hardwareReg()), imm);
}

void JitArmV8CodeGen::psrldqXmm(SSERegPtr dst, U32 imm) {
    x86.psrldq(X86Asm::XMM(dst->hardwareReg()), imm);
}

void JitArmV8CodeGen::psrlqXmm(SSERegPtr dst, U32 imm) {
    x86.psrlq(X86Asm::XMM(dst->hardwareReg()), imm);
}

void JitArmV8CodeGen::psrldXmm(SSERegPtr dst, U32 imm) {
    x86.psrld(X86Asm::XMM(dst->hardwareReg()), imm);
}

void JitArmV8CodeGen::psrlwXmm(SSERegPtr dst, U32 imm) {
    x86.psrlw(X86Asm::XMM(dst->hardwareReg()), imm);
}

void JitArmV8CodeGen::psllqXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.psllq(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::pslldXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.pslld(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::psllwXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.psllw(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::psradXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.psrad(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::psrawXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.psraw(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::psrlqXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.psrlq(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::psrldXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.psrld(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::psrlwXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.psrlw(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::pxorXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.pxor(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::orpdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.orpd(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::xorpdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.xorpd(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::cmppdXmmXmm(SSERegPtr dst, SSERegPtr src, U32 imm) {
    x86.cmppd(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()), (U8)imm);
}

void JitArmV8CodeGen::cmpsdXmmXmm(SSERegPtr dst, SSERegPtr src, U32 imm) {
    x86.cmpsd(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()), (U8)imm);
}

void JitArmV8CodeGen::comisdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.comisd(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
    updateFlagsIfNecessary();
}

void JitArmV8CodeGen::ucomisdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.ucomisd(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
    updateFlagsIfNecessary();
}

void JitArmV8CodeGen::pcmpgtbXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.pcmpgtb(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::pcmpgtwXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.pcmpgtw(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::pcmpgtdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.pcmpgtd(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::pcmpeqbXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.pcmpeqb(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::pcmpeqwXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.pcmpeqw(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::pcmpeqdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.pcmpeqd(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::cvtdq2pdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.cvtdq2pd(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::cvtdq2psXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.cvtdq2ps(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::cvtpd2piMmxXmm(MMXRegPtr dst, SSERegPtr src) {
    x86.cvtpd2dq(getMMXReg(dst), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::cvtpi2pdXmmMmx(SSERegPtr dst, MMXRegPtr src) {
    x86.cvtdq2pd(X86Asm::XMM(dst->hardwareReg()), getMMXReg(src));
}

void JitArmV8CodeGen::cvtpd2dqXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.cvtpd2dq(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::cvtpd2psXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.cvtpd2ps(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::cvttpd2piMmxXmm(MMXRegPtr dst, SSERegPtr src) {
    x86.cvttpd2dq(getMMXReg(dst), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::cvtps2dqXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.cvtps2dq(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::cvtps2pdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.cvtps2pd(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::cvtsd2siR32Xmm(RegPtr dst, SSERegPtr src) {
    x86.cvtsd2si(R32(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::cvtsd2ssXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.cvtsd2ss(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::cvtsi2sdXmmR32(SSERegPtr dst, RegPtr src) {
    x86.cvtsi2sd(X86Asm::XMM(dst->hardwareReg()), R32(src->hardwareReg()));
}

void JitArmV8CodeGen::cvtss2sdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.cvtss2sd(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::cvttpd2dqXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.cvttpd2dq(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::cvttps2dqXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.cvttps2dq(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::cvttsd2siR32Xmm(RegPtr dst, SSERegPtr src) {
    x86.cvttsd2si(R32(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::movsdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.movsd(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::movupdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.movdqu(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::movmskpd(RegPtr dst, SSERegPtr src) {
    x86.movmskpd(R32(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::movd(RegPtr dst, SSERegPtr src) {
    x86.movd(R32(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::movd(SSERegPtr dst, RegPtr src) {
    x86.movd(X86Asm::XMM(dst->hardwareReg()), R32(src->hardwareReg()));
}

void JitArmV8CodeGen::movdq2q(MMXRegPtr dst, SSERegPtr src) {
    x86.movq(getMMXReg(dst), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::movq2dq(SSERegPtr dst, MMXRegPtr src) {
    x86.movq(X86Asm::XMM(dst->hardwareReg()), getMMXReg(src));
}

void JitArmV8CodeGen::movq(SSERegPtr dst, SSERegPtr src) {
    x86.movq(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::maskmovdqu(SSERegPtr src, SSERegPtr mask, RegPtr address) {
    x86.push(RN(7));
    x86.mov(RN(7), RN(address->hardwareReg()));
    x86.maskmovdqu(X86Asm::XMM(src->hardwareReg()), X86Asm::XMM(mask->hardwareReg()));
    x86.pop(RN(7));
}

void JitArmV8CodeGen::pshufdXmmXmm(SSERegPtr dst, SSERegPtr src, U32 imm) {
    x86.pshufd(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()), imm);
}

void JitArmV8CodeGen::pshufhwXmmXmm(SSERegPtr dst, SSERegPtr src, U32 imm) {
    x86.pshufhw(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()), imm);
}

void JitArmV8CodeGen::pshuflwXmmXmm(SSERegPtr dst, SSERegPtr src, U32 imm) {
    x86.pshuflw(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()), imm);
}

void JitArmV8CodeGen::shufpdXmmXmm(SSERegPtr dst, SSERegPtr src, U32 imm) {
    x86.shufpd(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()), imm);
}

void JitArmV8CodeGen::unpckhpdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.unpckhpd(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::unpcklpdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.unpcklpd(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::punpckhbwXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.punpckhbw(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::punpckhwdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.punpckhwd(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::punpckhdqXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.punpckhdq(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::punpckhqdqXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.punpckhqdq(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::punpcklbwXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.punpcklbw(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::punpcklwdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.punpcklwd(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::punpckldqXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.punpckldq(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::punpcklqdqXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.punpcklqdq(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::packssdwXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.packssdw(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::packsswbXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.packsswb(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::packuswbXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.packuswb(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::pavgbXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.pavgb(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::pavgwXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.pavgw(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::psadbwXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.psadbw(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::pmaxswXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.pmaxsw(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::pmaxubXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.pmaxub(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::pminswXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.pminsw(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::pminubXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.pminub(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::pmulhuwXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.pmulhuw(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitArmV8CodeGen::lfence() {
    x86.lfence();
}

void JitArmV8CodeGen::mfence() {
    x86.mfence();
}

void JitArmV8CodeGen::clflush(RegPtr rm, RegPtr sib, U8 lsl, U32 disp) {
    x86.clflush(X86Asm::Mem8(R32(rm->hardwareReg()), R32(sib->hardwareReg()), lsl, disp));
}

void JitArmV8CodeGen::pause() {
    x86.pause();
}

void JitArmV8CodeGen::pextrwR32Xmm(RegPtr dst, SSERegPtr src, U32 imm) {
    x86.pextrw(R32(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()), (U8)imm);
}

void JitArmV8CodeGen::pinsrwXmmR32(SSERegPtr dst, RegPtr src, U32 imm) {
    x86.pinsrw(X86Asm::XMM(dst->hardwareReg()), R32(src->hardwareReg()), (U8)imm);
}

void JitArmV8CodeGen::pmovmskbR32Xmm(RegPtr dst, SSERegPtr src) {
    x86.pmovmskb(R32(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

*/

asmjit::a64::Vec toVec(FPURegPtr reg) {
    return asmjit::a64::Vec::make_d(reg->hardwareReg());
}

asmjit::a64::Vec toVec32(FPURegPtr reg) {
    return asmjit::a64::Vec::make_s(reg->hardwareReg());
}

void JitArmV8CodeGen::dynamic_FNINIT(DecodedOp* op) {
    JitFPU::dynamic_FNINIT(op);
    fpuRoundingMode = ROUND_Nearest;
}

bool JitArmV8CodeGen::shouldContinueCompilingAfterOp(DecodedOp* op) {
    // end a compile chain when an instruction that sets fpu rounding is called, this way it will be set in cpu->fpu.round when we compile what comes after in the next block
    return op->inst != FLDCW && op->inst != FRSTOR && op->inst != Fxrstor;
}

void JitArmV8CodeGen::updateFPURounding() {
}

void JitArmV8CodeGen::restoreFPURounding() {
}

void JitArmV8CodeGen::storeCpuFpuReg(FPURegPtr reg, RegPtr index) {
    compiler.str(toVec(reg), createMem(regCPU, index->hardwareReg(), 3, offsetof(CPU, fpu.regCache[0].d)));
}

void JitArmV8CodeGen::loadCpuFpuReg(FPURegPtr reg, RegPtr index) {
    compiler.ldr(toVec(reg), createMem(regCPU, index->hardwareReg(), 3, offsetof(CPU, fpu.regCache[0].d)));
}

void JitArmV8CodeGen::loadCpuFpuRegConst(FPURegPtr reg, U32 offset) {
    compiler.ldr(toVec(reg), Mem(xCPU, offset));
}

RegPtr JitArmV8CodeGen::fpuRegToInt32(FPURegPtr fpuRegSrc, bool truncate) {
    RegPtr result = getTmpReg();
    if (truncate) {
        compiler.fcvtzs(R32(result->hardwareReg()), toVec(fpuRegSrc));
    } else {
        if (fpuRoundingMode == ROUND_Nearest) {
            compiler.fcvtns(R32(result->hardwareReg()), toVec(fpuRegSrc));
        } else if (fpuRoundingMode == ROUND_Down) {
            compiler.fcvtms(R32(result->hardwareReg()), toVec(fpuRegSrc));
        } else if (fpuRoundingMode == ROUND_Up) {
            compiler.fcvtps(R32(result->hardwareReg()), toVec(fpuRegSrc));
        } else if (fpuRoundingMode == ROUND_Chop) {
            compiler.fcvtzs(R32(result->hardwareReg()), toVec(fpuRegSrc));
        }
    }
    return result;
}

void JitArmV8CodeGen::storeFPUToInt64(FPURegPtr src, RegPtr address, RegPtr offset, bool truncate) {
    RegPtr result = getTmpReg();
    if (truncate) {
        compiler.fcvtzs(R64(result->hardwareReg()), toVec(src));
    } else {
        if (fpuRoundingMode == ROUND_Nearest) {
            compiler.fcvtns(R64(result->hardwareReg()), toVec(src));
        } else if (fpuRoundingMode == ROUND_Down) {
            compiler.fcvtms(R64(result->hardwareReg()), toVec(src));
        } else if (fpuRoundingMode == ROUND_Up) {
            compiler.fcvtps(R64(result->hardwareReg()), toVec(src));
        } else if (fpuRoundingMode == ROUND_Chop) {
            compiler.fcvtzs(R64(result->hardwareReg()), toVec(src));
        }
    }
    write(JitWidth::b64, address, offset, 0, 0, result);
}

void JitArmV8CodeGen::roundFPUToInt64(FPURegPtr src) {
    if (fpuRoundingMode == ROUND_Nearest) {
        compiler.frintn(asmjit::a64::Vec::make_v64(src->hardwareReg()), toVec(src));
    } else if (fpuRoundingMode == ROUND_Down) {
        compiler.frintm(asmjit::a64::Vec::make_v64(src->hardwareReg()), toVec(src));
    } else if (fpuRoundingMode == ROUND_Up) {
        compiler.frintp(asmjit::a64::Vec::make_v64(src->hardwareReg()), toVec(src));
    } else if (fpuRoundingMode == ROUND_Chop) {
        compiler.frintz(asmjit::a64::Vec::make_v64(src->hardwareReg()), toVec(src));
    }
}

void JitArmV8CodeGen::storeFpuReg(FPURegPtr reg, RegPtr rm, RegPtr sib, DynFpuWidth width) {
    if (width == DYN_FPU_64_BIT) {
        compiler.str(toVec(reg), Mem(R64(rm->hardwareReg()), R64(sib->hardwareReg())));
    } else {
        compiler.str(toVec32(reg), Mem(R64(rm->hardwareReg()), R64(sib->hardwareReg())));
    }
}

void JitArmV8CodeGen::loadFpuReg(FPURegPtr reg, RegPtr rm, RegPtr sib, DynFpuWidth width) {
    if (width == DYN_FPU_64_BIT) {
        compiler.ldr(toVec(reg), Mem(R64(rm->hardwareReg()), R64(sib->hardwareReg())));
    } else {
        compiler.ldr(toVec32(reg), Mem(R64(rm->hardwareReg()), R64(sib->hardwareReg())));
    }
}

void JitArmV8CodeGen::fpuRegExtend32To64(FPURegPtr dst, FPURegPtr src) {
    compiler.fcvt(toVec(dst), toVec32(src));
}

void JitArmV8CodeGen::fpuReg64To32(FPURegPtr dst, FPURegPtr src) {
    compiler.fcvt(toVec32(dst), toVec(src));
    //compiler.fcvtn(asmjit::a64::Vec::make_v64_with_element_type(asmjit::a64::VecElementType::kS, dst->hardwareReg()), asmjit::a64::Vec::make_v128_with_element_type(asmjit::a64::VecElementType::kD, src->hardwareReg()));
}

void JitArmV8CodeGen::loadFpuRegFromInt(FPURegPtr reg, RegPtr rm, RegPtr sib) {
    RegPtr tmp = getTmpReg();
    read(JitWidth::b32, tmp, rm, sib, 0, 0);
    compiler.scvtf(toVec(reg), R32(tmp->hardwareReg())); // convert int64 to double
}

void JitArmV8CodeGen::regToFpuReg(FPURegPtr dst, RegPtr src) {
    compiler.scvtf(toVec(dst), R32(src->hardwareReg()));
}

#ifdef BOXEDWINE_64
void JitArmV8CodeGen::regToFpuReg64(FPURegPtr dst, RegPtr src) {
    compiler.scvtf(toVec(dst), R64(src->hardwareReg()));
}
#endif
void JitArmV8CodeGen::fpuAdd(FPURegPtr dst, FPURegPtr src) {
    compiler.fadd(toVec(dst), toVec(dst), toVec(src));
}

void JitArmV8CodeGen::fpuMul(FPURegPtr dst, FPURegPtr src) {
    compiler.fmul(toVec(dst), toVec(dst), toVec(src));
}

void JitArmV8CodeGen::fpuSub(FPURegPtr dst, FPURegPtr src) {
    compiler.fsub(toVec(dst), toVec(dst), toVec(src));
}

void JitArmV8CodeGen::fpuDiv(FPURegPtr dst, FPURegPtr src) {
    compiler.fdiv(toVec(dst), toVec(dst), toVec(src));
}

void JitArmV8CodeGen::fpuXor(FPURegPtr dst, FPURegPtr src) {
    compiler.eor(toVec(dst), toVec(dst), toVec(src));
}

void JitArmV8CodeGen::fpuAnd(FPURegPtr dst, FPURegPtr src) {
    compiler.and_(toVec(dst), toVec(dst), toVec(src));
}

void JitArmV8CodeGen::fpuSqrt(FPURegPtr dst, FPURegPtr src) {
    compiler.fsqrt(toVec(dst), toVec(src));
}

void JitArmV8CodeGen::IfEqual() {
    Label label = compiler.new_label();
    ifLabels.push_back(label);
    compiler.b_ne(label);
}

void JitArmV8CodeGen::IfGreater() {
    Label label = compiler.new_label();
    ifLabels.push_back(label);
    compiler.b_mi(label);
}

void JitArmV8CodeGen::IfLessThanOrEqual() {
    Label label = compiler.new_label();
    ifLabels.push_back(label);
    compiler.b_cs(label);
}

void JitArmV8CodeGen::If_v() {
    Label label = compiler.new_label();
    ifLabels.push_back(label);
    compiler.b_vc(label);
}

void JitArmV8CodeGen::fcompare(FPURegPtr fpuReg1, FPURegPtr fpuReg2, RegPtr ordTags, const std::function<void()>& pfnEqual, const std::function<void()>& pfnLessThan, const std::function<void()>& pfnGreaterThan, const std::function<void()>& pfnInvalid) {
    subValue(JitWidth::b8, ordTags, TAG_Empty);
    IfNot(JitWidth::b8, ordTags); {
        pfnInvalid();
    } StartElse(); {
        compiler.fcmp(toVec(fpuReg2), toVec(fpuReg1));
        If_v(); {
            pfnInvalid();
        } StartElse(); {
            IfEqual(); {
                pfnEqual();
            } StartElse(); {
                IfLessThanOrEqual(); {
                    pfnLessThan();
                } StartElse(); {
                    pfnGreaterThan();
                } EndIf();
            } EndIf();
        } EndIf();
    } EndIf();
}

U32 JitArmV8CodeGen::markBufferLocation() {
    asmjit::Label label = compiler.new_label();
    compiler.bind(label);
    labels.push_back(label);
    return (U32)labels.size()-1;
}

U32 JitArmV8CodeGen::getBufferLocation(U32 id) {
    return (U32)code.label_offset(labels.at(id));
}

U32 JitArmV8CodeGen::getBufferSize() {
    code.flatten();
    return (U32)code.code_size();
}

void JitArmV8CodeGen::copyBuffer(U8* dst, U32 size) {
    code.copy_flattened_data(dst, size);
}


U32 JitArmV8CodeGen::getIfJumpSize() {
    return (U32)ifLabels.size();
}

void JitArmV8CodeGen::JumpInBlock(U32 address) {    
    Label label;
    if (!opLabels.get(address, label)) {
        label = compiler.new_label();
        opLabels.set(address, label);
    }
    compiler.b(label);
}

void JitArmV8CodeGen::StartElse() {
    Label label = compiler.new_label();
    compiler.b(label);
    compiler.bind(ifLabels.back());
    ifLabels.pop_back();
    ifLabels.push_back(label);
}

void JitArmV8CodeGen::EndIf() {
    compiler.bind(ifLabels.back());
    ifLabels.pop_back();
}
/*
void JitArmV8CodeGen::dynamic_rdtsc(DecodedOp* op) {
    x86.rdtsc();
    getReg(0, -1, false); // will store EAX
    getReg(2, -1, false); // will store EDX
}

void JitArmV8CodeGen::updateHardwareFlags(U32 flags) {
    fillFlags();

    if (flags == CF) {
        x86.bt(X86Asm::Mem32(HOST_CPU, offsetof(CPU, flags)), 0);
        return;
    }
    bool eaxPushed = false;
    RegPtr reg;

#ifdef BOXEDWINE_64
    if (!isTmp[0]) {
        x86.push(RN(0));
        eaxPushed = true;
        reg = getReg(0);
    } else
#endif
    {
        if (!isTmpRegAvailable()) {
            x86.push(RN(0));
            eaxPushed = true;
            regUsed2[0] = false;
        }
        reg = getTmpRegForCallResult();
    }

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
        if (isTmp[0]) {
            regUsed2[0] = true;
        }
    }
}
*/
/*
void JitArmV8CodeGen::dynamic_cmpxchg8b_lock(DecodedOp* op) {
    JitCodeGen::write(JitWidth::b64, calculateEaa(op), nullptr, [op, this](RegPtr addressReg, RegPtr offsetReg) {
        if (currentOp->getNeededFlagsAfter(PF | SF | AF | CF | OF)) { // The ZF flag is set if the destination operand and EDX:EAX are equal; otherwise it is cleared. The CF, PF, AF, SF, and OF flags are unaffected.
            updateHardwareFlags(PF | SF | AF | CF | OF);
        }
        writeCache();
        this->x86.mov(RN(5), RN(addressReg->hardwareReg()));
        this->x86.mov(RN(6), RN(offsetReg->hardwareReg()));
        for (int i = 0; i < 4; i++) {
            x86.mov(R32(i), X86Asm::Mem32(HOST_CPU, CPU::offsetofReg32(i)));
        }
        this->x86.lock();
        this->x86.cmpxchg8b(X86Asm::Mem64(RN(6), RN(5)));
        for (int i = 0; i < 4; i++) {
            x86.mov(X86Asm::Mem32(HOST_CPU, CPU::offsetofReg32(i)), R32(i));
        }
        loadCache();
        updateFlagsIfNecessary();
    });
}
void JitArmV8CodeGen::dynamic_cmpxchge32r32_lock(DecodedOp* op) {
    JitCodeGen::write(JitWidth::b32, calculateEaa(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        RegPtr eax = getReg(0, 0);
        if (eax->hardwareReg() != 0) {
            kpanic("JitArmV8CodeGen::dynamic_cmpxchge32r32_lock");
        }
        RegPtr reg;

        if (op->reg == 0) {
            reg = eax;
        } else {
            reg = getReadOnlyReg(op->reg);
        }
        this->x86.lock();
        this->x86.cmpxchg(X86Asm::Mem32(RN(address->hardwareReg()), RN(offset->hardwareReg())), R32(reg->hardwareReg()));
        address = nullptr;
        offset = nullptr;
        updateFlagsIfNecessary();
    });
}
void JitArmV8CodeGen::dynamic_cmpxchge16r16_lock(DecodedOp* op) {
    JitCodeGen::write(JitWidth::b16, calculateEaa(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        RegPtr eax = getReg(0, 0);
        if (eax->hardwareReg() != 0) {
            kpanic("JitArmV8CodeGen::dynamic_cmpxchge32r32_lock");
        }
        RegPtr reg;

        if (op->reg == 0) {
            reg = eax;
        } else {
            reg = getReadOnlyReg(op->reg);
        }
        this->x86.lock();
        this->x86.cmpxchg(X86Asm::Mem16(RN(address->hardwareReg()), RN(offset->hardwareReg())), R16(reg->hardwareReg()));
        address = nullptr;
        offset = nullptr;
        updateFlagsIfNecessary();     
    });
}
void JitArmV8CodeGen::dynamic_cmpxchge8r8_lock(DecodedOp* op) {
    JitCodeGen::write(JitWidth::b8, calculateEaa(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        RegPtr eax = getReg(0, 0);
        if (eax->hardwareReg() != 0) {
            kpanic("JitArmV8CodeGen::dynamic_cmpxchge32r32_lock");
        }
        RegPtr reg;

        if (op->reg == 0) {
            reg = eax;
        } else {
            reg = getReadOnlyReg8(op->reg);
        }
        this->x86.lock();
        this->x86.cmpxchg(X86Asm::Mem8(RN(address->hardwareReg()), RN(offset->hardwareReg())), R8(get8bitReg(reg)));
        address = nullptr;
        offset = nullptr;
        updateFlagsIfNecessary();
    });
}

void JitArmV8CodeGen::dynamic_xchge32r32_lock(DecodedOp* op) {
    JitCodeGen::write(JitWidth::b32, calculateEaa(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        RegPtr reg = getReg(op->reg);
        this->x86.lock();
        this->x86.xchg(R32(reg->hardwareReg()), X86Asm::Mem32(RN(address->hardwareReg()), RN(offset->hardwareReg())));
    });
}

void JitArmV8CodeGen::dynamic_xchge16r16_lock(DecodedOp* op) {
    JitCodeGen::write(JitWidth::b16, calculateEaa(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        RegPtr reg = getReg(op->reg);
        this->x86.lock();
        this->x86.xchg(R16(reg->hardwareReg()), X86Asm::Mem16(RN(address->hardwareReg()), RN(offset->hardwareReg())));
    });
}

void JitArmV8CodeGen::dynamic_xchge8r8_lock(DecodedOp* op) {
    JitCodeGen::write(JitWidth::b8, calculateEaa(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        RegPtr reg = getReg8(op->reg);
        this->x86.lock();
        this->x86.xchg(R8(get8bitReg(reg)), X86Asm::Mem8(R32(address->hardwareReg()), R32(offset->hardwareReg())));        
    });
}

void JitArmV8CodeGen::dynamic_arithE32R32_lock(DecodedOp* op, std::function<void(RegPtr dest, RegPtr address, RegPtr offset)> callback, bool writeReg) {
    JitCodeGen::write(JitWidth::b32, calculateEaa(op), nullptr, [writeReg, op, callback, this](RegPtr address, RegPtr offset) {
        RegPtr reg;
        
        if (writeReg) {
            reg = getReg(op->reg);
        } else {
            reg = getReadOnlyReg(op->reg);
        }
        callback(reg, address, offset);        
        updateFlagsIfNecessary();
    });
}

void JitArmV8CodeGen::dynamic_arithE16R16_lock(DecodedOp* op, std::function<void(RegPtr dest, RegPtr address, RegPtr offset)> callback, bool writeReg) {
    JitCodeGen::write(JitWidth::b16, calculateEaa(op), nullptr, [writeReg, op, callback, this](RegPtr address, RegPtr offset) {
        RegPtr reg;

        if (writeReg) {
            reg = getReg(op->reg);
        } else {
            reg = getReadOnlyReg(op->reg);
        }
        callback(reg, address, offset);        
        updateFlagsIfNecessary();
    });
}
void JitArmV8CodeGen::dynamic_arithE8R8_lock(DecodedOp* op, std::function<void(RegPtr dest, RegPtr address, RegPtr offset)> callback, bool writeReg) {
    JitCodeGen::write(JitWidth::b8, calculateEaa(op), nullptr, [writeReg, op, callback, this](RegPtr address, RegPtr offset) {
        RegPtr reg;

        if (writeReg) {
            reg = getReg8(op->reg);
        } else {
            reg = getReadOnlyReg8(op->reg);
        }
        callback(reg, address, offset);        
        updateFlagsIfNecessary();
    });
}

void JitArmV8CodeGen::dynamic_arithE32_lock(DecodedOp* op, std::function<void(RegPtr address, RegPtr offset)> callback) {
    JitCodeGen::write(JitWidth::b32, calculateEaa(op), nullptr, [op, callback, this](RegPtr address, RegPtr offset) {
        callback(address, offset);
        updateFlagsIfNecessary();
    });
}

void JitArmV8CodeGen::dynamic_arithE16_lock(DecodedOp* op, std::function<void(RegPtr address, RegPtr offset)> callback) {
    JitCodeGen::write(JitWidth::b16, calculateEaa(op), nullptr, [op, callback, this](RegPtr address, RegPtr offset) {
        callback(address, offset);
        updateFlagsIfNecessary();
    });
}
void JitArmV8CodeGen::dynamic_arithE8_lock(DecodedOp* op, std::function<void(RegPtr address, RegPtr offset)> callback) {
    JitCodeGen::write(JitWidth::b8, calculateEaa(op), nullptr, [op, callback, this](RegPtr address, RegPtr offset) {
        callback(address, offset);
        updateFlagsIfNecessary();
    });
}

void JitArmV8CodeGen::dynamic_xaddr32e32_lock(DecodedOp* op) {
    dynamic_arithE32R32_lock(op, [this](RegPtr dest, RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.xadd(X86Asm::Mem32(RN(addressReg->hardwareReg()), RN(offsetReg->hardwareReg())), R32(dest->hardwareReg()));
    }, true);
}

void JitArmV8CodeGen::dynamic_xaddr16e16_lock(DecodedOp* op) {
    dynamic_arithE16R16_lock(op, [this](RegPtr dest, RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.xadd(X86Asm::Mem16(RN(addressReg->hardwareReg()), RN(offsetReg->hardwareReg())), R16(dest->hardwareReg()));
    }, true);
}
void JitArmV8CodeGen::dynamic_xaddr8e8_lock(DecodedOp* op) {
    dynamic_arithE8R8_lock(op, [this](RegPtr dest, RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.xadd(X86Asm::Mem8(RN(addressReg->hardwareReg()), RN(offsetReg->hardwareReg())), R8(get8bitReg(dest)));
    }, true);
}

void JitArmV8CodeGen::dynamic_adde32r32_lock(DecodedOp* op) {
    dynamic_arithE32R32_lock(op, [this](RegPtr dest, RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.add(X86Asm::Mem32(RN(addressReg->hardwareReg()), RN(offsetReg->hardwareReg())), R32(dest->hardwareReg()));
    });
}
void JitArmV8CodeGen::dynamic_adde16r16_lock(DecodedOp* op) {
    dynamic_arithE16R16_lock(op, [this](RegPtr dest, RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.add(X86Asm::Mem16(RN(addressReg->hardwareReg()), RN(offsetReg->hardwareReg())), R16(dest->hardwareReg()));
    });
}
void JitArmV8CodeGen::dynamic_adde8r8_lock(DecodedOp* op) {
    dynamic_arithE8R8_lock(op, [this](RegPtr dest, RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.add(X86Asm::Mem8(RN(addressReg->hardwareReg()), RN(offsetReg->hardwareReg())), R8(get8bitReg(dest)));
    });
}
void JitArmV8CodeGen::dynamic_add32_mem_lock(DecodedOp* op) {
    dynamic_arithE32_lock(op, [op, this](RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.add(X86Asm::Mem32(RN(addressReg->hardwareReg()), RN(offsetReg->hardwareReg())), op->imm);
    });
}
void JitArmV8CodeGen::dynamic_add16_mem_lock(DecodedOp* op) {
    dynamic_arithE16_lock(op, [op, this](RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.add(X86Asm::Mem16(RN(addressReg->hardwareReg()), RN(offsetReg->hardwareReg())), (U16)op->imm);
    });
}
void JitArmV8CodeGen::dynamic_add8_mem_lock(DecodedOp* op) {
    dynamic_arithE8_lock(op, [op, this](RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.add(X86Asm::Mem8(RN(addressReg->hardwareReg()), RN(offsetReg->hardwareReg())), op->imm);
    });
}

void JitArmV8CodeGen::dynamic_sube32r32_lock(DecodedOp* op) {
    dynamic_arithE32R32_lock(op, [this](RegPtr dest, RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.sub(X86Asm::Mem32(RN(addressReg->hardwareReg()), RN(offsetReg->hardwareReg())), R32(dest->hardwareReg()));
    });
}
void JitArmV8CodeGen::dynamic_sube16r16_lock(DecodedOp* op) {
    dynamic_arithE16R16_lock(op, [this](RegPtr dest, RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.sub(X86Asm::Mem16(RN(addressReg->hardwareReg()), RN(offsetReg->hardwareReg())), R16(dest->hardwareReg()));
    });
}
void JitArmV8CodeGen::dynamic_sube8r8_lock(DecodedOp* op) {
    dynamic_arithE8R8_lock(op, [this](RegPtr dest, RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.sub(X86Asm::Mem8(RN(addressReg->hardwareReg()), RN(offsetReg->hardwareReg())), R8(get8bitReg(dest)));
    });
}
void JitArmV8CodeGen::dynamic_sub32_mem_lock(DecodedOp* op) {
    dynamic_arithE32_lock(op, [op, this](RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.sub(X86Asm::Mem32(RN(addressReg->hardwareReg()), RN(offsetReg->hardwareReg())), op->imm);
    });
}
void JitArmV8CodeGen::dynamic_sub16_mem_lock(DecodedOp* op) {
    dynamic_arithE16_lock(op, [op, this](RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.sub(X86Asm::Mem16(RN(addressReg->hardwareReg()), RN(offsetReg->hardwareReg())), (U16)op->imm);
    });
}
void JitArmV8CodeGen::dynamic_sub8_mem_lock(DecodedOp* op) {
    dynamic_arithE8_lock(op, [op, this](RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.sub(X86Asm::Mem8(RN(addressReg->hardwareReg()), RN(offsetReg->hardwareReg())), (U8)op->imm);
    });
}
void JitArmV8CodeGen::dynamic_ore32r32_lock(DecodedOp* op) {
    dynamic_arithE32R32_lock(op, [this](RegPtr dest, RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.or_(X86Asm::Mem32(RN(addressReg->hardwareReg()), RN(offsetReg->hardwareReg())), R32(dest->hardwareReg()));
    });
}
void JitArmV8CodeGen::dynamic_ore16r16_lock(DecodedOp* op) {
    dynamic_arithE16R16_lock(op, [this](RegPtr dest, RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.or_(X86Asm::Mem16(RN(addressReg->hardwareReg()), RN(offsetReg->hardwareReg())), R16(dest->hardwareReg()));
    });
}
void JitArmV8CodeGen::dynamic_ore8r8_lock(DecodedOp* op) {
    dynamic_arithE8R8_lock(op, [this](RegPtr dest, RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.or_(X86Asm::Mem8(RN(addressReg->hardwareReg()), RN(offsetReg->hardwareReg())), R8(get8bitReg(dest)));
    });
}
void JitArmV8CodeGen::dynamic_or32_mem_lock(DecodedOp* op) {
    dynamic_arithE32_lock(op, [op, this](RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.or_(X86Asm::Mem32(RN(addressReg->hardwareReg()), RN(offsetReg->hardwareReg())), op->imm);
    });
}
void JitArmV8CodeGen::dynamic_or16_mem_lock(DecodedOp* op) {
    dynamic_arithE16_lock(op, [op, this](RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.or_(X86Asm::Mem16(RN(addressReg->hardwareReg()), RN(offsetReg->hardwareReg())), (U16)op->imm);
    });
}
void JitArmV8CodeGen::dynamic_or8_mem_lock(DecodedOp* op) {
    dynamic_arithE8_lock(op, [op, this](RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.or_(X86Asm::Mem8(RN(addressReg->hardwareReg()), RN(offsetReg->hardwareReg())), (U8)op->imm);
    });
}
void JitArmV8CodeGen::dynamic_ande32r32_lock(DecodedOp* op) {
    dynamic_arithE32R32_lock(op, [this](RegPtr dest, RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.and_(X86Asm::Mem32(RN(addressReg->hardwareReg()), RN(offsetReg->hardwareReg())), R32(dest->hardwareReg()));
    });
}
void JitArmV8CodeGen::dynamic_ande16r16_lock(DecodedOp* op) {
    dynamic_arithE16R16_lock(op, [this](RegPtr dest, RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.and_(X86Asm::Mem16(RN(addressReg->hardwareReg()), RN(offsetReg->hardwareReg())), R16(dest->hardwareReg()));
    });
}
void JitArmV8CodeGen::dynamic_ande8r8_lock(DecodedOp* op) {
    dynamic_arithE8R8_lock(op, [this](RegPtr dest, RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.and_(X86Asm::Mem8(RN(addressReg->hardwareReg()), RN(offsetReg->hardwareReg())), R8(get8bitReg(dest)));
    });
}
void JitArmV8CodeGen::dynamic_and32_mem_lock(DecodedOp* op) {
    dynamic_arithE32_lock(op, [op, this](RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.and_(X86Asm::Mem32(RN(addressReg->hardwareReg()), RN(offsetReg->hardwareReg())), op->imm);
        });
}
void JitArmV8CodeGen::dynamic_and16_mem_lock(DecodedOp* op) {
    dynamic_arithE16_lock(op, [op, this](RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.and_(X86Asm::Mem16(RN(addressReg->hardwareReg()), RN(offsetReg->hardwareReg())), (U16)op->imm);
    });
}
void JitArmV8CodeGen::dynamic_and8_mem_lock(DecodedOp* op) {
    dynamic_arithE8_lock(op, [op, this](RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.and_(X86Asm::Mem8(RN(addressReg->hardwareReg()), RN(offsetReg->hardwareReg())), (U8)op->imm);
    });
}
void JitArmV8CodeGen::dynamic_xore32r32_lock(DecodedOp* op) {
    dynamic_arithE32R32_lock(op, [this](RegPtr dest, RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.xor_(X86Asm::Mem32(RN(addressReg->hardwareReg()), RN(offsetReg->hardwareReg())), R32(dest->hardwareReg()));
    });
}
void JitArmV8CodeGen::dynamic_xore16r16_lock(DecodedOp* op) {
    dynamic_arithE16R16_lock(op, [this](RegPtr dest, RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.xor_(X86Asm::Mem16(RN(addressReg->hardwareReg()), RN(offsetReg->hardwareReg())), R16(dest->hardwareReg()));
    });
}
void JitArmV8CodeGen::dynamic_xore8r8_lock(DecodedOp* op) {
    dynamic_arithE8R8_lock(op, [this](RegPtr dest, RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.xor_(X86Asm::Mem8(RN(addressReg->hardwareReg()), RN(offsetReg->hardwareReg())), R8(get8bitReg(dest)));
    });
}
void JitArmV8CodeGen::dynamic_xor32_mem_lock(DecodedOp* op) {
    dynamic_arithE32_lock(op, [op, this](RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.xor_(X86Asm::Mem32(RN(addressReg->hardwareReg()), RN(offsetReg->hardwareReg())), op->imm);
    });
}
void JitArmV8CodeGen::dynamic_xor16_mem_lock(DecodedOp* op) {
    dynamic_arithE16_lock(op, [op, this](RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.xor_(X86Asm::Mem16(RN(addressReg->hardwareReg()), RN(offsetReg->hardwareReg())), (U16)op->imm);
    });
}
void JitArmV8CodeGen::dynamic_xor8_mem_lock(DecodedOp* op) {
    dynamic_arithE8_lock(op, [op, this](RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.xor_(X86Asm::Mem8(RN(addressReg->hardwareReg()), RN(offsetReg->hardwareReg())), (U8)op->imm);
    });
}
void JitArmV8CodeGen::dynamic_inc32_mem32_lock(DecodedOp* op) {
    dynamic_arithE32_lock(op, [op, this](RegPtr addressReg, RegPtr offsetReg) {
        if (op->getNeededFlagsAfter(CF)) {
            updateHardwareFlags(CF);
        }
        this->x86.lock();
        this->x86.inc(X86Asm::Mem32(RN(addressReg->hardwareReg()), RN(offsetReg->hardwareReg())));
    });
}
void JitArmV8CodeGen::dynamic_inc16_mem16_lock(DecodedOp* op) {
    dynamic_arithE16_lock(op, [op, this](RegPtr addressReg, RegPtr offsetReg) {
        if (currentOp->getNeededFlagsAfter(CF)) {
            updateHardwareFlags(CF);
        }
        this->x86.lock();
        this->x86.inc(X86Asm::Mem16(RN(addressReg->hardwareReg()), RN(offsetReg->hardwareReg())));
    });
}
void JitArmV8CodeGen::dynamic_inc8_mem8_lock(DecodedOp* op) {
    dynamic_arithE8_lock(op, [op, this](RegPtr addressReg, RegPtr offsetReg) {
        if (currentOp->getNeededFlagsAfter(CF)) {
            updateHardwareFlags(CF);
        }
        this->x86.lock();
        this->x86.inc(X86Asm::Mem8(RN(addressReg->hardwareReg()), RN(offsetReg->hardwareReg())));
    });
}
void JitArmV8CodeGen::dynamic_dec32_mem32_lock(DecodedOp* op) {
    dynamic_arithE32_lock(op, [op, this](RegPtr addressReg, RegPtr offsetReg) {
        if (currentOp->getNeededFlagsAfter(CF)) {
            updateHardwareFlags(CF);
        }
        this->x86.lock();
        this->x86.dec(X86Asm::Mem32(RN(addressReg->hardwareReg()), RN(offsetReg->hardwareReg())));
    });
}
void JitArmV8CodeGen::dynamic_dec16_mem16_lock(DecodedOp* op) {
    dynamic_arithE16_lock(op, [op, this](RegPtr addressReg, RegPtr offsetReg) {
        if (currentOp->getNeededFlagsAfter(CF)) {
            updateHardwareFlags(CF);
        }
        this->x86.lock();
        this->x86.dec(X86Asm::Mem16(RN(addressReg->hardwareReg()), RN(offsetReg->hardwareReg())));
    });
}
void JitArmV8CodeGen::dynamic_dec8_mem8_lock(DecodedOp* op) {
    dynamic_arithE8_lock(op, [op, this](RegPtr addressReg, RegPtr offsetReg) {
        if (currentOp->getNeededFlagsAfter(CF)) {
            updateHardwareFlags(CF);
        }
        this->x86.lock();
        this->x86.dec(X86Asm::Mem8(RN(addressReg->hardwareReg()), RN(offsetReg->hardwareReg())));
    });
}

void JitArmV8CodeGen::dynamic_note32_lock(DecodedOp* op) {
    dynamic_arithE32_lock(op, [op, this](RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.not_(X86Asm::Mem32(RN(addressReg->hardwareReg()), RN(offsetReg->hardwareReg())));
    });
}

void JitArmV8CodeGen::dynamic_note16_lock(DecodedOp* op) {
    dynamic_arithE16_lock(op, [op, this](RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.not_(X86Asm::Mem16(RN(addressReg->hardwareReg()), RN(offsetReg->hardwareReg())));
    });
}

void JitArmV8CodeGen::dynamic_note8_lock(DecodedOp* op) {
    dynamic_arithE8_lock(op, [op, this](RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.not_(X86Asm::Mem8(RN(addressReg->hardwareReg()), RN(offsetReg->hardwareReg())));
    });
}

void JitArmV8CodeGen::dynamic_nege32_lock(DecodedOp* op) {
    dynamic_arithE32_lock(op, [op, this](RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.neg(X86Asm::Mem32(RN(addressReg->hardwareReg()), RN(offsetReg->hardwareReg())));
    });
}

void JitArmV8CodeGen::dynamic_nege16_lock(DecodedOp* op) {
    dynamic_arithE16_lock(op, [op, this](RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.neg(X86Asm::Mem16(RN(addressReg->hardwareReg()), RN(offsetReg->hardwareReg())));
    });
}

void JitArmV8CodeGen::dynamic_nege8_lock(DecodedOp* op) {
    dynamic_arithE8_lock(op, [op, this](RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.neg(X86Asm::Mem8(RN(addressReg->hardwareReg()), RN(offsetReg->hardwareReg())));
    });
}

void JitArmV8CodeGen::dynamic_btse32_lock(DecodedOp* op) {
    JitCodeGen::write(JitWidth::b32, calculateEaa(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        // imm is the mask, not the bitshift
        U8 imm = std::countr_zero(op->imm);
        this->x86.lock();
        this->x86.bts(X86Asm::Mem32(RN(address->hardwareReg()), RN(offset->hardwareReg())), (U8)imm);
        updateFlagsIfNecessary();
    });
}

void JitArmV8CodeGen::dynamic_btse16_lock(DecodedOp* op) {
    JitCodeGen::write(JitWidth::b16, calculateEaa(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        // imm is the mask, not the bitshift
        U8 imm = std::countr_zero(op->imm);
        this->x86.lock();
        this->x86.bts(X86Asm::Mem16(RN(address->hardwareReg()), RN(offset->hardwareReg())), (U8)imm);
        updateFlagsIfNecessary();
    });
}

void JitArmV8CodeGen::dynamic_btse32r32_lock(DecodedOp* op) {
    JitCodeGen::write(JitWidth::b32, calculateEffectiveEaa32(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        RegPtr reg = getTmpReg(op->reg);
        andValue(JitWidth::b32, reg, 0x1f);
        this->x86.lock();
        this->x86.bts(X86Asm::Mem32(RN(address->hardwareReg()), RN(offset->hardwareReg())), R32(reg->hardwareReg()));
        updateFlagsIfNecessary();
    });
}

void JitArmV8CodeGen::dynamic_btse16r16_lock(DecodedOp* op) {
    JitCodeGen::write(JitWidth::b16, calculateEffectiveEaa16(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        RegPtr reg = getTmpReg(op->reg);
        andValue(JitWidth::b16, reg, 0xf);
        this->x86.lock();
        this->x86.bts(X86Asm::Mem16(RN(address->hardwareReg()), RN(offset->hardwareReg())), R16(reg->hardwareReg()));
        updateFlagsIfNecessary();
    });
}

void JitArmV8CodeGen::dynamic_btre32_lock(DecodedOp* op) {
    JitCodeGen::write(JitWidth::b32, calculateEaa(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        // imm is the mask, not the bitshift
        U8 imm = std::countr_zero(op->imm);
        this->x86.lock();
        this->x86.btr(X86Asm::Mem32(RN(address->hardwareReg()), RN(offset->hardwareReg())), (U8)imm);
        updateFlagsIfNecessary();
    });
}
void JitArmV8CodeGen::dynamic_btre16_lock(DecodedOp* op) {
    JitCodeGen::write(JitWidth::b16, calculateEaa(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        // imm is the mask, not the bitshift
        U8 imm = std::countr_zero(op->imm);
        this->x86.lock();
        this->x86.btr(X86Asm::Mem16(RN(address->hardwareReg()), RN(offset->hardwareReg())), (U8)imm);
        updateFlagsIfNecessary();
    });
}
void JitArmV8CodeGen::dynamic_btre32r32_lock(DecodedOp* op) {
    JitCodeGen::write(JitWidth::b32, calculateEffectiveEaa32(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        RegPtr reg = getTmpReg(op->reg);
        andValue(JitWidth::b32, reg, 0x1f);
        this->x86.lock();
        this->x86.btr(X86Asm::Mem32(RN(address->hardwareReg()), RN(offset->hardwareReg())), R32(reg->hardwareReg()));
        updateFlagsIfNecessary();
    });
}
void JitArmV8CodeGen::dynamic_btre16r16_lock(DecodedOp* op) {
    JitCodeGen::write(JitWidth::b16, calculateEffectiveEaa16(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        RegPtr reg = getTmpReg(op->reg);
        andValue(JitWidth::b16, reg, 0xf);
        this->x86.lock();
        this->x86.btr(X86Asm::Mem16(RN(address->hardwareReg()), RN(offset->hardwareReg())), R16(reg->hardwareReg()));
        updateFlagsIfNecessary();
    });
}

void JitArmV8CodeGen::dynamic_btce32_lock(DecodedOp* op) {
    JitCodeGen::write(JitWidth::b32, calculateEaa(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        // imm is the mask, not the bitshift
        U8 imm = std::countr_zero(op->imm);
        this->x86.lock();
        this->x86.btc(X86Asm::Mem32(RN(address->hardwareReg()), RN(offset->hardwareReg())), (U8)imm);
        updateFlagsIfNecessary();
    });
}
void JitArmV8CodeGen::dynamic_btce16_lock(DecodedOp* op) {
void JitArmV8CodeGen::dynamic_btce16_lock(DecodedOp* op) {
    JitCodeGen::write(JitWidth::b16, calculateEaa(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        // imm is the mask, not the bitshift
        U8 imm = std::countr_zero(op->imm);
        this->x86.lock();
        this->x86.btc(X86Asm::Mem16(RN(address->hardwareReg()), RN(offset->hardwareReg())), (U8)imm);
        updateFlagsIfNecessary();
    });
}
void JitArmV8CodeGen::dynamic_btce32r32_lock(DecodedOp* op) {
    JitCodeGen::write(JitWidth::b32, calculateEffectiveEaa32(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        RegPtr reg = getTmpReg(op->reg);
        andValue(JitWidth::b32, reg, 0x1f);
        this->x86.lock();
        this->x86.btc(X86Asm::Mem32(RN(address->hardwareReg()), RN(offset->hardwareReg())), R32(reg->hardwareReg()));
        updateFlagsIfNecessary();
    });
}
void JitArmV8CodeGen::dynamic_btce16r16_lock(DecodedOp* op) {
    JitCodeGen::write(JitWidth::b16, calculateEffectiveEaa16(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        RegPtr reg = getTmpReg(op->reg);
        andValue(JitWidth::b16, reg, 0xf);
        this->x86.lock();
        this->x86.btc(X86Asm::Mem16(RN(address->hardwareReg()), RN(offset->hardwareReg())), R16(reg->hardwareReg()));
        updateFlagsIfNecessary();
    });
}
*/

void writeBlockExitForJIT(U32 eip, U8* buffer) {
    asmjit::JitRuntime rt;
    asmjit::CodeHolder code;
    asmjit::a64::Assembler compiler;

    code.init(rt.environment());
    code.attach(&compiler);

    compiler.blr(xWriteCacheToCPU);
    compiler.mov(R32(xTmp7), eip);
    compiler.str(R32(xTmp7), Mem(xCPU, offsetof(CPU, eip.u32)));
    compiler.mov(R32(xTmp7), xCPU);
    for (int i = 19; i < 31; i++) {
        compiler.ldr(R64(i), Mem(R64(xTmp7), offsetof(CPU, storedRegs) + i * 8));
    }
    compiler.ret(asmjit::a64::x30);

    code.flatten();
    Platform::writeCodeToMemory(buffer, (U32)code.code_size(), [&code, buffer]() {
        code.copy_flattened_data(buffer, code.code_size());
    });
    Platform::clearInstructionCache(buffer, (U32)code.code_size());
    
}

void JitArmV8CodeGen::blockExit(bool syncCache) {
    if (syncCache) {
        compiler.blr(xWriteCacheToCPU);
    }
    compiler.mov(R64(xTmp7), xCPU);
    for (int i = 19; i < 31; i++) {
        compiler.ldr(R64(i), Mem(R64(xTmp7), offsetof(CPU, storedRegs) + i * 8));
    }
    compiler.ret(asmjit::a64::x30);
    //vReadMemMultiple64(12, 31, 4, true);
    //vReadMemMultiple64(8, 31, 4, true);
}

U8* JitArmV8CodeGen::createStartJITCode() {
    for (int i = 19; i < 31; i++) {
        compiler.str(R64(i), Mem(R64(0), offsetof(CPU, storedRegs) + i * 8));
    }
    compiler.mov(R64(29), asmjit::a64::sp); // mov fp, sp
    // only the bottom 64-bits of v8-v15 need to be saved
    //subValue64(31, 31, 32);
    //vWriteMemMultiple64(8, 31, 4, false);
    //subValue64(31, 31, 32);
    //vWriteMemMultiple64(12, 31, 4, false);

    compiler.mov(xCPU, R64(0));
    compiler.ldr(xBranch, Mem(R64(1), offsetof(DecodedOp, pfnJitCode)));
    compiler.mov(xMMU, (U64)&getMemData(KThread::currentThread()->memory)->mmu);
    compiler.mov(xLoadCacheFromCPU, (U64)KThread::currentThread()->process->syncToHost);
    compiler.mov(xWriteCacheToCPU, (U64)KThread::currentThread()->process->syncFromHost);
    loadCacheFromCPU();
    compiler.br(xBranch);

    return createDynamicExecutableMemory();
}

void JitArmV8CodeGen::preCommitJIT() {

}

void JitArmV8CodeGen::patch(U8* begin) {

}

void JitArmV8CodeGen::loadCacheFromCPU() {
    for (int i = 0; i < 8; i++) {
        if (regCache[i] != INVALID_REG) {
            compiler.ldr(R32(regCache[i]), Mem(xCPU, offsetof(CPU, reg[i].u32)));
        }
    }
    compiler.ldr(xFLAGS, createMem(regCPU, offsetof(CPU, flags)));
    compiler.orr(xFLAGS, xFLAGS, 2);
    compiler.ldrb(xFlagsType, createMem(regCPU, offsetof(CPU, lazyFlagType)));

    // addValue64(addressReg, xCPU, (U32)(offsetof(CPU, xmm[0])));
    // vWriteMemMultiple128(xXMM0, addressReg, 4, true);
    // vWriteMemMultiple128(xXMM4, addressReg, 4, false);
}

void JitArmV8CodeGen::writeCacheToCPU() {
    for (int i = 0; i < 8; i++) {
        if (regCache[i] != INVALID_REG) {
            compiler.str(R32(regCache[i]), Mem(xCPU, offsetof(CPU, reg[i].u32)));
        }
    }
    compiler.str(xFLAGS, createMem(regCPU, offsetof(CPU, flags)));
    compiler.strb(xFlagsType, createMem(regCPU, offsetof(CPU, lazyFlagType)));

    //U8 addressReg = getTmpReg();
    //addValue64(addressReg, xCPU, (U32)(offsetof(CPU, xmm[0])));
    //vReadMemMultiple128(xXMM0, addressReg, 4, true);
    //vReadMemMultiple128(xXMM4, addressReg, 4, false);
}

U8* JitArmV8CodeGen::createSyncToHost() {
    loadCacheFromCPU();
    compiler.ret(asmjit::a64::x30);
    return createDynamicExecutableMemory();
}

U8* JitArmV8CodeGen::createSyncFromHost() {
    writeCacheToCPU();
    compiler.ret(asmjit::a64::x30);
    return createDynamicExecutableMemory();
}

JitCodeGen* startNewJIT(CPU* cpu) {
    return new JitArmV8CodeGen(cpu);
}

void startNewJIT(CPU* cpu, U32 address, DecodedOp* op) {
    JitArmV8CodeGen data(cpu);
    data.doJIT(address, op);
}

#endif