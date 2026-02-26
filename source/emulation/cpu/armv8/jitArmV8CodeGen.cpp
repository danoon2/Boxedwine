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

enum class TSOMode {
    Automatic,
    None,
    Barrier,
    FEAT_LRCPC,
    FEAT_LRCPC2,
    FEAT_LRCPC3,
    Hardware
};

static TSOMode tsoMode = TSOMode::Automatic;

#define NUMBER_OF_REGS 31
#define NUMBER_OF_VREGS 32
#define NUMBER_OF_TMPS 9
#define NUMBER_OF_VREG_TMPS 9

static bool isVolitile[] = { true,  true,  true,  true,  true,  true,  true,  true,
                             true,  true,  true,  true,  true,  true,  true,  true,
                             true,  true,  true,  false, false, false, false, false,
                             false, false, false, false, false, false, false, false };

static bool isTmp[] = { false, false, false, false, false, false, false, false,
                        false, false, false, false, true, true, true, false,
                        false,  true,  false,  false,  false,  true, true, true,
                        true, true, false, false, false, false, false, false };

U8 regCache[] = { 0, 3, 1, 2, 4, 5, 6, 7 };
static U8 tmps[] = { 21, 22, 23, 24, 25, 17, 12, 13, 14 };
static U8 vtmps[] = { 16, 17, 18, 19, 20, 21, 22, 23, 24 }; // 8-15 are callee saved, so don't use them
U8 xmmCache[] = { 0, 1, 2, 3, 4, 5, 6, 7 };

#define INVALID_REG 0xff

typedef asmjit::a64::Mem Mem;
typedef asmjit::a64::Shift Shift;
typedef asmjit::a64::ShiftOp ShiftOp;
typedef asmjit::Label Label;
typedef asmjit::a64::Vec Vec;

asmjit::a64::Gp R64(U8 reg) {
    return asmjit::a64::gp64(reg);
}

asmjit::a64::Gp R64(RegPtr reg) {
    return asmjit::a64::gp64(reg->hardwareReg());
}

asmjit::a64::Gp R32(U8 reg) {
    return asmjit::a64::gp32(reg);
}

asmjit::a64::Gp R32(RegPtr reg) {
    return asmjit::a64::gp32(reg->hardwareReg());
}

// if these mappings change, make sure to checkout platformThreads.cpp / syncFromException
// 
// cmpxchg8b / caspal requires eax/edx and ebx/ecx to be next to each other
#define xEAX asmjit::a64::x0
#define wEAX asmjit::a64::w0
#define xEDX asmjit::a64::x1
#define wEDX asmjit::a64::w1
#define xEBX asmjit::a64::x2
#define wEBX asmjit::a64::w2
#define xECX asmjit::a64::x3
#define wECX asmjit::a64::w3
#define xESP 4
#define xEBP 5
#define xESI 6
#define xEDI 7
#define xFLAGS asmjit::a64::w8
#define regFlags 8

// x9 to x15 caller saved
#define xBranch asmjit::a64::x9 
#define wBranch asmjit::a64::w9 
#define regBranch 9
#define xDst asmjit::a64::w10
#define regDst 10

#define xFlagsType asmjit::a64::w11
#define regFlagsType 11

#define xTmp7 asmjit::a64::x12
#define wTmp7 asmjit::a64::w12
#define xTmp8 asmjit::a64::x13
#define wTmp8 asmjit::a64::w13
#define xTmp9 asmjit::a64::x14
#define wTmp9 asmjit::a64::w14
#define regTmp9 14
#define xResult asmjit::a64::w15
#define regResult 15
#define xSrc asmjit::a64::w16
#define regSrc 16
#define xTmp6 17

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
// this improves code density since every emulated read/write needs to compile this address, so instead of 3-4 mov/movk's, it's just a direct br xEmulateSingleOp
// but I diddn't really notice this improving performance
#define xEmulateSingleOp asmjit::a64::x26
#define xWriteCacheToCPU asmjit::a64::x27
#define xLoadCacheFromCPU asmjit::a64::x28
#define xScratch asmjit::a64::x29
#define wScratch asmjit::a64::w29
#define regScratch 29

#define ZERO_EXTEND 1
#define SIGN_EXTEND 2

using MakeSSE = Vec(SSERegPtr reg);

bool enableHardwareTSO();

class JitArmV8CodeGen : public JitSSE, asmjit::ErrorHandler {
public:  
    void handle_error(asmjit::Error err, const char* message, asmjit::BaseEmitter* origin) override {
        kpanic(message);
    }

    JitArmV8CodeGen(CPU* cpu) : JitSSE(cpu) {
        static bool initialized = false;
        if (!initialized) {
            initialized = true;
#ifdef BOXEDWINE_MSVC
            U64 features = get_ID_AA64ISAR0_EL1();
            U64 atomicLevel = ((int64_t)(features << (60 - 20)) >> 60);
            if (atomicLevel >= 1) {
                rt._cpu_features.add(asmjit::CpuFeatures::ARM::kLSE);
            }

            features = get_ID_AA64ISAR1_EL1();
            atomicLevel = ((int64_t)(features << (60 - 20)) >> 60);
            if (atomicLevel >= 1) {
                rt._cpu_features.add(asmjit::CpuFeatures::ARM::kLRCPC);
            }
            if (atomicLevel >= 2) {
                rt._cpu_features.add(asmjit::CpuFeatures::ARM::kLRCPC2);
            }
            if (atomicLevel >= 3) {
                rt._cpu_features.add(asmjit::CpuFeatures::ARM::kLRCPC3);
            }
#endif
            if (tsoMode == TSOMode::Automatic) {
#ifdef __linux__
                if (enableHardwareTSO()) {
                    tsoMode == TSOMode::Hardware;
                } else
#endif
                if (rt.cpu_features().has(asmjit::CpuFeatures::ARM::kLRCPC2)) {
                    tsoMode = TSOMode::FEAT_LRCPC2;
                } else {
                    tsoMode = TSOMode::None;
                }
            }
        }
        code.init(rt.environment());
        code.attach(&compiler);
        code.set_error_handler(this);
    }

    void preOp(DecodedOp* op) override;
    RegPtr getReadOnlyRegInLower(JitWidth width, U8 reg);
    RegPtr getReg(JitWidth width, U8 reg);
    RegPtr getReg(U8 reg, S8 hint = -1, bool load = true) override;
    RegPtr getReg8(U8 reg, bool load = true) override;
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
    RegPtr getTmpIfNotTmp(RegPtr reg);
    void writeEip(RegPtr eip) override;
    void writeEip(U32 eip) override;
    bool isTmpRegAvailable() override;    
    void forceSyncBackIfNotCached(RegPtr reg) override;
    RegPtr getConditionCalculationReg(U32 index) override;

#ifdef BOXEDWINE_MEM_CACHE
    void writeMemCache(JitWidth width, RegPtr addressReg, RegPtr src) override;
    void writeMemCache(JitWidth width, RegPtr addressReg, U32 value) override;
    void writeMemCache(JitWidth width, U32 mem, RegPtr src) override;
    void writeMemCache(JitWidth width, U32 mem, U32 imm) override;
    RegPtr readMemCache(JitWidth width, RegPtr addressReg, RegPtr tmp = nullptr) override;
    RegPtr readMemCache(JitWidth width, U32 mem, RegPtr result = nullptr) override;
    RegPtr readWriteMemCache(JitWidth width, RegPtr addressReg, std::function<void(RegPtr value)> prepareWrite, RegPtr tmp) override;
#endif
    U8 findTmpReg(bool allowInvalidReturn = false);
    void emulateSingleOp() override;
    RegPtr calculateEaa(DecodedOp* op, U32 popEspAmount = 0) override;

    void direct_cmp(JitWidth width, RegPtr left, RegPtr right) override;
    void direct_cmp(JitWidth width, RegPtr left, U32 right) override;
    void direct_test(JitWidth width, RegPtr left, RegPtr right) override;
    void direct_test(JitWidth width, RegPtr left, U32 right) override;
    void direct_jump(JitConditional condition, U32 address) override;
    void direct_cmov(JitWidth width, JitConditional condition, RegPtr dst, RegPtr src) override;
    void direct_setcc(JitConditional condition, RegPtr dst) override;
    bool directDoesAffectFlags(DecodedOp* op) override;
    void onBlockPreCommit(DecodedOp* op) override;
    asmjit::a64::CondCode getCondCode(JitConditional condition);
    bool supportsDirectCondition(JitConditional condition) override;

    void regReg(JitWidth regWidth, RegPtr reg, RegPtr rm, std::function<void(asmjit::a64::Gp dst, asmjit::a64::Gp reg, asmjit::a64::Gp rm)> fn, U32 extend = 0);
    void reg1(JitWidth regWidth, RegPtr reg, std::function<void(asmjit::a64::Gp dst, asmjit::a64::Gp src)> fn);
    void regValue(JitWidth regWidth, RegPtr reg, U32 value, std::function<void(asmjit::a64::Gp dst, asmjit::a64::Gp reg, U32 value)> fn, U32 extend = 0);

    void addReg(JitWidth regWidth, RegPtr reg, RegPtr rm) override;
    void addRegWithDest(JitWidth regWidth, RegPtr dst, RegPtr reg, RegPtr rm) override;
    void addValue(JitWidth regWidth, RegPtr reg, U32 imm) override;
    void addValueWithDest(JitWidth regWidth, RegPtr dst, RegPtr reg, U32 imm) override;
    void orReg(JitWidth regWidth, RegPtr reg, RegPtr rm) override;
    void orValue(JitWidth regWidth, RegPtr reg, U32 imm) override;
    void subReg(JitWidth regWidth, RegPtr reg, RegPtr rm) override;
    void subValue(JitWidth regWidth, RegPtr reg, U32 imm) override;
    void subValueWithDest(JitWidth regWidth, RegPtr dst, RegPtr reg, U32 imm) override;
    void andReg(JitWidth regWidth, RegPtr reg, RegPtr rm) override;
    void andValue(JitWidth regWidth, RegPtr reg, U32 immm) override;
    void andValue64(RegPtr reg, U64 immm) override;
    void andValueWithDest(JitWidth regWidth, RegPtr dst, RegPtr reg, U32 value);
    void xorReg(JitWidth regWidth, RegPtr reg, RegPtr rm) override;
    void xorValue(JitWidth regWidth, RegPtr reg, U32 immm) override;
    void shrReg(JitWidth regWidth, RegPtr reg, RegPtr rm) override;
    void shrValue(JitWidth regWidth, RegPtr reg, U32 immm) override;
    void shrValueWithDest(JitWidth regWidth, RegPtr dst, RegPtr reg, U32 value) override;
    void shlReg(JitWidth regWidth, RegPtr reg, RegPtr rm) override;
    void shlValue(JitWidth regWidth, RegPtr reg, U32 immm) override;
    void shlValueWithDest(JitWidth regWidth, RegPtr dst, RegPtr reg, U32 value) override;
    void sarReg(JitWidth regWidth, RegPtr reg, RegPtr rm) override;
    void sarValue(JitWidth regWidth, RegPtr reg, U32 immm) override;
    void sarValueWithDest(JitWidth regWidth, RegPtr dst, RegPtr reg, U32 immm) override;
    void notReg2(JitWidth regWidth, RegPtr reg) override;
    void negReg2(JitWidth regWidth, RegPtr reg) override;
    void bsfReg(JitWidth regWidth, RegPtr reg, RegPtr rm) override;
    void bsrReg(JitWidth regWidth, RegPtr reg, RegPtr rm) override;
    void rolReg(JitWidth regWidth, RegPtr reg, RegPtr rm) override;
    void rolValue(JitWidth regWidth, RegPtr reg, U32 imm) override;
    void rorReg(JitWidth regWidth, RegPtr reg, RegPtr rm) override;
    void rorValue(JitWidth regWidth, RegPtr reg, U32 imm) override;
    void rclReg(JitWidth regWidth, RegPtr reg, RegPtr rm, RegPtr cf) override;
    void rclValue(JitWidth regWidth, RegPtr reg, U32 imm, RegPtr cf) override;
    void rcrReg(JitWidth regWidth, RegPtr reg, RegPtr rm, RegPtr cf) override;
    void rcrValue(JitWidth regWidth, RegPtr reg, U32 imm, RegPtr cf) override;
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

    void readMMU(RegPtr dest, RegPtr index) override;
    void readMMU(RegPtr dest, U32 index) override;
    virtual void readHost(JitWidth width, MemPtr mem, RegPtr result, bool emlulatedMemory = true) override;
    virtual void writeHost(JitWidth width, MemPtr mem, RegPtr src, bool emlulatedMemory = true) override;
    virtual void writeHost(JitWidth width, MemPtr mem, U32 imm, bool emlulatedMemory = true) override;

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
    void jmpHost(RegPtr reg) override;
    void jmpHost(DYN_PTR_SIZE address) override;
    RegPtr getReadOnlyFlags(RegPtr tmp = nullptr) override;
    void storeLazyFlagType(LazyFlagType flags) override;
    void storeLazyFlagsResult(RegPtr reg) override;
    void storeLazyFlagsDest(RegPtr reg) override;
    void storeLazyFlagsSrc(RegPtr reg) override;
    void storeLazyFlagsSrc(U32 value) override;
    RegPtr getFlagDestReadOnly(RegPtr result = nullptr) override; // passed in result might be returned, but not guaranteed, its available just to help minimize use of temp registers    
    RegPtr getFlagDestTmp(RegPtr result = nullptr) override; // guaranteed to return result in result
    RegPtr getFlagSrcReadOnly(RegPtr result = nullptr) override; // passed in result might be returned, but not guaranteed, its available just to help minimize use of temp registers
    RegPtr getFlagSrcTmp(RegPtr result = nullptr) override; // guaranteed to return result in result
    RegPtr getFlagResultReadOnly(RegPtr result = nullptr) override; // passed in result might be returned, but not guaranteed, its available just to help minimize use of temp registers
    RegPtr getFlagResultTmp(RegPtr result = nullptr) override; // guaranteed to return result in result
    RegPtr getFlagsInTmp(RegPtr reg = nullptr) override;
    void setFlags(RegPtr flags, U32 mask) override;
    RegPtr getLazyFlagType() override;
    RegPtr getLazyFlagTypeInTmp() override;
    void writeFlags(RegPtr flags) override;

    void callHostFunction(void* address, const std::vector<DynParam>& params, bool restoreCache = true, bool saveCache = true) override;
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
    RegPtr fpuRegToInt(FPURegPtr fpuRegSrc, bool truncate, bool is64);

    FPURegPtr getFPUTmp() override;
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
    MMXRegPtr loadMMXFromMem32(U8 index, RegPtr rm, RegPtr sib) override;
    MMXRegPtr loadMMXFromMem64(U8 index, RegPtr rm, RegPtr sib) override;
    void storeMMXToMem32(MMXRegPtr reg, RegPtr rm, RegPtr sib) override;
    void storeMMXToMem64(MMXRegPtr reg, RegPtr rm, RegPtr sib) override;
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

    // SSE    
    SSERegPtr getTmpSSE() override;
    SSERegPtr loadSSEConst(U8 index);
    void IfSseLessThan(SSERegPtr src1, SSERegPtr src2) override;    
    bool isSseRegCached(U8 reg) override;
    void storeCpuXMMReg(SSERegPtr reg, U32 index) override;
    SSERegPtr loadCpuXMMReg(U8 index) override;
    SSERegPtr loadXMMFromMem128(U8 reg, RegPtr rm, RegPtr sib) override;
    SSERegPtr loadXMMFromMem64(U8 reg, RegPtr rm, RegPtr sib) override;
    SSERegPtr loadLowXMMFromMem64(U8 reg, RegPtr rm, RegPtr sib) override;
    SSERegPtr loadHighXMMFromMem64(U8 reg, RegPtr rm, RegPtr sib) override;
    SSERegPtr loadXMMFromMem32(U8 reg, RegPtr rm, RegPtr sib) override;
    void storeXMMToMem128(SSERegPtr reg, RegPtr rm, RegPtr sib) override;
    void storeXMMToMem64(SSERegPtr reg, RegPtr rm, RegPtr sib) override;
    void storeXMMToMem32(SSERegPtr reg, RegPtr rm, RegPtr sib) override;
    void storeHighXMMToMem64(SSERegPtr reg, RegPtr rm, RegPtr sib) override;
    void fmax(SSERegPtr dst, SSERegPtr src1, SSERegPtr src2, MakeSSE vMake);
    void fmin(SSERegPtr dst, SSERegPtr src1, SSERegPtr src2, MakeSSE vMake);
    void cvtps2pi(SSERegPtr dst, SSERegPtr src, bool truncate);
    void cvtsd2si(RegPtr dst, SSERegPtr src, bool truncate);
    void cvtpd2pi(SSERegPtr dst, SSERegPtr src, bool truncate);
    void sseConvertFloatToInt(Vec dst, Vec src, bool truncate);
    void sseConvertFloatToInt(RegPtr dst, Vec src, bool truncate);
    void sseCmp(SSERegPtr dst, SSERegPtr src1, SSERegPtr src2, U8 pred, MakeSSE vMake);
    void comis(SSERegPtr dst, SSERegPtr src, MakeSSE vMake);

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
    void clflush(RegPtr address) override;
    void pause() override;
    void pextrwR32Xmm(RegPtr dst, SSERegPtr src, U32 imm) override;
    void pinsrwXmmR32(SSERegPtr dst, RegPtr src, U32 imm) override;
    void pmovmskbR32Xmm(RegPtr dst, SSERegPtr src) override;

    void dynamic_rdtsc(DecodedOp* op) override;
    void dynamic_arith_lock(JitWidth width, DecodedOp* op, LazyFlagType flagsType, std::function<void(RegPtr src, RegPtr dst, RegPtr address)> atomicCallback, std::function<void(RegPtr result, RegPtr dstMem, RegPtr srcReg)> callback, bool writebackReg = false, bool addCF = false, bool immSrc = false, bool hasSrc = true);
    void dynamic_arith_value_lock(JitWidth width, DecodedOp* op, U32 value, LazyFlagType flagsType, std::function<void(RegPtr src, RegPtr dst, RegPtr address)> atomicCallback, std::function<void(RegPtr result, RegPtr dst, U32 src)> callback);

    /*
    void dynamic_arithE16R16_lock(DecodedOp* op, std::function<void(RegPtr dest, RegPtr address, RegPtr offset)> callback, bool writeReg = false);
    void dynamic_arithE8R8_lock(DecodedOp* op, std::function<void(RegPtr dest, RegPtr address, RegPtr offset)> callback, bool writeReg = false);
    void dynamic_arithE32_lock(DecodedOp* op, std::function<void(RegPtr address, RegPtr offset)> callback);
    void dynamic_arithE16_lock(DecodedOp* op, std::function<void(RegPtr address, RegPtr offset)> callback);
    void dynamic_arithE8_lock(DecodedOp* op, std::function<void(RegPtr address, RegPtr offset)> callback);
*/
    void ldaxr(JitWidth width, asmjit::a64::Gp reg, Mem mem);
    void stlxr(JitWidth width, asmjit::a64::Gp cond, asmjit::a64::Gp reg, Mem mem);
    void casal(JitWidth width, asmjit::a64::Gp src, asmjit::a64::Gp dst, Mem mem);

    void dynamic_cmpxchg8b_lock(DecodedOp* op) override;
    void dynamic_cmpxchg_lock(JitWidth width, DecodedOp* op);
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
    void dynamic_adce32r32_lock(DecodedOp* op) override;
    void dynamic_adce16r16_lock(DecodedOp* op) override;
    void dynamic_adce8r8_lock(DecodedOp* op) override;
    void dynamic_adc32_mem_lock(DecodedOp* op) override;
    void dynamic_adc16_mem_lock(DecodedOp* op) override;
    void dynamic_adc8_mem_lock(DecodedOp* op) override;
    void dynamic_sube32r32_lock(DecodedOp* op) override;
    void dynamic_sube16r16_lock(DecodedOp* op) override;
    void dynamic_sube8r8_lock(DecodedOp* op) override;
    void dynamic_sub32_mem_lock(DecodedOp* op) override;
    void dynamic_sub16_mem_lock(DecodedOp* op) override;
    void dynamic_sub8_mem_lock(DecodedOp* op) override;
    void dynamic_sbbe32r32_lock(DecodedOp* op) override;
    void dynamic_sbbe16r16_lock(DecodedOp* op) override;
    void dynamic_sbbe8r8_lock(DecodedOp* op) override;
    void dynamic_sbb32_mem_lock(DecodedOp* op) override;
    void dynamic_sbb16_mem_lock(DecodedOp* op) override;
    void dynamic_sbb8_mem_lock(DecodedOp* op) override;
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
    /*
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

    U8* createStartJITCode() override;
    U8* createBlockExit(bool syncRegs) override;

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
    void IfLessThan();
    void If_v();

    U8 getMMXReg(MMXRegPtr reg);
    RegPtr getReg8InLowByte(RegPtr reg);
    bool isRegHigh(RegPtr reg);
    Mem createMem(MemPtr mem);
    Mem createMem(RegPtr reg, RegPtr sib, U8 lsl, U32 disp);
    Mem createMem(RegPtr reg, U32 disp);
    Mem createMem(U8 reg, U8 sib, U8 lsl, U32 disp);
    Mem createMem(U8 reg, U32 disp);
    RegPtr loadConst(U64 value);    
    void modValue32(RegPtr dst, RegPtr src, RegPtr value);

    static asmjit::JitRuntime rt;
    asmjit::CodeHolder code;
    asmjit::a64::Assembler compiler;

    std::array<bool, NUMBER_OF_REGS> rUsed{ 0 };
    std::array<bool, NUMBER_OF_VREGS> vUsed{ 0 };
    std::vector<Label> labels;

    std::vector<Label> ifLabels;
    BHashTable<U32, Label> opLabels;
    BHashTable<U32, Label> pendingLabels;
    bool cfInverted = false;
};

asmjit::JitRuntime JitArmV8CodeGen::rt;

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
    pendingLabels.remove(currentEip);
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

bool JitArmV8CodeGen::isSseRegCached(U8 reg) {
    return true;
}

SSERegPtr JitArmV8CodeGen::getTmpSSE() {
    return std::shared_ptr<SSERegInternal>(new SSERegInternal(findTmpXMM(), 0xff), [this](SSERegInternal* p) {
        vUsed[p->hardwareReg()] = false;
        delete p;
    });
}

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

    //compiler.mov(xBranch, (DYN_PTR_SIZE)cpu->thread->process->emulateSingleOp);
    compiler.br(xEmulateSingleOp); // we won't return to here
}

U8 JitArmV8CodeGen::findTmpReg(bool allowInvalidReturn) {
    if (disableTmps) {
        kpanic("JitArmV8CodeGen::findTmpReg disableTmps");
    }
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

RegPtr JitArmV8CodeGen::getConditionCalculationReg(U32 index) {
    U8 tmp = 0xff;
    if (index == 0) {
        tmp = tmps[NUMBER_OF_TMPS - 1];
        if (regUsed[tmp]) {
            kpanic("JitX86CodeGen::JitArmV8CodeGen 0");
        }        
    } else if (index == 1) {
        tmp = tmps[NUMBER_OF_TMPS - 2];
        if (regUsed[tmp]) {
            kpanic("JitX86CodeGen::JitArmV8CodeGen 1");
        }
    } else if (index == 2) {
        tmp = tmps[NUMBER_OF_TMPS - 3];
        if (regUsed[tmp]) {
            kpanic("JitX86CodeGen::JitArmV8CodeGen 2");
        }
    } else {
        kpanic("JitX86CodeGen::JitArmV8CodeGen");
        return nullptr;
    }
    regUsed[tmp] = true;
    return std::shared_ptr<JitReg>(new JitReg(tmp, 0xff), [this](JitReg* p) {
        if (p->isLoaded()) {
            regUsed[p->hardwareReg()] = false;
        }
        delete p;
    });
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
            compiler.mov(R64(result), R64(regCache[reg]));
        } else {
            compiler.ldr(R32(result), Mem(xCPU, CPU::offsetofReg32(reg)));
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
    compiler.ldr(R32(result), Mem(xCPU, CPU::offsetofSegAddress(reg)));
    return result;
}

RegPtr JitArmV8CodeGen::getReadOnlySegValue(U8 reg) {
    RegPtr result = getTmpReg();
    compiler.ldr(R32(result), Mem(xCPU, CPU::offsetofSegValue(reg)));
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

#ifdef BOXEDWINE_MEM_CACHE
RegPtr JitArmV8CodeGen::readWriteMemCache(JitWidth width, RegPtr addressReg, std::function<void(RegPtr value)> prepareWrite, RegPtr tmp) {
#ifdef _DEBUG
    writeCurrentEip(0);
#endif
    RegPtr value = getTmpRegForCallResult();
    shrValueWithDest(JitWidth::b32, tmp, addressReg, K_PAGE_SHIFT);
    compiler.ldr(R64(tmp), createMem(regMMU, tmp->hardwareReg(), 3, K_NUMBER_OF_PAGES * 2 * sizeof(void*)));
    if (!KSystem::canJitUse4KPage && width != JitWidth::b8) {
        RegPtr offsetReg = getTmpReg();
        compiler.and_(R32(offsetReg), R32(addressReg), 0xfff);
        compiler.add(R64(tmp), R64(tmp), R64(addressReg));
        addressReg = nullptr;
        clearMMUPermissionIfSpansPage(width, offsetReg, tmp);
        
        MemPtr mem = createMemPtr(tmp, 0);
        readHost(width, mem, value);
        prepareWrite(value);
        writeHost(width, mem, value);
    } else {
        MemPtr mem = createMemPtr(tmp, addressReg, 0, 0);
        readHost(width, mem, value);
        prepareWrite(value);
        writeHost(width, mem, value);
    }    

    
    return value;
}

void JitArmV8CodeGen::writeMemCache(JitWidth width, RegPtr addressReg, RegPtr src) {
#ifdef _DEBUG
    writeCurrentEip(1);
#endif
    RegPtr tmp = getTmpReg();
    shrValueWithDest(JitWidth::b32, tmp, addressReg, K_PAGE_SHIFT);
    compiler.ldr(R64(tmp), createMem(regMMU, tmp->hardwareReg(), 3, K_NUMBER_OF_PAGES * 2 * sizeof(void*)));

    if (!KSystem::canJitUse4KPage && width != JitWidth::b8) {
        RegPtr offsetReg = getTmpReg();
        compiler.and_(R32(offsetReg), R32(addressReg), K_PAGE_MASK);
        compiler.add(R64(tmp), R64(tmp), R64(addressReg));        
        clearMMUPermissionIfSpansPage(width, offsetReg, tmp);
        writeHost(width, createMemPtr(tmp, 0), src);
    } else {
        writeHost(width, createMemPtr(tmp, addressReg, 0, 0), src);
    }
}

void JitArmV8CodeGen::writeMemCache(JitWidth width, RegPtr addressReg, U32 src) {
#ifdef _DEBUG
    writeCurrentEip(0);
#endif
    RegPtr tmp = getTmpReg();

    shrValueWithDest(JitWidth::b32, tmp, addressReg, K_PAGE_SHIFT);
    compiler.ldr(R64(tmp), createMem(regMMU, tmp->hardwareReg(), 3, K_NUMBER_OF_PAGES * 2 * sizeof(void*)));

    if (!KSystem::canJitUse4KPage && width != JitWidth::b8) {
        RegPtr offsetReg = getTmpReg();
        compiler.and_(R32(offsetReg), R32(addressReg), 0xfff);
        compiler.add(R64(tmp), R64(tmp), R64(addressReg));
        clearMMUPermissionIfSpansPage(width, offsetReg, tmp);
        writeHost(width, createMemPtr(tmp, 0), src);
    } else {
        writeHost(width, createMemPtr(tmp, addressReg, 0, 0), src);
    }
}

void JitArmV8CodeGen::writeMemCache(JitWidth width, U32 mem, RegPtr src) {
#ifdef _DEBUG
    writeCurrentEip(0);
#endif
    RegPtr tmp = getTmpReg();
    compiler.ldr(R64(tmp), createMem(regMMU, (mem >> K_PAGE_SHIFT) * sizeof(void*) + K_NUMBER_OF_PAGES * 2 * sizeof(void*)));
    writeHost(width, createMemPtr(tmp, mem), src);
}

void JitArmV8CodeGen::writeMemCache(JitWidth width, U32 mem, U32 imm) {
#ifdef _DEBUG
    writeCurrentEip(0);
#endif
    RegPtr tmp = getTmpReg();
    compiler.ldr(R64(tmp), createMem(regMMU, (mem >> K_PAGE_SHIFT) * sizeof(void*) + K_NUMBER_OF_PAGES * 2 * sizeof(void*)));
    writeHost(width, createMemPtr(tmp, mem), imm);
}

RegPtr JitArmV8CodeGen::readMemCache(JitWidth width, RegPtr addressReg, RegPtr tmp) {
    if (!tmp) {
        tmp = getTmpReg8();
    }
#ifdef _DEBUG
    writeCurrentEip(0);
#endif
    shrValueWithDest(JitWidth::b32, tmp, addressReg, K_PAGE_SHIFT);
    compiler.ldr(R64(tmp), createMem(regMMU, tmp->hardwareReg(), 3, K_NUMBER_OF_PAGES * sizeof(void*)));
    if (!KSystem::canJitUse4KPage && width != JitWidth::b8) {
        RegPtr offsetReg = getTmpReg();
        compiler.and_(R32(offsetReg), R32(addressReg), 0xfff);
        compiler.add(R64(tmp), R64(tmp), R64(addressReg));
        clearMMUPermissionIfSpansPage(width, offsetReg, tmp);
        readHost(width, createMemPtr(tmp, 0), tmp);
    } else {
        readHost(width, createMemPtr(tmp, addressReg, 0, 0), tmp);
    }
    return tmp;
}

RegPtr JitArmV8CodeGen::readMemCache(JitWidth width, U32 mem, RegPtr result) {
    if (!result) {
        result = getTmpReg8();
    }
#ifdef _DEBUG
    writeCurrentEip(0);
#endif
    compiler.ldr(R64(result), createMem(regMMU, (mem >> K_PAGE_SHIFT) * sizeof(void*) + K_NUMBER_OF_PAGES * sizeof(void*)));
    readHost(width, createMemPtr(result, mem), result);
    return result;
}
#endif

RegPtr JitArmV8CodeGen::readEip() {
    RegPtr result = getTmpReg();
    compiler.ldr(R32(result), Mem(xCPU, offsetof(CPU, eip.u32)));
    return result;
}

void JitArmV8CodeGen::writeEip(RegPtr reg) {
    compiler.str(R32(reg), Mem(xCPU, offsetof(CPU, eip.u32)));
}

void JitArmV8CodeGen::writeEip(U32 eip) {
    RegPtr reg = getTmpReg();
    compiler.mov(R32(reg), eip);
    writeEip(reg);
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
        compiler.str(R32(reg), Mem(xCPU, CPU::offsetofReg32(reg->emulatedReg)));
    }
}

RegPtr JitArmV8CodeGen::getReadOnlyRegInLower(JitWidth width, U8 reg) {
    return width == JitWidth::b8 ? getReg8InLowByte(getReadOnlyReg8(reg)) : getReadOnlyReg(reg);
}

RegPtr JitArmV8CodeGen::getReg(JitWidth width, U8 reg) {
    return width == JitWidth::b8 ? getReg8(reg) : getReg(reg);
}

RegPtr JitArmV8CodeGen::getReg(U8 reg, S8 hint, bool load) {
    if (regCache[reg] == INVALID_REG) {
        RegPtr result = std::shared_ptr<JitReg>(new JitReg(findTmpReg(), reg), [this](JitReg* p) {
            compiler.str(R32(p->hardwareReg()), Mem(xCPU, CPU::offsetofReg32(p->emulatedReg)));
            regUsed[p->hardwareReg()] = false;
            delete p;
        });
        if (load) {
            compiler.ldr(R32(result), Mem(xCPU, CPU::offsetofReg32(reg)));
        }
        return result;
    } else {
        return std::make_shared<JitReg>(regCache[reg], reg);
    }
}

RegPtr JitArmV8CodeGen::getReg8(U8 reg, bool load) {
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

RegPtr JitArmV8CodeGen::getTmpIfNotTmp(RegPtr reg) {
    if (reg->emulatedReg != 0xff) {
        RegPtr result = getTmpReg();
        mov(JitWidth::b32, result, reg);
        return result;
    }
    return reg;
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
    compiler.ubfx(R32(tmp), R32(reg), 8, 8);
    return tmp;
}

bool JitArmV8CodeGen::isRegHigh(RegPtr reg) {
    return reg->emulatedReg >= 4 && reg->emulatedReg <= 7;
}

void JitArmV8CodeGen::reg1(JitWidth regWidth, RegPtr reg, std::function<void(asmjit::a64::Gp dst, asmjit::a64::Gp src)> fn) {
    if (regWidth == JitWidth::b32) {
        fn(R32(reg), R32(reg));
    } else if (regWidth == JitWidth::b64) {
        fn(R64(reg), R64(reg));
    } else if (regWidth == JitWidth::b16) {
        RegPtr tmp = getTmpReg();
        fn(R32(tmp), R32(reg));
        compiler.bfxil(R32(reg), R32(tmp), 0, 16);
    } else if (regWidth == JitWidth::b8) {
        RegPtr tmp = getTmpReg();

        if (!isRegHigh(reg)) {
            fn(R32(tmp), R32(reg));
            compiler.bfi(R32(reg), R32(tmp), 0, 8);
        } else {
            compiler.ubfx(R32(tmp), R32(reg), 8, 8);
            fn(R32(tmp), R32(tmp));
            compiler.bfi(R32(reg), R32(tmp), 8, 8);
        }
    } else {
        kpanic("JitArmV8CodeGen::reg");
    }
}

void JitArmV8CodeGen::regReg(JitWidth regWidth, RegPtr reg, RegPtr rm, std::function<void(asmjit::a64::Gp dst, asmjit::a64::Gp reg, asmjit::a64::Gp rm)> fn, U32 extend) {    
    if (regWidth == JitWidth::b32) {
        fn(R32(reg), R32(reg), R32(rm));
    } else if (regWidth == JitWidth::b64) {
        fn(R64(reg), R64(reg), R64(rm));
    } else if (regWidth == JitWidth::b16) {
        RegPtr tmp = std::make_shared<JitReg>(regScratch, 0xff); // make this safe for internal functions by not using gettmp
        if (extend == ZERO_EXTEND) {
            compiler.ubfx(R32(tmp), R32(reg), 0, 16);
            fn(R32(tmp), R32(tmp), R32(rm));
        } else if (extend == SIGN_EXTEND) {
            movsx(JitWidth::b32, tmp, JitWidth::b16, reg);
            fn(R32(tmp), R32(tmp), R32(rm));
        } else {
            fn(R32(tmp), R32(reg), R32(rm));
        }
        compiler.bfxil(R32(reg), R32(tmp), 0, 16);
    } else if (regWidth == JitWidth::b8) {
        rm = getReg8InLowByte(std::move(rm));
        RegPtr tmp = std::make_shared<JitReg>(regScratch, 0xff); // make this safe for internal functions by not using gettmp

        if (!isRegHigh(reg)) {
            if (extend == ZERO_EXTEND) {
                compiler.ubfx(R32(tmp), R32(reg), 0, 8);
                fn(R32(tmp), R32(tmp), R32(rm));
            } else if (extend == SIGN_EXTEND) {
                movsx(JitWidth::b32, tmp, JitWidth::b8, reg);
                fn(R32(tmp), R32(tmp), R32(rm));
            } else {
                fn(R32(tmp), R32(reg), R32(rm));
            }
            compiler.bfxil(R32(reg), R32(tmp), 0, 8);
        } else {
            if (extend == SIGN_EXTEND) {
                movsx(JitWidth::b32, tmp, JitWidth::b8, reg);
            } else {
                compiler.ubfx(R32(tmp), R32(reg), 8, 8);
            }
            fn(R32(tmp), R32(tmp), R32(rm));
            compiler.bfi(R32(reg), R32(tmp), 8, 8);
        }
    } else {
        kpanic("JitArmV8CodeGen::regReg");
    }
}

void JitArmV8CodeGen::regValue(JitWidth regWidth, RegPtr reg, U32 value, std::function<void(asmjit::a64::Gp dst, asmjit::a64::Gp reg, U32 value)> fn, U32 extend) {
    if (regWidth == JitWidth::b32) {
        fn(R32(reg), R32(reg), value);
    } else if (regWidth == JitWidth::b64) {
        fn(R64(reg), R64(reg), value);
    } else if (regWidth == JitWidth::b16) {
        RegPtr tmp = getTmpReg();

        if (extend == ZERO_EXTEND) {
            compiler.ubfx(R32(tmp), R32(reg), 0, 16);
            fn(R32(tmp), R32(tmp), value);
        } else if (extend == SIGN_EXTEND) {
            movsx(JitWidth::b32, tmp, JitWidth::b16, reg);
            fn(R32(tmp), R32(tmp), value);
        } else {
            fn(R32(tmp), R32(reg), value);
        }
        compiler.bfxil(R32(reg), R32(tmp), 0, 16);
    } else if (regWidth == JitWidth::b8) {
        RegPtr tmp = getTmpReg();

        if (!isRegHigh(reg)) {
            if (extend == ZERO_EXTEND) {
                compiler.ubfx(R32(tmp), R32(reg), 0, 8);
                fn(R32(tmp), R32(tmp), value);
            } else if (extend == SIGN_EXTEND) {
                movsx(JitWidth::b32, tmp, JitWidth::b8, reg);
                fn(R32(tmp), R32(tmp), value);
            } else {
                fn(R32(tmp), R32(reg), value);
            }
            compiler.bfxil(R32(reg), R32(tmp), 0, 8);
        } else {
            if (extend == SIGN_EXTEND) {
                movsx(JitWidth::b32, tmp, JitWidth::b8, reg);
            } else {
                compiler.ubfx(R32(tmp), R32(reg), 8, 8);
            }
            fn(R32(tmp), R32(tmp), value);
            compiler.bfi(R32(reg), R32(tmp), 8, 8);
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

void JitArmV8CodeGen::addRegWithDest(JitWidth regWidth, RegPtr dst, RegPtr reg, RegPtr rm) {
    if (regWidth == JitWidth::b32) {
        compiler.add(R32(dst), R32(reg), R32(rm));
    } else if (regWidth == JitWidth::b64) {
        compiler.add(R64(dst), R64(reg), R64(rm));
    } else {
        JitCodeGen::addRegWithDest(regWidth, dst, reg, rm);
    }
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

void JitArmV8CodeGen::addValueWithDest(JitWidth regWidth, RegPtr dst, RegPtr reg, U32 value) {
    if (asmjit::a64::Utils::is_add_sub_imm(value)) {
        if (regWidth == JitWidth::b32) {
            compiler.add(R32(dst), R32(reg), value);
            return;
        } else if (regWidth == JitWidth::b64) {
            compiler.add(R64(dst), R64(reg), value);
            return;
        }
    }
    JitCodeGen::addValueWithDest(regWidth, dst, reg, value);
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

void JitArmV8CodeGen::subValueWithDest(JitWidth regWidth, RegPtr dst, RegPtr reg, U32 value) {
    if (regWidth != JitWidth::b32 || !asmjit::a64::Utils::is_add_sub_imm(value)) {
        JitCodeGen::subValueWithDest(regWidth, dst, reg, value);
        return;
    }
    compiler.sub(R32(dst), R32(reg), value);
}

void JitArmV8CodeGen::andReg(JitWidth regWidth, RegPtr reg, RegPtr rm) {
    regReg(regWidth, reg, rm, [this](asmjit::a64::Gp dst, asmjit::a64::Gp reg, asmjit::a64::Gp rm) {
        compiler.and_(dst, reg, rm);
    });
}

void JitArmV8CodeGen::andValueWithDest(JitWidth regWidth, RegPtr dst, RegPtr reg, U32 value) {
    if (regWidth != JitWidth::b32 || !asmjit::a64::Utils::is_logical_imm(value, 32)) {
        JitCodeGen::andValueWithDest(regWidth, dst, reg, value);
        return;
    }
    compiler.and_(R32(dst), R32(reg), value);
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
        compiler.and_(R64(reg), R64(reg), R64(value));
        return;
    }
    compiler.and_(R64(reg), R64(reg), imm);
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
    RegPtr cl = std::make_shared<JitReg>(regBranch, 0xff); // make this safe for internal functions by not using gettmp
    compiler.and_(R32(cl), R32(rm), regWidth == JitWidth::b64 ? 0x3f : 0x1f);
    regReg(regWidth, reg, cl, [this](asmjit::a64::Gp dst, asmjit::a64::Gp reg, asmjit::a64::Gp rm) {
        compiler.lsr(dst, reg, rm);
    }, ZERO_EXTEND);
}

void JitArmV8CodeGen::shrValueWithDest(JitWidth regWidth, RegPtr dst, RegPtr reg, U32 value) {
    if (regWidth != JitWidth::b32) {
        JitCodeGen::shrValueWithDest(regWidth, dst, reg, value);
        return;
    }
    compiler.lsr(R32(dst), R32(reg), value);
}

void JitArmV8CodeGen::shrValue(JitWidth regWidth, RegPtr reg, U32 imm) {
    regValue(regWidth, reg, imm, [this](asmjit::a64::Gp dst, asmjit::a64::Gp reg, U32 value) {
        compiler.lsr(dst, reg, value);
    }, ZERO_EXTEND);
}

void JitArmV8CodeGen::shlReg(JitWidth regWidth, RegPtr reg, RegPtr rm) {
    RegPtr cl = std::make_shared<JitReg>(regBranch, 0xff); // make this safe for internal functions by not using gettmp
    compiler.and_(R32(cl), R32(rm), regWidth == JitWidth::b64 ? 0x3f : 0x1f);
    regReg(regWidth, reg, cl, [this](asmjit::a64::Gp dst, asmjit::a64::Gp reg, asmjit::a64::Gp rm) {
        compiler.lsl(dst, reg, rm);
    });
}

void JitArmV8CodeGen::shlValue(JitWidth regWidth, RegPtr reg, U32 imm) {
    regValue(regWidth, reg, imm, [this](asmjit::a64::Gp dst, asmjit::a64::Gp reg, U32 value) {
        compiler.lsl(dst, reg, value);
    });
}

void JitArmV8CodeGen::shlValueWithDest(JitWidth regWidth, RegPtr dst, RegPtr reg, U32 value) {
    if (regWidth == JitWidth::b32) {
        compiler.lsl(R32(dst), R32(reg), value);
    } else if (regWidth == JitWidth::b64) {
        compiler.lsl(R64(dst), R64(reg), value);
    } else {
        JitCodeGen::shlValueWithDest(regWidth, dst, reg, value);
    }    
}

void JitArmV8CodeGen::sarReg(JitWidth regWidth, RegPtr reg, RegPtr rm) {
    RegPtr cl = std::make_shared<JitReg>(regBranch, 0xff); // make this safe for internal functions by not using gettmp
    compiler.and_(R32(cl), R32(rm), regWidth == JitWidth::b64 ? 0x3f : 0x1f);
    regReg(regWidth, reg, cl, [this](asmjit::a64::Gp dst, asmjit::a64::Gp reg, asmjit::a64::Gp rm) {
        compiler.asr(dst, reg, rm);
    }, SIGN_EXTEND);
}

void JitArmV8CodeGen::sarValueWithDest(JitWidth regWidth, RegPtr dst, RegPtr reg, U32 value) {
    if (regWidth != JitWidth::b32) {
        JitCodeGen::sarValueWithDest(regWidth, dst, reg, value);
        return;
    }
    compiler.asr(R32(dst), R32(reg), value);
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
        compiler.rbit(R32(reg), R32(rm));
        compiler.clz(R32(reg), R32(reg));
    } else if (regWidth == JitWidth::b16) {
        RegPtr tmp = getTmpReg();
        movzx(JitWidth::b32, tmp, JitWidth::b16, rm);
        compiler.rbit(R32(tmp), R32(tmp));
        compiler.clz(R32(tmp), R32(tmp));
        compiler.bfxil(R32(reg), R32(tmp), 0, 16);        
    } else {
        kpanic("JitArmV8CodeGen::bsfReg");
    }
}

void JitArmV8CodeGen::bsrReg(JitWidth regWidth, RegPtr reg, RegPtr rm) {
    // rm was already checked to not be 0 before getting here
    if (regWidth == JitWidth::b32) {
        compiler.clz(R32(reg), R32(rm));
        compiler.sub(R32(reg), R32(reg), 31);
        compiler.neg(R32(reg), R32(reg));
    } else if (regWidth == JitWidth::b16) {
        RegPtr tmp = getTmpReg();
        movzx(JitWidth::b32, tmp, JitWidth::b16, rm);
        compiler.clz(R32(tmp), R32(tmp));
        compiler.sub(R32(tmp), R32(tmp), 31);
        compiler.neg(R32(tmp), R32(tmp));
        compiler.bfxil(R32(reg), R32(tmp), 0, 16);
    } else {
        kpanic("JitArmV8CodeGen::bsrReg");
    }
}

void JitArmV8CodeGen::rolReg(JitWidth regWidth, RegPtr reg, RegPtr rm) {
    RegPtr cl = getTmpReg();    
    if (regWidth == JitWidth::b32) {
        RegPtr tmp = loadConst(32);
        compiler.and_(R32(cl), R32(rm), 0x1f);
        compiler.sub(R32(cl), R32(tmp), R32(cl));
        compiler.ror(R32(reg), R32(reg), R32(cl));
    } else if (regWidth == JitWidth::b16) {
        RegPtr tmp = getTmpReg();
        RegPtr tmp2 = loadConst(16);
        RegPtr tmp3 = getTmpReg();
        compiler.and_(R32(cl), R32(rm), 0xf);
        movzx(JitWidth::b32, tmp3, JitWidth::b16, reg);
        compiler.lsl(R32(tmp), R32(tmp3), R32(cl));
        compiler.sub(R32(tmp2), R32(tmp2), R32(cl));
        compiler.lsr(R32(tmp2), R32(tmp3), R32(tmp2));
        compiler.orr(R32(tmp), R32(tmp), R32(tmp2));
        compiler.bfxil(R32(reg), R32(tmp), 0, 16);
    } else if (regWidth == JitWidth::b8) {
        RegPtr tmp = getTmpReg();
        RegPtr tmp2 = loadConst(8);
        RegPtr tmp3 = getTmpReg();
        compiler.and_(R32(cl), R32(rm), 0x7);
        movzx(JitWidth::b32, tmp3, JitWidth::b8, reg);
        compiler.lsl(R32(tmp), R32(tmp3), R32(cl));
        compiler.sub(R32(tmp2), R32(tmp2), R32(cl));
        compiler.lsr(R32(tmp2), R32(tmp3), R32(tmp2));
        compiler.orr(R32(tmp), R32(tmp), R32(tmp2));
        compiler.bfi(R32(reg), R32(tmp), reg->isHigh ? 8 : 0, 8);
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
        compiler.ror(R32(reg), R32(reg), 32-imm);
    } else if (regWidth == JitWidth::b16) {
        RegPtr tmp = getTmpReg();
        imm = imm % 16;
        if (imm == 0) {
            return;
        }
        compiler.lsl(R32(tmp), R32(reg), imm);
        compiler.bfxil(R32(tmp), R32(reg), 16 - imm, imm);
        compiler.bfxil(R32(reg), R32(tmp), 0, 16);
    } else if (regWidth == JitWidth::b8) {
        imm = imm % 8;
        if (imm == 0) {
            return;
        }
        RegPtr tmp = getTmpReg();
        if (reg->isHigh) {
            compiler.lsr(R32(tmp), R32(reg), 8 - imm);
            compiler.bfxil(R32(tmp), R32(reg), 16 - imm, imm);
            compiler.bfi(R32(reg), R32(tmp), 8, 8);
        } else {
            compiler.lsl(R32(tmp), R32(reg), imm);
            compiler.bfxil(R32(tmp), R32(reg), 8 - imm, imm);
            compiler.bfxil(R32(reg), R32(tmp), 0, 8);
        }
    } else {
        kpanic("JitArmV8CodeGen::rolValue");
    }
}

void JitArmV8CodeGen::rorReg(JitWidth regWidth, RegPtr reg, RegPtr rm) {
    RegPtr cl = getTmpReg();
    if (regWidth == JitWidth::b32) {
        compiler.and_(R32(cl), R32(rm), 0x1f);
        compiler.ror(R32(reg), R32(reg), R32(cl));
    } else if (regWidth == JitWidth::b16) {
        RegPtr tmp = getTmpReg();
        RegPtr tmp2 = loadConst(16);
        RegPtr tmp3 = getTmpReg();
        compiler.and_(R32(cl), R32(rm), 0xf);
        movzx(JitWidth::b32, tmp3, JitWidth::b16, reg);
        compiler.lsr(R32(tmp), R32(tmp3), R32(cl));
        compiler.sub(R32(tmp2), R32(tmp2), R32(cl));
        compiler.lsl(R32(tmp2), R32(tmp3), R32(tmp2));
        compiler.orr(R32(tmp), R32(tmp), R32(tmp2));
        compiler.bfxil(R32(reg), R32(tmp), 0, 16);
    } else if (regWidth == JitWidth::b8) {
        RegPtr tmp = getTmpReg();
        RegPtr tmp2 = loadConst(8);
        RegPtr tmp3 = getTmpReg();
        compiler.and_(R32(cl), R32(rm), 0x7);
        movzx(JitWidth::b32, tmp3, JitWidth::b8, reg);
        compiler.lsr(R32(tmp), R32(tmp3), R32(cl));
        compiler.sub(R32(tmp2), R32(tmp2), R32(cl));
        compiler.lsl(R32(tmp2), R32(tmp3), R32(tmp2));
        compiler.orr(R32(tmp), R32(tmp), R32(tmp2));
        compiler.bfi(R32(reg), R32(tmp), reg->isHigh ? 8 : 0, 8);
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
        compiler.ror(R32(reg), R32(reg), imm);
    }
    else if (regWidth == JitWidth::b16) {
        RegPtr tmp = getTmpReg();
        imm = imm % 16;
        if (imm == 0) {
            return;
        }
        compiler.lsr(R32(tmp), R32(reg), imm);
        compiler.bfi(R32(tmp), R32(reg), 16 - imm, imm);
        compiler.bfxil(R32(reg), R32(tmp), 0, 16);
    }
    else if (regWidth == JitWidth::b8) {
        imm = imm % 8;
        if (imm == 0) {
            return;
        }
        RegPtr tmp = getTmpReg();
        if (reg->isHigh) {
            RegPtr low8 = getReg8InLowByte(reg);
            compiler.lsr(R32(tmp), R32(low8), imm);
            compiler.bfi(R32(tmp), R32(low8), 8 - imm, imm);
            compiler.bfi(R32(reg), R32(tmp), 8, 8);
        }
        else {
            compiler.lsr(R32(tmp), R32(reg), imm);
            compiler.bfi(R32(tmp), R32(reg), 8 - imm, imm);
            compiler.bfxil(R32(reg), R32(tmp), 0, 8);
        }
    }
    else {
        kpanic("JitArmV8CodeGen::rorValue");
    }
}

void JitArmV8CodeGen::modValue32(RegPtr dst, RegPtr src, RegPtr value) {
    RegPtr tmp = getTmpReg(); // in case dst == src
    compiler.udiv(R32(tmp), R32(src), R32(value));
    compiler.msub(R32(dst), R32(tmp), R32(value), R32(src));
}

void JitArmV8CodeGen::rclReg(JitWidth regWidth, RegPtr reg, RegPtr rm, RegPtr cf) {
    // result = (var1 << var2) | (cf << (var2 - 1)) | (var1 >> (33 - var2));
    if (regWidth == JitWidth::b32) {        
        RegPtr cl = getTmpReg();
        RegPtr tmp1 = getTmpReg();

        // (var1 << var2)
        compiler.and_(R32(cl), R32(rm), 0x1f);
        compiler.lsl(R32(tmp1), R32(reg), R32(cl));

        // (var1 >> (33 - var2))
        RegPtr tmp2 = loadConst(33);
        compiler.sub(R32(tmp2), R32(tmp2), R32(cl));
        compiler.lsr(R64(tmp2), R64(reg), R64(tmp2)); // R64 so that we shift in 0's in the case of cl = 1
        
        compiler.orr(R32(tmp1), R32(tmp2), R32(tmp1));

        // (cf << (var2 - 1))
        compiler.sub(R32(tmp2), R32(cl), 1);
        compiler.lsl(R32(tmp2), R32(cf), R32(tmp2));
        
        compiler.orr(R32(reg), R32(tmp2), R32(tmp1));        
    } else if (regWidth == JitWidth::b16) {
        RegPtr cl = getTmpReg();
        RegPtr tmp1 = getTmpReg();
        RegPtr tmp2 = loadConst(17);
        RegPtr src = getTmpReg();

        movzx(JitWidth::b32, src, JitWidth::b16, reg);
        // (var1 << var2)
        compiler.and_(R32(cl), R32(rm), 0x1f);
        modValue32(cl, cl, tmp2);
        compiler.lsl(R32(tmp1), R32(src), R32(cl));

        // (var1 >> (17 - var2))        
        compiler.sub(R32(tmp2), R32(tmp2), R32(cl));
        compiler.lsr(R32(tmp2), R32(src), R32(tmp2));

        compiler.orr(R32(tmp1), R32(tmp2), R32(tmp1));

        // (cf << (var2 - 1))
        compiler.sub(R32(tmp2), R32(cl), 1);
        compiler.lsl(R32(tmp2), R32(cf), R32(tmp2));

        compiler.orr(R32(tmp1), R32(tmp2), R32(tmp1));
        compiler.bfxil(R32(reg), R32(tmp1), 0, 16);
    } else if (regWidth == JitWidth::b8) {
        RegPtr cl = getTmpReg();
        RegPtr tmp1 = getTmpReg();
        RegPtr tmp2 = loadConst(9);
        RegPtr src = getTmpReg();

        movzx(JitWidth::b32, src, JitWidth::b8, reg);
        // (var1 << var2)
        compiler.and_(R32(cl), R32(rm), 0x1f);
        modValue32(cl, cl, tmp2);
        compiler.lsl(R32(tmp1), R32(src), R32(cl));

        // (var1 >> (9 - var2))        
        compiler.sub(R32(tmp2), R32(tmp2), R32(cl));
        compiler.lsr(R32(tmp2), R32(src), R32(tmp2));

        compiler.orr(R32(tmp1), R32(tmp2), R32(tmp1));

        // (cf << (var2 - 1))
        compiler.sub(R32(tmp2), R32(cl), 1);
        compiler.lsl(R32(tmp2), R32(cf), R32(tmp2));

        compiler.orr(R32(tmp1), R32(tmp2), R32(tmp1));
        compiler.bfi(R32(reg), R32(tmp1), reg->isHigh ? 8 : 0, 8);
    } else {
        kpanic("JitArmV8CodeGen::rclReg");
    }
}

void JitArmV8CodeGen::rclValue(JitWidth regWidth, RegPtr reg, U32 imm, RegPtr cf) {
    // result = (var1 << var2) | ((cpu->flags & CF) << (var2 - 1)) | (var1 >> (9 - var2));
    if (regWidth == JitWidth::b32) {
        RegPtr tmp = getTmpReg();
        imm = imm % 33;
        if (imm == 0) {
            return;
        }
        if (imm == 1) {
            compiler.lsl(R32(reg), R32(reg), imm);
            compiler.bfi(R32(reg), R32(cf), imm - 1, 1);
        } else {
            compiler.lsl(R32(tmp), R32(reg), imm);
            compiler.bfxil(R32(tmp), R32(reg), 33 - imm, imm - 1);
            compiler.bfi(R32(tmp), R32(cf), imm - 1, 1);
            compiler.mov(R32(reg), R32(tmp));
        }
    } else if (regWidth == JitWidth::b16) {
        RegPtr tmp = getTmpReg();
        imm = imm % 17;
        if (imm == 0) {
            return;
        }
        compiler.lsl(R32(tmp), R32(reg), imm);
        if (imm != 1) {
            compiler.bfxil(R32(tmp), R32(reg), 17 - imm, imm - 1);
        }
        compiler.bfi(R32(tmp), R32(cf), imm - 1, 1);
        compiler.bfxil(R32(reg), R32(tmp), 0, 16);
    } else if (regWidth == JitWidth::b8) {
        imm = imm % 9;
        if (imm == 0) {
            return;
        }
        RegPtr tmp = getTmpReg();
        if (reg->isHigh) {
            compiler.lsr(R32(tmp), R32(reg), 8 - imm);
            if (imm != 1) {
                compiler.bfxil(R32(tmp), R32(reg), 17 - imm, imm - 1);
            }
            compiler.bfi(R32(tmp), R32(cf), imm - 1, 1);
            compiler.bfi(R32(reg), R32(tmp), 8, 8);
        } else {
            compiler.lsl(R32(tmp), R32(reg), imm);
            if (imm != 1) {
                compiler.bfxil(R32(tmp), R32(reg), 9 - imm, imm - 1);
            }
            compiler.bfi(R32(tmp), R32(cf), imm - 1, 1);
            compiler.bfxil(R32(reg), R32(tmp), 0, 8);
        }
    } else {
        kpanic("JitArmV8CodeGen::rclValue");
    }
}

void JitArmV8CodeGen::rcrReg(JitWidth regWidth, RegPtr reg, RegPtr rm, RegPtr cf) {
    // result = (var1 >> var2) | (cf << (32 - var2)) | (var1 << (33 - var2));
    if (regWidth == JitWidth::b32) {
        RegPtr cl = getTmpReg();
        RegPtr tmp1 = getTmpReg();

        // (var1 >> var2)
        compiler.and_(R32(cl), R32(rm), 0x1f);
        compiler.lsr(R32(tmp1), R32(reg), R32(cl));

        // (var1 << (33 - var2))
        RegPtr tmp2 = loadConst(33);
        compiler.sub(R32(tmp2), R32(tmp2), R32(cl));
        compiler.lsl(R64(tmp2), R64(reg), R64(tmp2)); // R64, otherwise in the case of cl = 1, 33 - 1 = 32, which does nothing on a 32-bit shift

        compiler.orr(R32(tmp1), R32(tmp2), R32(tmp1));

        // (cf << (32 - var2))
        RegPtr tmp3 = loadConst(32);
        compiler.sub(R32(tmp2), R32(tmp3), R32(cl));
        compiler.lsl(R32(tmp2), R32(cf), R32(tmp2));

        compiler.orr(R32(reg), R32(tmp2), R32(tmp1));
    } else if (regWidth == JitWidth::b16) {
        RegPtr cl = getTmpReg();
        RegPtr tmp1 = getTmpReg();
        RegPtr tmp2 = loadConst(17);
        RegPtr src = getTmpReg();

        movzx(JitWidth::b32, src, JitWidth::b16, reg);
        // (var1 >> var2)
        compiler.and_(R32(cl), R32(rm), 0x1f);
        modValue32(cl, cl, tmp2);
        compiler.lsr(R32(tmp1), R32(src), R32(cl));

        // (var1 << (17 - var2))       
        compiler.sub(R32(tmp2), R32(tmp2), R32(cl));
        compiler.lsl(R32(tmp2), R32(src), R32(tmp2));

        compiler.orr(R32(tmp1), R32(tmp2), R32(tmp1));

        // (cf << (16 - var2))
        compiler.sub(R32(tmp2), R32(cl), 16);
        compiler.neg(R32(tmp2), R32(tmp2)); // instead of load const, saves a tmp reg
        compiler.lsl(R32(tmp2), R32(cf), R32(tmp2));

        compiler.orr(R32(tmp1), R32(tmp2), R32(tmp1));
        compiler.bfxil(R32(reg), R32(tmp1), 0, 16);
    } else if (regWidth == JitWidth::b8) {
        RegPtr cl = getTmpReg();
        RegPtr tmp1 = getTmpReg();
        RegPtr tmp2 = loadConst(9);
        RegPtr src = getTmpReg();

        movzx(JitWidth::b32, src, JitWidth::b8, reg);
        // (var1 >> var2)
        compiler.and_(R32(cl), R32(rm), 0x1f);
        modValue32(cl, cl, tmp2);
        compiler.lsl(R32(tmp1), R32(src), R32(cl));

        // (var1 << (9 - var2))       
        compiler.sub(R32(tmp2), R32(tmp2), R32(cl));
        compiler.lsr(R32(tmp2), R32(src), R32(tmp2));

        compiler.orr(R32(tmp1), R32(tmp2), R32(tmp1));

        // (cf << (8 - var2))
        compiler.sub(R32(tmp2), R32(cl), 1);
        compiler.lsl(R32(tmp2), R32(cf), R32(tmp2));

        compiler.orr(R32(tmp1), R32(tmp2), R32(tmp1));
        compiler.bfi(R32(reg), R32(tmp1), reg->isHigh ? 8 : 0, 8);
    } else {
        kpanic("JitArmV8CodeGen::rclValue");
    }
}

void JitArmV8CodeGen::rcrValue(JitWidth regWidth, RegPtr reg, U32 imm, RegPtr cf) {
    // result = (var1 << var2) | ((cpu->flags & CF) << (var2 - 1)) | (var1 >> (9 - var2));
    if (regWidth == JitWidth::b32) {
        RegPtr tmp = getTmpReg();
        imm = imm % 33;
        if (imm == 0) {
            return;
        }
        if (imm == 1) {
            compiler.lsr(R32(reg), R32(reg), imm);
            compiler.bfi(R32(reg), R32(cf), 32 - imm, 1);
        } else {
            compiler.lsr(R32(tmp), R32(reg), imm);
            compiler.bfi(R32(tmp), R32(reg), 33 - imm, imm - 1);
            compiler.bfi(R32(tmp), R32(cf), 32 - imm, 1);
            compiler.mov(R32(reg), R32(tmp));
        }
    } else if (regWidth == JitWidth::b16) {
        RegPtr tmp = getTmpReg();
        imm = imm % 17;
        if (imm == 0) {
            return;
        }
        compiler.lsr(R32(tmp), R32(reg), imm);
        if (imm != 1) {
            compiler.bfi(R32(tmp), R32(reg), 17 - imm, imm - 1);
        }
        compiler.bfi(R32(tmp), R32(cf), 16 - imm, 1);
        compiler.bfxil(R32(reg), R32(tmp), 0, 16);
    } else if (regWidth == JitWidth::b8) {
        imm = imm % 9;
        if (imm == 0) {
            return;
        }
        RegPtr tmp = getTmpReg();
        if (reg->isHigh) {            
            if (imm == 1) {
                compiler.lsr(R32(tmp), R32(reg), 8 + imm);
            } else {
                compiler.lsr(R32(tmp), R32(reg), imm - 1);
                compiler.bfxil(R32(tmp), R32(reg), imm + 8, 8 - imm);
            }
            compiler.bfi(R32(tmp), R32(cf), 8 - imm, 1);
            compiler.bfi(R32(reg), R32(tmp), 8, 8);
        } else {
            compiler.lsr(R32(tmp), R32(reg), imm);
            if (imm != 1) {
                compiler.bfi(R32(tmp), R32(reg), 9 - imm, imm - 1);
            }
            compiler.bfi(R32(tmp), R32(cf), 8 - imm, 1);
            compiler.bfxil(R32(reg), R32(tmp), 0, 8);
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
        compiler.lsl(R32(tmp), R32(reg), R32(cl));
        RegPtr tmp1 = loadConst(32);
        compiler.sub(R32(tmp1), R32(tmp1), R32(cl));
        compiler.lsr(R32(reg), R32(rm), R32(tmp1));
        compiler.orr(R32(reg), R32(reg), R32(tmp));
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
        compiler.lsl(R32(tmp), R32(reg16), R32(cl));
        RegPtr tmp1 = loadConst(16);
        compiler.sub(R32(tmp1), R32(tmp1), R32(cl));
        compiler.lsr(R32(reg16), R32(rm16), R32(tmp1));
        compiler.orr(R32(reg16), R32(reg16), R32(tmp));
        compiler.bfxil(R32(reg), R32(reg16), 0, 16);
    } else {
        kpanic("JitArmV8CodeGen::shldValue");
    }
}

void JitArmV8CodeGen::shldValue(JitWidth regWidth, RegPtr reg, RegPtr rm, U32 imm) {
    // don't need to check if imm is 0, that was handled in the decoder, if it was 0, the decoder will replace shld with nop

    if (regWidth == JitWidth::b32) {
        // cpu->result.u32=(cpu->reg[reg].u32 << imm) | (cpu->reg[rm].u32 >> (32-imm));
        RegPtr tmp = getTmpReg();
        compiler.bfi(R32(tmp), R32(reg), imm, 32 - imm);
        compiler.bfxil(R32(tmp), R32(rm), 32 - imm, imm);
        compiler.mov(R32(reg), R32(tmp));
    } else if (regWidth == JitWidth::b16) {
        RegPtr tmp = getTmpReg();
        if (imm >= 16) {
            imm -= 16;
            mov(JitWidth::b16, reg, rm);
        }
        if (imm != 0) {
            compiler.bfi(R32(tmp), R32(reg), imm, 16 - imm);
            compiler.bfxil(R32(tmp), R32(rm), 16 - imm, imm);
            compiler.bfxil(R32(reg), R32(tmp), 0, 16);
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
        compiler.lsr(R32(tmp), R32(reg), R32(cl));
        RegPtr tmp1 = loadConst(32);
        compiler.sub(R32(tmp1), R32(tmp1), R32(cl));
        compiler.lsl(R32(reg), R32(rm), R32(tmp1));
        compiler.orr(R32(reg), R32(reg), R32(tmp));
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
        compiler.lsr(R32(tmp), R32(reg16), R32(cl));
        RegPtr tmp1 = loadConst(16);
        compiler.sub(R32(tmp1), R32(tmp1), R32(cl));
        compiler.lsl(R32(reg16), R32(rm16), R32(tmp1));
        compiler.orr(R32(reg16), R32(reg16), R32(tmp));
        compiler.bfxil(R32(reg), R32(reg16), 0, 16);
    } else {
        kpanic("JitArmV8CodeGen::shrdReg");
    }
}

void JitArmV8CodeGen::shrdValue(JitWidth regWidth, RegPtr reg, RegPtr rm, U32 imm) {
    if (regWidth == JitWidth::b32) {
        // cpu->result.u32 = (cpu->reg[reg].u32 >> imm) | (cpu->reg[rm].u32 << (32 - imm));
        RegPtr tmp = getTmpReg();
        compiler.lsr(R32(tmp), R32(reg), imm);
        compiler.lsl(R32(reg), R32(rm), 32 - imm);
        compiler.orr(R32(reg), R32(reg), R32(tmp));
    } else if (regWidth == JitWidth::b16) {
        RegPtr tmp = getTmpReg();
        if (imm >= 16) {
            imm -= 16;
            mov(JitWidth::b16, reg, rm);
        }
        if (imm != 0) {
            compiler.lsr(R32(tmp), R32(reg), imm);
            compiler.bfi(R32(tmp), R32(rm), 16 - imm, imm);
            compiler.bfxil(R32(reg), R32(tmp), 0, 16);
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
    compiler.rev32(R32(reg), R32(reg));
}

RegPtr JitArmV8CodeGen::compare(JitEvaluate condition, RegPtr result) {
    if (!result) {
        result = getTmpReg();
    }
    switch (condition) {
    case JitEvaluate::EQUALS:
        compiler.csinc(R32(result), asmjit::a64::wzr, asmjit::a64::wzr, asmjit::a64::CondCode::kNE);
        break;
    case JitEvaluate::NOT_EQUALS:
        compiler.csinc(R32(result), asmjit::a64::wzr, asmjit::a64::wzr, asmjit::a64::CondCode::kEQ);
        break;
    case JitEvaluate::GREATER_THAN_UNSIGNED:
        compiler.csinc(R32(result), asmjit::a64::wzr, asmjit::a64::wzr, asmjit::a64::CondCode::kLS);
        break;
    case JitEvaluate::GREATER_THAN_EQUAL_UNSIGNED:
        compiler.csinc(R32(result), asmjit::a64::wzr, asmjit::a64::wzr, asmjit::a64::CondCode::kLO);
        break;
    case JitEvaluate::LESS_THAN_UNSIGNED:
        compiler.csinc(R32(result), asmjit::a64::wzr, asmjit::a64::wzr, asmjit::a64::CondCode::kHS);
        break;
    case JitEvaluate::LESS_THAN_EQUAL_UNSIGNED:
        compiler.csinc(R32(result), asmjit::a64::wzr, asmjit::a64::wzr, asmjit::a64::CondCode::kHI);
        break;
    case JitEvaluate::GREATER_THAN_SIGNED:
        compiler.csinc(R32(result), asmjit::a64::wzr, asmjit::a64::wzr, asmjit::a64::CondCode::kLE);
        break;
    case JitEvaluate::GREATER_THAN_EQUAL_SIGNED:
        compiler.csinc(R32(result), asmjit::a64::wzr, asmjit::a64::wzr, asmjit::a64::CondCode::kLT);
        break;
    case JitEvaluate::LESS_THAN_SIGNED:
        compiler.csinc(R32(result), asmjit::a64::wzr, asmjit::a64::wzr, asmjit::a64::CondCode::kGE);
        break;
    case JitEvaluate::LESS_THAN_EQUAL_SIGNED:
        compiler.csinc(R32(result), asmjit::a64::wzr, asmjit::a64::wzr, asmjit::a64::CondCode::kGT);
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
        x86.test(R32(reg), R32(reg));
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
    x86.movzx(R32(result), R8(get8bitReg(result)));
    */
    return result;
}

void JitArmV8CodeGen::mulReg(JitWidth regWidth, RegPtr reg) {
    if (regWidth == JitWidth::b32) {
        // EDX:EAX = (U64)EAX * src;
        compiler.mul(xEAX, xEAX, R64(reg));
        compiler.ubfx(xEDX, xEAX, 32, 32);
        compiler.mov(wEAX, wEAX); // clear top 32-bits
    } else if (regWidth == JitWidth::b16) {
        // DX:AX = AX * src;
        RegPtr tmp = getTmpReg();
        RegPtr tmp2 = getTmpReg();
        compiler.ubfx(R32(tmp), wEAX, 0, 16);
        compiler.ubfx(R32(tmp2), R32(reg), 0, 16);
        compiler.mul(R32(tmp), R32(tmp), R32(tmp2));
        compiler.bfxil(wEDX, R32(tmp), 16, 16);
        compiler.bfxil(wEAX, R32(tmp), 0, 16);
    } else if (regWidth == JitWidth::b8) {
        // AX = AL * src;
        RegPtr tmp = getTmpReg();
        RegPtr tmp2 = getTmpReg();
        compiler.ubfx(R32(tmp), wEAX, 0, 8);
        compiler.ubfx(R32(tmp2), R32(reg), reg->isHigh ? 8 : 0, 8);
        compiler.mul(R32(tmp), R32(tmp), R32(tmp2));
        compiler.bfxil(wEAX, R32(tmp), 0, 16);
    } else {
        kpanic("JitArmV8CodeGen::mulReg");
    }
}

void JitArmV8CodeGen::imulRR(JitWidth regWidth, RegPtr dst, RegPtr src, RegPtr overflow) {
    if (regWidth == JitWidth::b32) {
        compiler.smull(R64(dst), R32(dst), R32(src));

        if (overflow) {
            compiler.lsr(R64(overflow), R64(dst), 32);
        }
        compiler.mov(R32(dst), R32(dst)); // clear out top 32-bits
    } else if (regWidth == JitWidth::b16) {
        if (overflow) {
            kpanic("JitArmV8CodeGen::imulRR overflow");
        }
        RegPtr dst16 = getTmpReg();
        RegPtr src16 = getTmpReg();

        movsx(JitWidth::b32, dst16, JitWidth::b16, dst);
        movsx(JitWidth::b32, src16, JitWidth::b16, src);
        compiler.smull(R64(dst16), R32(dst16), R32(src16));
        compiler.bfxil(R32(dst), R32(dst16), 0, 16);
    } else {
        kpanic("JitArmV8CodeGen::imulRR");
    }
}

void JitArmV8CodeGen::imulRRI(JitWidth regWidth, RegPtr dst, RegPtr src, U32 src2, RegPtr overflow) {    
    RegPtr value = loadConst(src2);
    if (regWidth == JitWidth::b32) {
        RegPtr value = loadConst(src2);
        compiler.smull(R64(dst), R32(src), R32(value));
        if (overflow) {
            compiler.ubfx(R64(overflow), R64(dst), 32, 32);
        }
    } else if (regWidth == JitWidth::b16) {
        RegPtr value = loadConst((S32)((S16)src2));
        RegPtr src16 = getTmpReg();
        RegPtr dst16 = getTmpReg();
        movsx(JitWidth::b32, src16, JitWidth::b16, src);
        compiler.smull(R64(dst16), R32(src16), R32(value));
        mov(JitWidth::b16, dst, dst16);
        if (overflow) {
            compiler.ubfx(R32(overflow), R32(dst16), 16, 16);
        }
    } else {
        kpanic("JitArmV8CodeGen::imulRRI");
    }
}

void JitArmV8CodeGen::imulReg(JitWidth regWidth, RegPtr reg) {
    if (regWidth == JitWidth::b32) {
        // EDX:EAX = (S64)EAX * src;
        compiler.smull(xEAX, wEAX, R32(reg));
        compiler.ubfx(xEDX, xEAX, 32, 32);
        compiler.mov(wEAX, wEAX); // clear top 32-bits
    } else if (regWidth == JitWidth::b16) {
        // DX:AX = AX * src;
        RegPtr tmp = getTmpReg();
        RegPtr tmp2 = getTmpReg();
        compiler.sxth(R32(tmp), wEAX);
        compiler.sxth(R32(tmp2), R32(reg));
        compiler.smull(R64(tmp), R32(tmp), R32(tmp2));
        compiler.bfxil(wEDX, R32(tmp), 16, 16);
        compiler.bfxil(wEAX, R32(tmp), 0, 16);
    } else if (regWidth == JitWidth::b8) {
        // AX = AL * src;
        RegPtr tmp = getTmpReg();
        RegPtr tmp2 = getTmpReg();
        compiler.sxtb(R32(tmp), wEAX);
        compiler.sbfx(R32(tmp2), R32(reg), reg->isHigh ? 8 : 0, 8);
        compiler.smull(R64(tmp), R32(tmp), R32(tmp2));
        compiler.bfxil(wEAX, R32(tmp), 0, 16);
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
        compiler.add(R64(tmp), R64(reg), R64(reg), asmjit::Imm(asmjit::a64::Shift(asmjit::a64::ShiftOp::kASR, 63)));
        compiler.eor(R64(reg), R64(tmp), R64(reg), asmjit::Imm(asmjit::a64::Shift(asmjit::a64::ShiftOp::kASR, 63)));
        return;
    } else if (regWidth != JitWidth::b32) {
        kpanic("JitArmV8CodeGen::absReg");
    }

    compiler.add(R32(tmp), R32(reg), R32(reg), asmjit::Imm(asmjit::a64::Shift(asmjit::a64::ShiftOp::kASR, 31)));
    compiler.eor(R32(reg), R32(tmp), R32(reg), asmjit::Imm(asmjit::a64::Shift(asmjit::a64::ShiftOp::kASR, 31)));
}

void JitArmV8CodeGen::clzReg(JitWidth regWidth, RegPtr result, RegPtr reg) {
   if (regWidth == JitWidth::b32) {
        compiler.clz(R32(result), R32(reg));
#ifdef BOXEDWINE_64
    } else if (regWidth == JitWidth::b64) {
        compiler.clz(R64(result), R64(reg));
#endif
    } else {
        kpanic("JitArmV8CodeGen::clzReg");
    }
}

void JitArmV8CodeGen::divRegRegWithRemainder(JitWidth regWidth, RegPtr dest, RegPtr destHighAndRemainder, RegPtr src) {
    if (regWidth == JitWidth::b32) {
        // U64 num = ((U64)EDX << 32) | EAX;
        RegPtr tmp = getTmpReg();
        compiler.orr(R64(tmp), R64(dest), R64(destHighAndRemainder), asmjit::Imm(asmjit::a64::Shift(asmjit::a64::ShiftOp::kLSL, 32)));
        compiler.udiv(R64(dest), R64(tmp), R64(src));
        compiler.msub(R64(destHighAndRemainder), R64(dest), R64(src), R64(tmp));
        compiler.mov(R32(dest), R32(dest)); // clear top 32-bits
    } else if (regWidth == JitWidth::b16) {
        // U32 num = ((U32)DX << 16) | AX;
        RegPtr tmp = getTmpReg();
        RegPtr result = getTmpReg();
        RegPtr src16 = getTmpReg();

        compiler.ubfx(R32(src16), R32(src), 0, 16);
        compiler.ubfx(R32(tmp), R32(dest), 0, 16);
        compiler.orr(R32(tmp), R32(tmp), R32(destHighAndRemainder), asmjit::Imm(asmjit::a64::Shift(asmjit::a64::ShiftOp::kLSL, 16)));
        compiler.udiv(R32(result), R32(tmp), R32(src16));
        compiler.bfxil(R32(dest), R32(result), 0, 16);
        compiler.msub(R32(tmp), R32(result), R32(src16), R32(tmp));
        compiler.bfxil(R32(destHighAndRemainder), R32(tmp), 0, 16);
    } else if (regWidth == JitWidth::b8) {
        // quo = AX / src;
        RegPtr tmp = getTmpReg();
        RegPtr result = getTmpReg();
        RegPtr src8 = getTmpReg();

        compiler.ubfx(R32(src8), R32(src), src->isHigh ? 8 : 0, 8);
        compiler.ubfx(R32(tmp), R32(dest), 0, 16);
        compiler.udiv(R32(result), R32(tmp), R32(src8));
        compiler.bfxil(R32(dest), R32(result), 0, 8);
        compiler.msub(R32(tmp), R32(result), R32(src8), R32(tmp));
        compiler.bfi(R32(dest), R32(tmp), 8, 8);
    } else {
        kpanic("JitArmV8CodeGen::divRegRegWithRemainder");
    }
}

void JitArmV8CodeGen::idivRegRegWithRemainder(JitWidth regWidth, RegPtr dest, RegPtr destHighAndRemainder, RegPtr src) {
    if (regWidth == JitWidth::b32) {
        // S64 num = ((S64)EDX << 32) | EAX;
        RegPtr tmp = getTmpReg();
        RegPtr src32 = getTmpReg();

        compiler.sbfx(R64(src32), R64(src), 0, 32);
        compiler.orr(R64(tmp), R64(dest), R64(destHighAndRemainder), asmjit::Imm(asmjit::a64::Shift(asmjit::a64::ShiftOp::kLSL, 32)));
        compiler.sdiv(R64(dest), R64(tmp), R64(src32));
        compiler.msub(R64(destHighAndRemainder), R64(dest), R64(src32), R64(tmp));
        compiler.mov(R32(dest), R32(dest)); // clear top 32-bits
    } else if (regWidth == JitWidth::b16) {
        // S32 num = ((S32)DX << 16) | AX;
        RegPtr tmp = getTmpReg();
        RegPtr result = getTmpReg();
        RegPtr src16 = getTmpReg();

        compiler.sbfx(R32(src16), R32(src), 0, 16);
        compiler.ubfx(R32(tmp), R32(dest), 0, 16);
        compiler.orr(R32(tmp), R32(tmp), R32(destHighAndRemainder), asmjit::Imm(asmjit::a64::Shift(asmjit::a64::ShiftOp::kLSL, 16)));
        compiler.sdiv(R32(result), R32(tmp), R32(src16));
        compiler.bfxil(R32(dest), R32(result), 0, 16);
        compiler.msub(R32(tmp), R32(result), R32(src16), R32(tmp));
        compiler.bfxil(R32(destHighAndRemainder), R32(tmp), 0, 16);
    } else if (regWidth == JitWidth::b8) {
        // quo = AX / src;
        RegPtr tmp = getTmpReg();
        RegPtr result = getTmpReg();
        RegPtr src8 = getTmpReg();

        compiler.sbfx(R32(src8), R32(src), src->isHigh ? 8 : 0, 8);
        compiler.sbfx(R32(tmp), R32(dest), 0, 16);
        compiler.sdiv(R32(result), R32(tmp), R32(src8));
        compiler.bfxil(R32(dest), R32(result), 0, 8);
        compiler.msub(R32(tmp), R32(result), R32(src8), R32(tmp));
        compiler.bfi(R32(dest), R32(tmp), 8, 8);
    } else {
        kpanic("JitArmV8CodeGen::idivRegRegWithRemainder");
    }
}

#include "../../softmmu/kmemory_soft.h"
void JitArmV8CodeGen::readMMU(RegPtr dest, RegPtr index) {
    compiler.ldr(R64(dest), Mem(xMMU, R64(index), Shift(ShiftOp::kLSL, 3)));
}

void JitArmV8CodeGen::readMMU(RegPtr dest, U32 index) {
    compiler.ldr(R64(dest), createMem(regMMU, index * 8));
}

Mem JitArmV8CodeGen::createMem(MemPtr mem) {
    if (mem->sib) {
        return createMem(mem->rm, mem->sib, mem->lsl, mem->offset);
    }
    return createMem(mem->rm, mem->offset);
}

Mem JitArmV8CodeGen::createMem(RegPtr reg, RegPtr sib, U8 lsl, U32 disp) {
    return createMem(reg->hardwareReg(), sib->hardwareReg(), lsl, disp);
}

Mem JitArmV8CodeGen::createMem(U8 reg, U8 sib, U8 lsl, U32 disp) {
    if (lsl) {
        if (disp) {
            RegPtr tmp = getTmpReg();
            if (asmjit::a64::Utils::is_add_sub_imm(disp)) {
                compiler.add(R64(tmp), R64(reg), disp);
            } else {
                compiler.add(R64(tmp), R64(reg), R64(loadConst(disp)));
            }
            return Mem(R64(tmp), R64(sib), Shift(ShiftOp::kLSL, lsl));
        } else {
            return Mem(R64(reg), R64(sib), Shift(ShiftOp::kLSL, lsl));
        }
    } else {
        if (disp) {
            RegPtr tmp = getTmpReg();
            if (asmjit::a64::Utils::is_add_sub_imm(disp)) {
                compiler.add(R64(tmp), R64(sib), disp);
            } else {
                compiler.add(R64(tmp), R64(sib), R64(loadConst(disp)));
            }
            return Mem(R64(reg), R64(tmp));
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
    return Mem(R64(reg), R64(dispReg));
}

RegPtr JitArmV8CodeGen::loadConst(U64 value) {
    RegPtr reg = getTmpReg();
    compiler.mov(R64(reg), value);
    return reg;
}

MMXRegPtr JitArmV8CodeGen::loadMMXConst(U8 index) {
    MMXRegPtr result = getTmpMMX();
    compiler.ldr(Vec::make_v128(result->hardwareReg()), createMem(regCPU, offsetof(CPU, sseConstants) + index * 16));
    return result;
}

SSERegPtr JitArmV8CodeGen::loadSSEConst(U8 index) {
    SSERegPtr result = getTmpSSE();
    compiler.ldr(Vec::make_v128(result->hardwareReg()), createMem(regCPU, offsetof(CPU, sseConstants) + index * 16));
    return result;
}

void JitArmV8CodeGen::readHost(JitWidth width, MemPtr mem, RegPtr dest, bool emlulatedMemory) {
    // arm zero extends reads
    if (!isTmp[dest->hardwareReg()] && (width == JitWidth::b8 || width == JitWidth::b16)) {
        RegPtr tmp = getTmpReg();
        readHost(width, mem, tmp);
        mov(width, dest, tmp);
        return;
    }
    if (width == JitWidth::b32) {
        if (emlulatedMemory && tsoMode == TSOMode::FEAT_LRCPC2) {
            // compiler.ldapr(R32(dest), createMemR(reg, sib, lsl, disp));
            RegPtr addressReg = calculateAddress(mem, JitWidth::b64);
            U32 op = 0xB8BFC000 | dest->hardwareReg() | (addressReg->hardwareReg() << 5);
            compiler.embed_int32(op);
        } else {
            compiler.ldr(R32(dest), createMem(mem));
        }
    } else if (width == JitWidth::b16) {
        if (emlulatedMemory && tsoMode == TSOMode::FEAT_LRCPC2) {
            // compiler.ldaprh(R32(dest), createMemR(reg, sib, lsl, disp));
            RegPtr addressReg = calculateAddress(mem, JitWidth::b64);
            U32 op = 0x78BFC000 | dest->hardwareReg() | (addressReg->hardwareReg() << 5);
            compiler.embed_int32(op);
        } else {
            compiler.ldrh(R32(dest), createMem(mem));
        }
    } else if (width == JitWidth::b8) {
        if (isRegHigh(dest)) {
            kpanic("JitArmV8CodeGen::readHost unexpected dest");
        }
        if (emlulatedMemory && tsoMode == TSOMode::FEAT_LRCPC2) {
            // compiler.ldaprb(R32(dest), createMemR(reg, sib, lsl, disp));
            RegPtr addressReg = calculateAddress(mem, JitWidth::b64);
            U32 op = 0x38BFC000 | dest->hardwareReg() | (addressReg->hardwareReg() << 5);
            compiler.embed_int32(op);
        } else {
            compiler.ldrb(R32(dest), createMem(mem));
        }
#ifdef BOXEDWINE_64
    } else if (width == JitWidth::b64) {
        if (emlulatedMemory && tsoMode == TSOMode::FEAT_LRCPC2) {
            // compiler.ldapr(R64(dest), createMemR(reg, sib, lsl, disp));
            RegPtr addressReg = calculateAddress(mem, JitWidth::b64);
            U32 op = 0xF8BFC000 | dest->hardwareReg() | (addressReg->hardwareReg() << 5);
            compiler.embed_int32(op);
        } else {
            compiler.ldr(R64(dest), createMem(mem));
        }
#endif
    } else {
        kpanic_fmt("JitArmV8CodeGen::readMem unexpected width: %d", (U32)width);
    }
    if (emlulatedMemory && tsoMode == TSOMode::Barrier) {
        compiler.dmb(asmjit::a64::Predicate::DB::kISH);
    }
}

void JitArmV8CodeGen::writeHost(JitWidth width, MemPtr mem, RegPtr src, bool emlulatedMemory) {
    if (emlulatedMemory && tsoMode == TSOMode::Barrier) {
        compiler.dmb(asmjit::a64::Predicate::DB::kISH);
    }
    if(width == JitWidth::b32) {
        if (emlulatedMemory && tsoMode == TSOMode::FEAT_LRCPC2) {
            // compiler.stlur(R32(dest), createMem9(reg, disp));
            U32 imm = 0;
            if (mem->offset && (S32)mem->offset >= -256 && (S32)mem->offset <= 255) {
                // Quake 2 hits this path
                imm = mem->offset & 0x1ff;
                mem->offset = 0;
            }
            RegPtr addressReg = calculateAddress(mem, JitWidth::b64);
            U32 op = 0x99000000 | src->hardwareReg() | (addressReg->hardwareReg() << 5) | (imm << 12);
            compiler.embed_int32(op);
        } else {
            compiler.str(R32(src), createMem(mem));
        }
    } else if (width == JitWidth::b16) {
        if (emlulatedMemory && tsoMode == TSOMode::FEAT_LRCPC2) {
            // compiler.stlurh(R32(dest), createMem9(reg, disp));
            U32 imm = 0;
            if (mem->offset && (S32)mem->offset >= -256 && (S32)mem->offset <= 255) {
                imm = mem->offset & 0x1ff;
                mem->offset = 0;
            }
            RegPtr addressReg = calculateAddress(mem, JitWidth::b64);
            U32 op = 0x59000000 | src->hardwareReg() | (addressReg->hardwareReg() << 5) | (imm << 12);
            compiler.embed_int32(op);
        } else {
            compiler.strh(R32(src), createMem(mem));
        }
    } else if (width == JitWidth::b8) {
        if (emlulatedMemory && tsoMode == TSOMode::FEAT_LRCPC2) {
            // compiler.stlurb(R32(dest), createMem9(reg, disp));
            U32 imm = 0;
            if (mem->offset && (S32)mem->offset >= -256 && (S32)mem->offset <= 255) {
                imm = mem->offset & 0x1ff;
                mem->offset = 0;
            }
            RegPtr addressReg = calculateAddress(mem, JitWidth::b64);
            RegPtr src8 = getReg8InLowByte(src);
            U32 op = 0x19000000 | src8->hardwareReg() | (addressReg->hardwareReg() << 5) | (imm << 12);
            compiler.embed_int32(op);
        } else {
            compiler.strb(R32(getReg8InLowByte(src)), createMem(mem));
        }
#ifdef BOXEDWINE_64
    }
    else if (width == JitWidth::b64) {
        if (emlulatedMemory && tsoMode == TSOMode::FEAT_LRCPC2) {
            // compiler.stlur(R32(dest), createMem9(reg, disp));
            U32 imm = 0;
            if (mem->offset && (S32)mem->offset >= -256 && (S32)mem->offset <= 255) {
                imm = mem->offset & 0x1ff;
                mem->offset = 0;
            }
            RegPtr addressReg = calculateAddress(mem, JitWidth::b64);
            U32 op = 0xD9000000 | src->hardwareReg() | (addressReg->hardwareReg() << 5) | (imm << 12);
            compiler.embed_int32(op);
        } else {
            compiler.str(R64(src), createMem(mem));
        }
#endif
    } else {
        kpanic_fmt("JitArmV8CodeGen::write unexpected width: %d", (U32)width);
    }
}

void JitArmV8CodeGen::writeHost(JitWidth width, MemPtr mem, U32 value, bool emlulatedMemory) {
    writeHost(width, mem, loadConst(value), emlulatedMemory);
}

RegPtr JitArmV8CodeGen::readCPU(JitWidth width, U32 offset, RegPtr reg) {
    if (!reg) {
        reg = getTmpReg();
    }
    if (reg->hardwareReg() < 8) {
        kpanic("JitArmV8CodeGen::readCPU");
    }
    // mov reg, [edi+srcOffset]    
    if (width == JitWidth::b32) {
        compiler.ldr(R32(reg), Mem(xCPU, offset));
    } else if (width == JitWidth::b16) {
        compiler.ldrh(R32(reg), Mem(xCPU, offset));
    } else if (width == JitWidth::b8) {
        compiler.ldrb(R32(reg), Mem(xCPU, offset));
#ifdef BOXEDWINE_64
    } else if (width == JitWidth::b64) {
        compiler.ldr(R64(reg), Mem(xCPU, offset));
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
    if (reg->hardwareReg() < 8) {
        kpanic("JitArmV8CodeGen::readCPU");
    }
    if (width == JitWidth::b32) {
        compiler.ldr(R32(reg), createMem(regCPU, sib->hardwareReg(), lsl, offset));
    } else if (width == JitWidth::b16) {
        compiler.ldrh(R32(reg), createMem(regCPU, sib->hardwareReg(), lsl, offset));
    } else if (width == JitWidth::b8) {
        compiler.ldrb(R32(reg), createMem(regCPU, sib->hardwareReg(), lsl, offset));
#ifdef BOXEDWINE_64
    } else if (width == JitWidth::b64) {
        compiler.ldr(R64(reg), createMem(regCPU, sib->hardwareReg(), lsl, offset));
#endif
    } else {
        kpanic_fmt("unknown dstWidth in JitArmV8CodeGen::readCPU %d", width);
    }
    return reg;
}

void JitArmV8CodeGen::writeCPUValue(JitWidth width, RegPtr sib, U8 lsl, U32 offset, DYN_PTR_SIZE src) {
    if (width == JitWidth::b32) {
        RegPtr reg = loadConst(src & 0xffffffff);
        compiler.str(R32(reg), createMem(regCPU, sib->hardwareReg(), lsl, offset));
    } else if (width == JitWidth::b16) {
        RegPtr reg = loadConst(src & 0xffff);
        compiler.strh(R32(reg), createMem(regCPU, sib->hardwareReg(), lsl, offset));
    } else if (width == JitWidth::b8) {
        RegPtr reg = loadConst(src & 0xff);
        compiler.strb(R32(reg), createMem(regCPU, sib->hardwareReg(), lsl, offset));
#ifdef BOXEDWINE_64
    } else if (width == JitWidth::b64) {
        RegPtr reg = loadConst(src);
        compiler.str(R64(reg), createMem(regCPU, sib->hardwareReg(), lsl, offset));
#endif
    } else {
        kpanic_fmt("unknown dstWidth in JitArmV8CodeGen::writeCPU %d", width);
    }
}

void JitArmV8CodeGen::writeCPUValue(JitWidth width, U32 offset, DYN_PTR_SIZE src) {
    if (width == JitWidth::b32) {
        RegPtr reg = loadConst(src & 0xffffffff);
        compiler.str(R32(reg), createMem(regCPU, offset));
    } else if (width == JitWidth::b16) {
        RegPtr reg = loadConst(src & 0xffff);
        compiler.strh(R32(reg), createMem(regCPU, offset));
    } else if (width == JitWidth::b8) {
        RegPtr reg = loadConst(src & 0xff);
        compiler.strb(R32(reg), createMem(regCPU, offset));
#ifdef BOXEDWINE_64
    } else if (width == JitWidth::b64) {
        RegPtr reg = loadConst(src);
        compiler.str(R64(reg), createMem(regCPU, offset));
#endif
    } else {
        kpanic_fmt("unknown dstWidth in JitArmV8CodeGen::writeCPU %d", width);
    }
}

void JitArmV8CodeGen::mov(JitWidth regWidth, RegPtr dest, RegPtr src) {
    if (regWidth == JitWidth::b32) {
        compiler.mov(R32(dest), R32(src));
    } else if (regWidth == JitWidth::b16) {
        compiler.bfxil(R32(dest), R32(src), 0, 16);
    } else if (regWidth == JitWidth::b8) {
        if (isRegHigh(dest) && isRegHigh(src)) {
            RegPtr tmp = getTmpReg();
            mov(regWidth, tmp, src);
            mov(regWidth, dest, tmp);
        } else if (isRegHigh(dest)) {
            compiler.bfi(R32(dest), R32(src), 8, 8);
        } else if (isRegHigh(src)) {
            compiler.bfxil(R32(dest), R32(src), 8, 8);
        } else {
            compiler.bfxil(R32(dest), R32(src), 0, 8);
        }        
#ifdef BOXEDWINE_64
    } else if (regWidth == JitWidth::b64) {
        compiler.mov(R64(dest), R64(src));
#endif
    } else {
        kpanic_fmt("JitArmV8CodeGen::mov unexpected width: %d", (U32)regWidth);
    }
}

void JitArmV8CodeGen::movValue(JitWidth regWidth, RegPtr dst, DYN_PTR_SIZE imm) {
    if (regWidth == JitWidth::b32) {
        compiler.mov(R32(dst), (U32)imm);
    } else if (regWidth == JitWidth::b16) {
        compiler.movk(R32(dst), (U16)imm, 0);
    } else if (regWidth == JitWidth::b8) {
        mov(regWidth, dst, loadConst(imm));
#ifdef BOXEDWINE_64
    } else if (regWidth == JitWidth::b64) {
        compiler.mov(R64(dst), imm);
#endif
    } else {
        kpanic_fmt("JitArmV8CodeGen::mov unexpected width: %d", (U32)regWidth);
    }
}

void JitArmV8CodeGen::movzx(JitWidth dstWidth, RegPtr dest, JitWidth srcWidth, RegPtr src) {
    if (dstWidth == JitWidth::b32) {
        if (srcWidth == JitWidth::b16) {
            compiler.ubfx(R32(dest), R32(src), 0, 16);
        } else if (srcWidth == JitWidth::b32) {
            compiler.mov(R32(dest), R32(src));
        } else if (srcWidth == JitWidth::b8) {
            if (isRegHigh(src)) {
                compiler.ubfx(R32(dest), R32(src), 8, 8);
            } else {
                compiler.ubfx(R32(dest), R32(src), 0, 8);
            }
            dest->isHigh = false; // for when src == dest
        } else {
            kpanic_fmt("unknown width in JitArmV8CodeGen::movzx %d <= %d", dstWidth, srcWidth);
        }
    } else if (dstWidth == JitWidth::b16) {
        if (srcWidth == JitWidth::b8) {
            RegPtr tmp = getTmpReg();
            if (isRegHigh(src)) {
                compiler.ubfx(R32(tmp), R32(src), 8, 8);
            } else {
                compiler.ubfx(R32(tmp), R32(src), 0, 8);
            }
            compiler.bfxil(R32(dest), R32(tmp), 0, 16);
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
            compiler.sxth(R32(dest), R32(src));
        } else if (srcWidth == JitWidth::b8) {
            if (isRegHigh(src)) {
                compiler.sbfx(R32(dest), R32(src), 8, 8);
            } else {
                compiler.sxtb(R32(dest), R32(src));
            }
            dest->isHigh = false; // for when src == dest
        } else {
            kpanic_fmt("unknown width in JitArmV8CodeGen::movsx %d <= %d", dstWidth, srcWidth);
        }
    } else if (dstWidth == JitWidth::b16) {
        if (srcWidth == JitWidth::b8) {
            RegPtr tmp = getTmpReg();
            if (isRegHigh(src)) {
                compiler.sbfx(R32(tmp), R32(src), 8, 8);
            } else {
                compiler.sxtb(R32(tmp), R32(src));
            }
            compiler.bfxil(R32(dest), R32(tmp), 0, 16);
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
        compiler.uxtb(R64(index), R64(param.reg));
        break;
    case JitCallParamType::REG_16:
        compiler.uxth(R64(index), R64(param.reg));
        break;
    case JitCallParamType::REG_32:
        compiler.mov(R64(index), R64(param.reg));
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

void JitArmV8CodeGen::callHostFunction(void* address, const std::vector<DynParam>& params, bool restoreCache, bool saveCache) {
    if (saveCache) {
        compiler.blr(xWriteCacheToCPU);
    }
    setParams(params);    

    compiler.mov(xBranch, (U64)address);
    compiler.blr(xBranch);

    if (restoreCache) {
        compiler.blr(xLoadCacheFromCPU);
    }
}

void JitArmV8CodeGen::nakedCall(RegPtr reg) {
    compiler.blr(R64(reg));
}

void JitArmV8CodeGen::nakedReturn() {
    compiler.ret(asmjit::a64::x30);
}

void JitArmV8CodeGen::callHostFunctionWithResult(RegPtr result, void* address, const std::vector<DynParam>& params) {
    compiler.blr(xWriteCacheToCPU);

    setParams(params);

    compiler.mov(xBranch, (U64)address);
    compiler.blr(xBranch);
    compiler.mov(R64(result), xEAX);
    compiler.blr(xLoadCacheFromCPU);
}

void JitArmV8CodeGen::If(JitWidth regWidth, RegPtr reg) {
    Label label = compiler.new_label();
    ifLabels.push_back(label);

    if (regWidth == JitWidth::b32) {
        compiler.cbz(R32(reg), label);
    } else if (regWidth == JitWidth::b16) {
        RegPtr tmp = getTmpReg();
        compiler.ubfx(R32(tmp), R32(reg), 0, 16);
        compiler.cbz(R32(tmp), label);
    } else if (regWidth == JitWidth::b8) {
        RegPtr tmp = getTmpReg();
        if (isRegHigh(reg)) {
            compiler.ubfx(R32(tmp), R32(reg), 8, 8);
        } else {
            compiler.ubfx(R32(tmp), R32(reg), 0, 8);
        }
        compiler.cbz(R32(tmp), label);
#ifdef BOXEDWINE_64
    } else if (regWidth == JitWidth::b64) {
        compiler.cbz(R64(reg), label);
#endif
    } else {
        kpanic_fmt("JitArmV8CodeGen::If unexpected width: %d", (U32)regWidth);
    }
}

void JitArmV8CodeGen::IfTest(JitWidth regWidth, RegPtr reg, RegPtr mask) {
    if (regWidth == JitWidth::b32) {
        compiler.tst(R32(reg), R32(mask));
    } else if (regWidth == JitWidth::b16 || regWidth == JitWidth::b8) {
        RegPtr left = getTmpReg();
        RegPtr right = getTmpReg();
        movzx(JitWidth::b32, left, regWidth, reg);
        movzx(JitWidth::b32, right, regWidth, mask);
        compiler.tst(R32(left), R32(right));
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
        compiler.tst(R32(reg), R32(imm));
    } else {
        compiler.tst(R32(reg), value);
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
    compiler.tbnz(R32(reg), bitPos, label);
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
    compiler.tbz(R32(reg), bitPos, label);
}

void JitArmV8CodeGen::cmp(JitWidth regWidth, RegPtr reg1, RegPtr reg2) {
    if (regWidth == JitWidth::b32) {
        compiler.cmp(R32(reg1), R32(reg2));
    } else if (regWidth == JitWidth::b64) {
        compiler.cmp(R64(reg1), R64(reg2));
    } else if (regWidth == JitWidth::b16) {
        // JitCodeGen::getCF requires not getting a new tmp reg
        // xBranch should be safe to clobber
        compiler.lsl(wBranch, R32(reg1), 16);
        compiler.lsl(wScratch, R32(reg2), 16);
        compiler.cmp(wBranch, wScratch);
    } else if (regWidth == JitWidth::b8) {
        // JitCodeGen::getCF requires not getting a new tmp reg
        // xBranch should be safe to clobber
        if (reg1->isHigh) {
            compiler.lsr(wBranch, R32(reg1), 8);
            compiler.lsl(wBranch, wBranch, 24);
        } else {
            compiler.lsl(wBranch, R32(reg1), 24);
        }
        if (reg2->isHigh) {
            compiler.lsr(wScratch, R32(reg2), 8);
            compiler.lsl(wScratch, wScratch, 24);
        } else {
            compiler.lsl(wScratch, R32(reg2), 24);
        }
        compiler.cmp(wBranch, wScratch);
    } else {
        kpanic_fmt("JitArmV8CodeGen::cmp unexpected width: %d", (U32)regWidth);
    }
}

void JitArmV8CodeGen::cmp(JitWidth regWidth, RegPtr reg, DYN_PTR_SIZE value) {
    if (!asmjit::a64::Utils::is_add_sub_imm(value)) {
        cmp(regWidth, reg, loadConst(value));
        return;
    }
    if (regWidth == JitWidth::b32) {
        compiler.cmp(R32(reg), value);
    } else if (value == 0 && regWidth == JitWidth::b16) {
        compiler.lsl(wBranch, R32(reg), 16);
        compiler.cmp(wBranch, 0);
    } else if (value == 0 && regWidth == JitWidth::b8) {
        if (reg->isHigh) {
            compiler.lsr(wBranch, R32(reg), 8);
            compiler.lsl(wBranch, wBranch, 24);
        } else {
            compiler.lsl(wBranch, R32(reg), 24);
        }
        compiler.cmp(wBranch, 0);
    } else {
        cmp(regWidth, reg, loadConst(value));
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
        compiler.cbnz(R32(reg), label);
    } else if (regWidth == JitWidth::b16) {
        RegPtr tmp = getTmpReg();
        compiler.ubfx(R32(tmp), R32(reg), 0, 16);
        compiler.cbnz(R32(tmp), label);
    } else if (regWidth == JitWidth::b8) {
        RegPtr tmp = getTmpReg();
        if (isRegHigh(reg)) {
            compiler.ubfx(R32(tmp), R32(reg), 8, 8);
        } else {
            compiler.ubfx(R32(tmp), R32(reg), 0, 8);
        }
        compiler.cbnz(R32(tmp), label);
#ifdef BOXEDWINE_64
    } else if (regWidth == JitWidth::b64) {
        compiler.cbnz(R64(reg), label);
#endif
    } else {
        kpanic_fmt("JitArmV8CodeGen::If unexpected width: %d", (U32)regWidth);
    }
}

RegPtr JitArmV8CodeGen::calculateEaa(DecodedOp* op, U32 popEspAmount) {
    if (op->ea16) {
        return JitCodeGen::calculateEaa(op, popEspAmount);
    } else {
        // [r1 + r2 << 2] will be 1 instruction instead of 3 (mov, shl, add) in JitCodeGen::calculateEaa 
        RegPtr result;
        U32 disp = op->data.disp;
        RegPtr base;

        if (op->base < 6 && KThread::currentThread()->process->hasSetSeg[op->base]) {
            base = getReadOnlySegAddress(op->base);
        }
        if (popEspAmount && op->rm == 4 && op->sibIndex == 4) {
            disp += popEspAmount * 2;
        } else if (popEspAmount && (op->rm == 4 || op->sibIndex == 4)) {
            disp += popEspAmount;
        }

        if (op->sibIndex != 8) {
            if (op->sibScale) {
                result = getTmpReg();
                if (op->rm != 8) {                    
                    compiler.add(R32(result), R32(getReadOnlyReg(op->rm)), R32(getReadOnlyReg(op->sibIndex)), asmjit::Imm(asmjit::a64::Shift(asmjit::a64::ShiftOp::kLSL, op->sibScale)));
                    if (disp) {
                        addValue(JitWidth::b32, result, disp);
                    }
                } else if (disp) {
                    if (!asmjit::a64::Utils::is_add_sub_imm(disp)) {
                        compiler.add(R32(result), R32(loadConst(disp)), R32(getReadOnlyReg(op->sibIndex)), asmjit::Imm(asmjit::a64::Shift(asmjit::a64::ShiftOp::kLSL, op->sibScale)));
                    } else {
                        compiler.lsl(R32(result), R32(getReadOnlyReg(op->sibIndex)), op->sibScale);
                        compiler.add(R32(result), R32(result), disp);                                
                    }
                } else if (base) {
                    compiler.add(R32(result), R32(base), R32(getReadOnlyReg(op->sibIndex)), asmjit::Imm(asmjit::a64::Shift(asmjit::a64::ShiftOp::kLSL, op->sibScale)));
                    base = nullptr;
                } else {
                    compiler.lsl(R32(result), R32(getReadOnlyReg(op->sibIndex)), op->sibScale);
                }
            } else {
                if (op->rm != 8) {
                    result = getTmpReg();
                    compiler.add(R32(result), R32(getReadOnlyReg(op->rm)), R32(getReadOnlyReg(op->sibIndex)));
                    if (disp) {
                        addValue(JitWidth::b32, result, disp);
                    }
                } else if (disp) {
                    result = getTmpReg();
                    if (!asmjit::a64::Utils::is_add_sub_imm(disp)) {
                        compiler.add(R32(result), R32(getReadOnlyReg(op->sibIndex)), R32(loadConst(disp)));
                    } else {
                        compiler.add(R32(result), R32(getReadOnlyReg(op->sibIndex)), disp);
                    }
                } else if (base) {
                    result = getTmpReg();
                    compiler.add(R32(result), R32(getReadOnlyReg(op->sibIndex)), R32(base));
                    base = nullptr;
                } else {
                    result = getTmpReg(op->sibIndex);
                }
            }
        } else if (op->rm != 8) {            
            if (!disp) {
                if (base) {
                    result = getTmpReg();
                    compiler.add(R32(result), R32(getReadOnlyReg(op->rm)), R32(base));
                    base = nullptr;
                } else {
                    result = getTmpReg(op->rm);
                }
            } else {
                result = getTmpReg();
                if (!asmjit::a64::Utils::is_add_sub_imm(disp)) {
                    compiler.add(R32(result), R32(getReadOnlyReg(op->rm)), R32(loadConst(disp)));
                } else {
                    compiler.add(R32(result), R32(getReadOnlyReg(op->rm)), disp);
                }
            }
        } else if (disp) {
            result = getTmpReg();
            if (base) {
                if (!asmjit::a64::Utils::is_add_sub_imm(disp)) {
                    compiler.add(R32(result), R32(base), R32(loadConst(disp)));
                } else {
                    compiler.add(R32(result), R32(base), disp);
                }
                base = nullptr;
            } else {
                movValue(JitWidth::b32, result, disp);
            }
        } else {
            result = getTmpReg();
            xorReg(JitWidth::b32, result, result);
        }

        // seg[6] is always 0
        if (base) {
            addReg(JitWidth::b32, result, base);
        }
        return result;
    }
}

void JitArmV8CodeGen::IfNotCPU(JitWidth regWidth, RegPtr sib, U8 lsl, U32 offset) {
    RegPtr tmp = readCPU(regWidth, sib, lsl, offset);
    IfNot(regWidth, tmp);
}

void JitArmV8CodeGen::clearMMUPermissionIfSpansPage(JitWidth width, RegPtr offset, RegPtr reg) {
    if (width == JitWidth::b16) {
        compiler.cmp(R32(offset), 0xFFF);
    } else if (width == JitWidth::b32) {
        compiler.cmp(R32(offset), 0xFFD);
    } else if (width == JitWidth::b64) {
        compiler.cmp(R32(offset), 0xFF9);
    } else if (width == JitWidth::b128) {
        compiler.cmp(R32(offset), 0xFF1);
    } else if (width == JitWidth::b256) {
        compiler.cmp(R32(offset), 0xFE1);
    } else {
        kpanic_fmt("JitArmV8CodeGen::clearRegIfSpansPage unknown width %d", (U32)width);
    }

    compiler.csel(R64(reg), R64(reg), asmjit::a64::xzr, asmjit::a64::CondCode::kLT);
    // csel improved Quake 2 from 47fps to 50fps on snapdragon x
    //Label label = compiler.new_label();
    //compiler.b_lt(label);
    //compiler.mov(R64(reg), 0);
    //compiler.bind(label);
}

void JitArmV8CodeGen::JumpIfCondition(JitConditional condition, U32 address) {    
    Label label;

    bool negative = false;
    RegPtr reg;

    preIfCondition(condition, negative, reg);

    if (!opLabels.get(address, label)) {
        label = compiler.new_label();
        opLabels.set(address, label);    
    }   
    if (negative) {
        compiler.cbz(R32(reg), label);
    } else {
        compiler.cbnz(R32(reg), label);
    }
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

void JitArmV8CodeGen::jmpHost(RegPtr reg) {
    compiler.br(R64(reg));
}

void JitArmV8CodeGen::jmpHost(DYN_PTR_SIZE address) {
    compiler.mov(xTmp9, address);
    compiler.br(xTmp9);
}

void JitArmV8CodeGen::writeCPU(JitWidth width, U32 offset, RegPtr src) {
    if (width == JitWidth::b32) {
        compiler.str(R32(src), createMem(regCPU, offset));
    } else if (width == JitWidth::b16) {
        compiler.strh(R32(src), createMem(regCPU, offset));
    } else if (width == JitWidth::b8) {
        compiler.strb(R32(getReg8InLowByte(src)), createMem(regCPU, offset));
#ifdef BOXEDWINE_64
    } else if (width == JitWidth::b64) {
        compiler.str(R64(src), createMem(regCPU, offset));
#endif
    } else {
        kpanic_fmt("unknown dstWidth in JitArmV8CodeGen::writeCPU %d", width);
    }
}

void JitArmV8CodeGen::writeCPU(JitWidth width, RegPtr sib, U8 lsl, U32 offset, RegPtr src) {
    if (width == JitWidth::b32) {
        compiler.str(R32(src), createMem(regCPU, sib->hardwareReg(), lsl, offset));
    } else if (width == JitWidth::b16) {
        compiler.strh(R32(src), createMem(regCPU, sib->hardwareReg(), lsl, offset));
    } else if (width == JitWidth::b8) {
        compiler.strb(R32(getReg8InLowByte(src)), createMem(regCPU, sib->hardwareReg(), lsl, offset));
#ifdef BOXEDWINE_64
    } else if (width == JitWidth::b64) {
        compiler.str(R64(src), createMem(regCPU, sib->hardwareReg(), lsl, offset));
#endif
    } else {
        kpanic_fmt("unknown dstWidth in JitArmV8CodeGen::writeCPU %d", width);
    }
}

Vec toSse8(SSERegPtr reg) {
    return Vec::make_b(reg->hardwareReg());
}

Vec toSse16(SSERegPtr reg) {
    return Vec::make_h(reg->hardwareReg());
}

Vec toSse32(SSERegPtr reg) {
    return Vec::make_s(reg->hardwareReg());
}

Vec toSse64(SSERegPtr reg) {
    return Vec::make_d(reg->hardwareReg());
}

Vec toSse128(SSERegPtr reg) {
    return Vec::make_q(reg->hardwareReg());
}

Vec toSse128(int reg) {
    return Vec::make_q(reg);
}

Vec toSseB(SSERegPtr reg) {
    return Vec::make_b(reg->hardwareReg());
}

Vec toSseB8(SSERegPtr reg) {
    return Vec::make_b(reg->hardwareReg()).b8();
}

Vec toSseB16(SSERegPtr reg) {
    return Vec::make_b(reg->hardwareReg()).b16();
}

Vec toSseB16(SSERegPtr reg, U8 index) {
    Vec result = Vec::make_b(reg->hardwareReg()).b16();
    result.set_element_index(index);
    return result;
}

Vec toSseH4(SSERegPtr reg) {
    return Vec::make_h(reg->hardwareReg()).h4();
}

Vec toSseH8(SSERegPtr reg) {
    return Vec::make_h(reg->hardwareReg()).h8();
}

Vec toSseH8(SSERegPtr reg, U8 index) {
    Vec result = Vec::make_h(reg->hardwareReg()).h8();
    result.set_element_index(index);
    return result;
}

Vec toSseS(SSERegPtr reg) {
    return Vec::make_s(reg->hardwareReg()).s();
}

Vec toSseS2(SSERegPtr reg) {
    return Vec::make_s(reg->hardwareReg()).s2();
}

Vec toSseS4(SSERegPtr reg) {
    return Vec::make_s(reg->hardwareReg()).s4();
}

Vec toSseS4(SSERegPtr reg, U8 index) {
    Vec result = Vec::make_s(reg->hardwareReg()).s4();
    result.set_element_index(index);
    return result;
}

Vec toSseD(SSERegPtr reg) {
    return Vec::make_d(reg->hardwareReg()).d();
}

Vec toSseD2(SSERegPtr reg) {
    return Vec::make_d(reg->hardwareReg()).d2();
}

Vec toSseD2(U32 reg) {
    return Vec::make_d(reg).d2();
}

Vec toSseD2(SSERegPtr reg, U8 index) {
    Vec result = Vec::make_d(reg->hardwareReg()).d2();
    result.set_element_index(index);
    return result;
}

void JitArmV8CodeGen::IfSseLessThan(SSERegPtr src1, SSERegPtr src2) {
    compiler.fcmp(toSseS(src1), toSseS(src2));
    IfLessThan();
}

void JitArmV8CodeGen::fmax(SSERegPtr dst, SSERegPtr src1, SSERegPtr src2, MakeSSE vMake) {
    SSERegPtr vTmpReg = getTmpSSE();
    SSERegPtr vTmpReg2 = getTmpSSE();    

    if (dst->hardwareReg() != src2->hardwareReg()) {
        // get the min values
        compiler.fmaxnm(vMake(vTmpReg), vMake(src1), vMake(src2));

        // see if they are equal (like -0.0f == 0.0f) or 
        // if they are neither greather than or less than (because NaN was used)
        // in both those cases, SSE says to use the one on the right
        compiler.fcmgt(vMake(vTmpReg2), vMake(src1), vMake(src2));
        compiler.fcmgt(vMake(dst), vMake(src2), vMake(src1));
        compiler.orr(toSse128(dst), toSse128(dst), toSse128(vTmpReg2));


        // if the above comparisons are true, then 1's will be in reg, other wise 0's
        // when doing this call, the min values will be used as calculated for the 1's
        // for the 0's, the right side will be used (nan's and equal values)
        compiler.bsl(toSse128(dst), toSse128(vTmpReg), toSse128(src2));
    } else {
        SSERegPtr vTmpReg3 = getTmpSSE();
        compiler.fmaxnm(vMake(vTmpReg), vMake(src1), vMake(src2));
        compiler.fcmgt(vMake(vTmpReg2), vMake(src1), vMake(src2));
        compiler.fcmgt(vMake(vTmpReg3), vMake(src2), vMake(src1));
        compiler.orr(toSse128(vTmpReg3), toSse128(vTmpReg3), toSse128(vTmpReg2));
        compiler.bsl(toSse128(vTmpReg3), toSse128(vTmpReg), toSse128(src2));
        compiler.mov(toSse128(dst), toSse128(vTmpReg3));
    }
}

void JitArmV8CodeGen::fmin(SSERegPtr dst, SSERegPtr src1, SSERegPtr src2, MakeSSE vMake) {
    SSERegPtr vTmpReg = getTmpSSE();
    SSERegPtr vTmpReg2 = getTmpSSE();

    if (dst->hardwareReg() != src2->hardwareReg()) {
        // get the min values
        compiler.fminnm(vMake(vTmpReg), vMake(src1), vMake(src2));

        // see if they are equal (like -0.0f == 0.0f) or 
        // if they are neither greather than or less than (because NaN was used)
        // in both those cases, SSE says to use the one on the right
        compiler.fcmgt(vMake(vTmpReg2), vMake(src1), vMake(src2));
        compiler.fcmgt(vMake(dst), vMake(src2), vMake(src1));
        compiler.orr(toSse128(dst), toSse128(dst), toSse128(vTmpReg2));


        // if the above comparisons are true, then 1's will be in reg, other wise 0's
        // when doing this call, the min values will be used as calculated for the 1's
        // for the 0's, the right side will be used (nan's and equal values)
        compiler.bsl(toSseB16(dst), toSseB16(vTmpReg), toSseB16(src2));
    } else {
        SSERegPtr vTmpReg3 = getTmpSSE();
        compiler.fminnm(vMake(vTmpReg), vMake(src1), vMake(src2));
        compiler.fcmgt(vMake(vTmpReg2), vMake(src1), vMake(src2));
        compiler.fcmgt(vMake(vTmpReg3), vMake(src2), vMake(src1));
        compiler.orr(toSse128(vTmpReg3), toSse128(vTmpReg3), toSse128(vTmpReg2));
        compiler.bsl(toSseB16(vTmpReg3), toSseB16(vTmpReg), toSseB16(src2));
        compiler.mov(toSse128(dst), toSse128(vTmpReg3));
    }
}

void JitArmV8CodeGen::sseConvertFloatToInt(Vec dst, Vec src, bool truncate) {
    if (truncate) {
        compiler.fcvtzs(dst, src);
    } else {
        RegPtr round = readCPU(JitWidth::b32, offsetof(CPU, fpu.round));
        IfNot(JitWidth::b32, round); { // ROUND_Nearest
            compiler.fcvtns(dst, src);
        } StartElse(); {
            IfEqual(JitWidth::b32, round, ROUND_Down); {
                compiler.fcvtms(dst, src);
            } StartElse(); {
                IfEqual(JitWidth::b32, round, ROUND_Up); {
                    compiler.fcvtps(dst, src);
                } StartElse(); {
                    compiler.fcvtzs(dst, src);
                } EndIf();
            } EndIf();
        } EndIf();
    }
}

void JitArmV8CodeGen::sseConvertFloatToInt(RegPtr dst, Vec src, bool truncate) {
    if (truncate) {
        compiler.fcvtzs(R32(dst), src);
    } else {
        U32 roundingMode = (cpu->mxcsr >> 13) & 2;
        if (roundingMode == ROUND_Nearest) {
            compiler.fcvtns(R32(dst), src);
        } else if (roundingMode == ROUND_Down) {
            compiler.fcvtms(R32(dst), src);
        } else if (roundingMode == ROUND_Up) {
            compiler.fcvtps(R32(dst), src);
        } else if (roundingMode == ROUND_Chop) {
            compiler.fcvtzs(R32(dst), src);
        }
    }
}

void JitArmV8CodeGen::cvtsd2si(RegPtr dst, SSERegPtr src, bool truncate) {
    // cpu->reg[r1].u32 = cpu->xmm[rm].pd.f64[0];

    //if (cpu->xmm[rm].pd.f64[0] >= 2147483648.0 || cpu->xmm[rm].pd.f64[0] <= -2147483649.0) {
    //    cpu->reg[r1].u32 = 0x80000000;
    //} else {
    //    cpu->reg[r1].u32 = (int32_t)cpu->xmm[rm].pd.f64[0];
    //}    

    SSERegPtr vPlusOne = loadSSEConst(SSE_MAX_INT32_PLUS_ONE_AS_DOUBLE);
    SSERegPtr vMinusOne = loadSSEConst(SSE_MIN_INT32_MINUS_ONE_AS_DOUBLE);

    SSERegPtr vTmpReg1 = getTmpSSE();
    SSERegPtr vTmpReg2 = getTmpSSE();
    RegPtr tmpReg = getTmpReg();
    RegPtr tmpReg2 = getTmpReg();

    // will set lane to all 1's if value is too large or too small to convert
    compiler.fcmge(toSseD2(vTmpReg1), toSseD2(src), toSseD2(vPlusOne));
    compiler.fcmge(toSseD2(vTmpReg2), toSseD2(vMinusOne), toSseD2(src));
    compiler.orr(toSse128(vTmpReg1), toSse128(vTmpReg1), toSse128(vTmpReg2));

    compiler.umov(R32(tmpReg), toSseS4(vTmpReg1, 0));

    // do the convert from double to int32
    sseConvertFloatToInt(tmpReg2, toSseD(src), truncate);

    // clear out int32 if was too large or too small
    compiler.bic(R32(tmpReg2), R32(tmpReg2), R32(tmpReg));

    // if the value was too large or too smaller, then tmpReg contains all 1's.  
    compiler.and_(R32(tmpReg), R32(tmpReg), 0x80000000);

    // will be good value or'd with 0, or bad value (0), or'd with 0x80000000
    compiler.orr(R32(dst), R32(tmpReg2), R32(tmpReg));
}

void JitArmV8CodeGen::cvtpd2pi(SSERegPtr dst, SSERegPtr src, bool truncate) {
    // cpu->reg_mmx[reg].sd.d0 = cpu->xmm[rm].pd.f64[0];
    // cpu->reg_mmx[reg].sd.d1 = cpu->xmm[rm].pd.f64[1];

    //if (cpu->xmm[rm].pd.f64[0] >= 2147483648.0 || cpu->xmm[rm].pd.f64[0] <= -2147483649.0) {
    //    cpu->reg_mmx[reg].sd.d0 = 0x80000000;
    //} else {
    //    cpu->reg_mmx[reg].sd.d0 = (int32_t)cpu->xmm[rm].pd.f64[0];
    //}    

    SSERegPtr vPlusOne = loadSSEConst(SSE_MAX_INT32_PLUS_ONE_AS_DOUBLE);
    SSERegPtr vMinusOne = loadSSEConst(SSE_MIN_INT32_MINUS_ONE_AS_DOUBLE);

    SSERegPtr vTmpReg1 = getTmpSSE();
    SSERegPtr vTmpReg2 = getTmpSSE();


    // will set lane to all 1's if value is too large or too small to convert
    compiler.fcmge(toSseD2(vTmpReg1), toSseD2(src), toSseD2(vPlusOne));
    compiler.fcmge(toSseD2(vTmpReg2), toSseD2(vMinusOne), toSseD2(src));
    compiler.orr(toSse128(vTmpReg1), toSse128(vTmpReg1), toSse128(vTmpReg2));

    sseConvertFloatToInt(toSseD2(vTmpReg2), toSseD2(src), truncate);

    // clear out int64's if was too large or too small
    compiler.bic(toSse128(vTmpReg2), toSse128(vTmpReg2), toSse128(vTmpReg1));

    // do the convert from int64 to int32
    compiler.xtn(toSseS2(vTmpReg2), toSseD2(vTmpReg2));

    // if the value was too large or too smaller, then vTmpReg1 contains all 1's.  
    // Right shift 63, convert 64-bit to 32-bit, left shift by 31 to create 0x80000000
    compiler.ushr(toSseD2(vTmpReg1), toSseD2(vTmpReg1), 63);
    compiler.xtn(toSseS2(vTmpReg1), toSseD2(vTmpReg1));
    compiler.shl(toSseS2(vTmpReg1), toSseS2(vTmpReg1), 31);

    // vTmpReg1 will be 0x80000000 or 0x00000000 depending on if the value was too large or too small
    // vTmpReg2 will be the actual result or 0x00000000 (if too big or too small)
    compiler.orr(toSse64(dst), toSse64(vTmpReg2), toSse64(vTmpReg1));
}

// When converting Double/Float to int32, if the value doesn't fit, SSE convert the value to 0x80000000, but ARM converts it to 0x7FFFFFFF,
// so we have to spend a lot of effort to correct that
void JitArmV8CodeGen::cvtps2pi(SSERegPtr dst, SSERegPtr src, bool truncate) {
    //if (cpu->xmm[rm].pd.f32[0] >= 2147483648.0 || cpu->xmm[rm].pd.f32[0] <= -2147483649.0) {
    //    cpu->reg_mmx[reg].sd.d0 = 0x80000000;
    //} else {
    //    cpu->reg_mmx[reg].sd.d0 = (int32_t)cpu->xmm[rm].pd.f32[0];
    //} 

    SSERegPtr vPlusOne = loadSSEConst(SSE_MAX_INT32_PLUS_ONE_AS_FLOAT);
    SSERegPtr vMinusOne = loadSSEConst(SSE_MIN_INT32_MINUS_ONE_AS_FLOAT);

    SSERegPtr vTmpReg1 = getTmpSSE();
    SSERegPtr vTmpReg2 = getTmpSSE();

    // will set lane to all 1's if value is too large or too small to convert
    compiler.fcmge(toSseS4(vTmpReg1), toSseS4(src), toSseS4(vPlusOne));
    compiler.fcmge(toSseS4(vTmpReg2), toSseS4(vMinusOne), toSseS4(src));
    compiler.orr(toSse128(vTmpReg1), toSse128(vTmpReg1), toSse128(vTmpReg2));

    // do the convert from float to int32
    sseConvertFloatToInt(toSseS4(vTmpReg2), toSseS4(src), truncate);

    // clear out int32's if was too large or too small
    compiler.bic(toSse128(vTmpReg2), toSse128(vTmpReg2), toSse128(vTmpReg1));

    // if the value was too large or too smaller, then vTmpReg1 contains all 1's.  
    // Right shift 31, left shift by 31 to create 0x80000000
    compiler.ushr(toSseS4(vTmpReg1), toSseS4(vTmpReg1), 31);
    compiler.shl(toSseS4(vTmpReg1), toSseS4(vTmpReg1), 31);

    // vTmpReg1 will be 0x80000000 or 0x00000000 depending on if the value was too large or too small
    // vTmpReg2 will be the actual result or 0x00000000 (if too big or too small)
    compiler.orr(toSse128(dst), toSse128(vTmpReg2), toSse128(vTmpReg1));
}

void JitArmV8CodeGen::sseCmp(SSERegPtr dst, SSERegPtr src1, SSERegPtr src2, U8 pred, MakeSSE vMake) {
    switch (pred) {
    case 0: // eq
        compiler.fcmeq(vMake(dst), vMake(src1), vMake(src2));
        break;
    case 1: // lt
        compiler.fcmgt(vMake(dst), vMake(src2), vMake(src1));
        break;
    case 2: // le
        compiler.fcmge(vMake(dst), vMake(src2), vMake(src1));
        break;
    case 3: // unord
    {
        SSERegPtr vTmpReg = getTmpSSE();
        SSERegPtr vTmpReg2 = getTmpSSE();
        compiler.fcmeq(vMake(vTmpReg), vMake(src1), vMake(src1));
        compiler.fcmeq(vMake(vTmpReg2), vMake(src2), vMake(src2));
        compiler.and_(toSse128(vTmpReg), toSse128(vTmpReg2), toSse128(vTmpReg));
        compiler.not_(toSse128(dst), toSse128(vTmpReg));
        break;
    }
    case 4: // neq
        compiler.fcmeq(vMake(dst), vMake(src1), vMake(src2));
        compiler.not_(toSse128(dst), toSse128(dst));
        break;
    case 5: // nlt
        compiler.fcmge(vMake(dst), vMake(src1), vMake(src2));
        break;
    case 6: // nle
        compiler.fcmgt(vMake(dst), vMake(src1), vMake(src2));
        break;
    case 7: // ord
    {
        SSERegPtr vTmpReg = getTmpSSE();
        SSERegPtr vTmpReg2 = getTmpSSE();
        compiler.fcmeq(vMake(vTmpReg), vMake(src1), vMake(src1));
        compiler.fcmeq(vMake(vTmpReg2), vMake(src2), vMake(src2));
        compiler.and_(toSse128(dst), toSse128(vTmpReg2), toSse128(vTmpReg));
        break;
    }
    default:
        kpanic("JitArmV8CodeGen::sseCmp");
    }
}

void JitArmV8CodeGen::comis(SSERegPtr dst, SSERegPtr src, MakeSSE vMake) {
    // cpu->flags &= ~(AF | OF | SF | CF | PF | ZF);
    // if (isnan(a.f64[0]) || isnan(b.f64[0])) {
    //     cpu->flags |= CF | ZF | PF;
    // } else if (a.f64[0] == b.f64[0]) {
    //     cpu->flags |= ZF;
    // } else if (a.f64[0] < b.f64[0]) {
    //     cpu->flags |= CF;
    // }
    RegPtr tmpReg = getTmpReg();

    // cpu->flags &= ~(AF | OF | SF | CF | PF | ZF);
    compiler.movn(R32(tmpReg), AF | OF | SF | CF | PF | ZF);
    compiler.and_(xFLAGS, xFLAGS, R32(tmpReg));
    
    compiler.fcmp(vMake(dst), vMake(src));

    // if (isnan(a.f64[0]) || isnan(b.f64[0])) { cpu->flags |= CF | ZF | PF; }
    compiler.mov(R32(tmpReg), CF | PF | ZF);
    compiler.orr(R32(tmpReg), xFLAGS, R32(tmpReg));
    compiler.csel(xFLAGS, R32(tmpReg), xFLAGS, asmjit::a64::CondCode::kVS);

    // else if (a.f64[0] == b.f64[0]) { cpu->flags |= ZF; }
    compiler.cset(R32(tmpReg), asmjit::a64::CondCode::kEQ);
    compiler.orr(xFLAGS, xFLAGS, R32(tmpReg), asmjit::Imm(asmjit::a64::Shift(asmjit::a64::ShiftOp::kLSL, 6))); // 6 for ZF

    // else if (a.f64[0] < b.f64[0]) {cpu->flags |= CF; }
    compiler.cset(R32(tmpReg), asmjit::a64::CondCode::kLT);
    compiler.orr(xFLAGS, xFLAGS, R32(tmpReg));
    storeLazyFlagType(FLAGS_NONE);
    currentLazyFlags = FLAGS_NONE;
}

void JitArmV8CodeGen::storeCpuXMMReg(SSERegPtr reg, U32 index) {
    if (index >= 8) {
        kpanic("JitArmV8CodeGen::storeCpuXMMReg");
        return;
    }
    if (xmmCache[index] == INVALID_REG) {
        compiler.str(toSse128(reg), createMem(regCPU, index * 16 + offsetof(CPU, xmm)));
    } else if (xmmCache[index] != reg->hardwareReg()) {
        compiler.mov(Vec::make_q(xmmCache[index]), toSse128(reg));
    }    
}

void JitArmV8CodeGen::storeXMMToMem128(SSERegPtr reg, RegPtr rm, RegPtr sib) {
    compiler.str(toSse128(reg), createMem(rm, sib, 0, 0));
}

void JitArmV8CodeGen::storeXMMToMem64(SSERegPtr reg, RegPtr rm, RegPtr sib) {
    compiler.str(toSse64(reg), createMem(rm, sib, 0, 0));
}

void JitArmV8CodeGen::storeXMMToMem32(SSERegPtr reg, RegPtr rm, RegPtr sib) {
    compiler.str(toSse32(reg), createMem(rm, sib, 0, 0));
}

void JitArmV8CodeGen::storeHighXMMToMem64(SSERegPtr reg, RegPtr rm, RegPtr sib) {
    RegPtr address = getTmpReg();
    compiler.add(R64(address), R64(rm), R64(sib));
    compiler.st1(toSseD2(reg, 1), createMem(address, 0));
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
        vUsed[p->hardwareReg()] = false;
        delete p;
    });
    if (load && index != INVALID_REG) {
        compiler.ldr(toSse128(result), createMem(regCPU, index * 16 + offsetof(CPU, xmm)));
    }
    return result;
}

SSERegPtr JitArmV8CodeGen::loadCpuXMMReg(U8 index) {        
    return getXMM(index, true);
}

SSERegPtr JitArmV8CodeGen::loadXMMFromMem128(U8 index, RegPtr rm, RegPtr sib) {
    SSERegPtr reg = getXMM(index, false);
    compiler.ldr(toSse128(reg), createMem(rm, sib, 0, 0));
    return reg;
}

SSERegPtr JitArmV8CodeGen::loadXMMFromMem64(U8 index, RegPtr rm, RegPtr sib) {
    SSERegPtr reg = getXMM(index, false);
    compiler.ldr(toSse64(reg), createMem(rm, sib, 0, 0));
    return reg;
}

SSERegPtr JitArmV8CodeGen::loadLowXMMFromMem64(U8 index, RegPtr rm, RegPtr sib) {
    SSERegPtr reg = getXMM(index, true);
    RegPtr address = getTmpReg();
    compiler.add(R64(address), R64(rm), R64(sib));
    compiler.ld1(toSseD2(reg, 0), createMem(address, 0));
    return reg;
}

SSERegPtr JitArmV8CodeGen::loadHighXMMFromMem64(U8 index, RegPtr rm, RegPtr sib) {
    SSERegPtr reg = getXMM(index, true);
    RegPtr address = getTmpReg();
    compiler.add(R64(address), R64(rm), R64(sib));
    compiler.ld1(toSseD2(reg, 1), createMem(address, 0));
    return reg;
}

SSERegPtr JitArmV8CodeGen::loadXMMFromMem32(U8 index, RegPtr rm, RegPtr sib) {
    SSERegPtr reg = getXMM(index, true);
    compiler.ldr(toSseS(reg), createMem(rm, sib, 0, 0));
    return reg;
}

void JitArmV8CodeGen::addpsXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.fadd(toSseS4(dst), toSseS4(dst), toSseS4(src));
}

void JitArmV8CodeGen::addssXmmXmm(SSERegPtr dst, SSERegPtr src) {
    // might clear upper bits
    // compiler.fadd(toSse32(dst), toSse32(dst), toSse32(src));

    SSERegPtr tmp = getTmpSSE();
    compiler.fadd(toSse32(tmp), toSse32(dst), toSse32(src));
    compiler.ins(toSseS4(dst, 0), toSseS4(tmp, 0));
}

void JitArmV8CodeGen::subpsXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.fsub(toSseS4(dst), toSseS4(dst), toSseS4(src));
}

void JitArmV8CodeGen::subssXmmXmm(SSERegPtr dst, SSERegPtr src) {
    SSERegPtr tmp = getTmpSSE();
    compiler.fsub(toSse32(tmp), toSse32(dst), toSse32(src));
    compiler.ins(toSseS4(dst, 0), toSseS4(tmp, 0));
}

void JitArmV8CodeGen::mulpsXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.fmul(toSseS4(dst), toSseS4(dst), toSseS4(src));
}

void JitArmV8CodeGen::mulssXmmXmm(SSERegPtr dst, SSERegPtr src) {
    SSERegPtr tmp = getTmpSSE();
    compiler.fmul(toSse32(tmp), toSse32(dst), toSse32(src));
    compiler.ins(toSseS4(dst, 0), toSseS4(tmp, 0));
}

void JitArmV8CodeGen::divpsXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.fdiv(toSseS4(dst), toSseS4(dst), toSseS4(src));
}

void JitArmV8CodeGen::divssXmmXmm(SSERegPtr dst, SSERegPtr src) {
    SSERegPtr tmp = getTmpSSE();
    compiler.fdiv(toSse32(tmp), toSse32(dst), toSse32(src));
    compiler.ins(toSseS4(dst, 0), toSseS4(tmp, 0));
}

void JitArmV8CodeGen::rcppsXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.frecpe(toSseS4(dst), toSseS4(src));
}

void JitArmV8CodeGen::rcpssXmmXmm(SSERegPtr dst, SSERegPtr src) {
    SSERegPtr tmp = getTmpSSE();
    compiler.frecpe(toSse32(tmp), toSse32(src));
    compiler.ins(toSseS4(dst, 0), toSseS4(tmp, 0));
}

void JitArmV8CodeGen::sqrtpsXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.fsqrt(toSseS4(dst), toSseS4(src));
}

void JitArmV8CodeGen::sqrtssXmmXmm(SSERegPtr dst, SSERegPtr src) {
    SSERegPtr tmp = getTmpSSE();
    compiler.fsqrt(toSse32(tmp), toSse32(src));
    compiler.ins(toSseS4(dst, 0), toSseS4(tmp, 0));
}

void JitArmV8CodeGen::rsqrtpsXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.frsqrte(toSseS4(dst), toSseS4(src));
}

void JitArmV8CodeGen::rsqrtssXmmXmm(SSERegPtr dst, SSERegPtr src) {
    SSERegPtr tmp = getTmpSSE();
    compiler.frsqrte(toSseS(tmp), toSseS(src));
    compiler.ins(toSseS4(dst, 0), toSseS4(tmp, 0));
}

void JitArmV8CodeGen::maxpsXmmXmm(SSERegPtr dst, SSERegPtr src) {
    fmax(dst, dst, src, toSseS4);
}

void JitArmV8CodeGen::maxssXmmXmm(SSERegPtr dst, SSERegPtr src) {
    SSERegPtr tmp = getTmpSSE();
    fmax(tmp, dst, src, toSseS);
    compiler.ins(toSseS4(dst, 0), toSseS4(tmp, 0));
}

void JitArmV8CodeGen::minpsXmmXmm(SSERegPtr dst, SSERegPtr src) {
    fmin(dst, dst, src, toSseS4);
}

void JitArmV8CodeGen::minssXmmXmm(SSERegPtr dst, SSERegPtr src) {
    SSERegPtr tmp = getTmpSSE();
    fmin(tmp, dst, src, toSseS);
    compiler.ins(toSseS4(dst, 0), toSseS4(tmp, 0));
}

Vec toMmxB(MMXRegPtr reg) {
    return Vec::make_b(reg->hardwareReg());
}

Vec toMmxB8(MMXRegPtr reg) {
    return Vec::make_b(reg->hardwareReg()).b8();
}

Vec toMmxB8(MMXRegPtr reg, U8 index) {
    Vec result = Vec::make_b(reg->hardwareReg()).b8();
    result.set_element_index(index);
    return result;
}

Vec toMmxB16(MMXRegPtr reg) {
    return Vec::make_b(reg->hardwareReg()).b16();
}

Vec toMmxH(MMXRegPtr reg) {
    return Vec::make_h(reg->hardwareReg());
}

Vec toMmxH4(MMXRegPtr reg) {
    return Vec::make_h(reg->hardwareReg()).h4();
}

Vec toMmxH4(MMXRegPtr reg, U8 index) {
    Vec result = Vec::make_h(reg->hardwareReg()).h4();
    result.set_element_index(index);
    return result;
}

Vec toMmxH8(MMXRegPtr reg) {
    return Vec::make_h(reg->hardwareReg()).h8();
}

Vec toMmxH8(MMXRegPtr reg, U8 index) {
    Vec result = Vec::make_h(reg->hardwareReg()).h8();
    result.set_element_index(index);
    return result;
}

Vec toMmxS(MMXRegPtr reg) {
    return Vec::make_s(reg->hardwareReg());
}

Vec toMmxS2(MMXRegPtr reg) {
    return Vec::make_s(reg->hardwareReg()).s2();
}

Vec toMmxS2(MMXRegPtr reg, U8 index) {
    Vec result = Vec::make_h(reg->hardwareReg()).s2();
    result.set_element_index(index);
    return result;
}

Vec toMmxS4(MMXRegPtr reg) {
    return Vec::make_s(reg->hardwareReg()).s4();
}

Vec toMmxS4(MMXRegPtr reg, U8 index) {
    Vec result = Vec::make_s(reg->hardwareReg()).s4();
    result.set_element_index(index);
    return result;
}

Vec toMmxD(MMXRegPtr reg) {
    return Vec::make_d(reg->hardwareReg());
}

Vec toMmxD1(MMXRegPtr reg) {
    return Vec::make_d(reg->hardwareReg()).d();
}

Vec toMmx64(MMXRegPtr reg) {
    return Vec::make_d(reg->hardwareReg());
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
    compiler.umov(R32(dst), toMmxH4(src, srcIndex));
}

void JitArmV8CodeGen::pinsrwMmxReg(MMXRegPtr dst, RegPtr src, U8 dstIndex) {
    compiler.ins(toMmxH8(dst, dstIndex & 3), R32(src));
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

    compiler.umov(R32(dst), toMmxB8(tmp, 0));
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

void JitArmV8CodeGen::andnpsXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.bic(toSse128(dst), toSse128(src), toSse128(dst));
}

void JitArmV8CodeGen::andpsXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.and_(toSse128(dst), toSse128(dst), toSse128(src));
}

void JitArmV8CodeGen::orpsXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.orr(toSse128(dst), toSse128(dst), toSse128(src));
}

void JitArmV8CodeGen::xorpsXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.eor(toSse128(dst), toSse128(dst), toSse128(src));
}

void JitArmV8CodeGen::cvtpi2psXmmMmx(SSERegPtr dst, MMXRegPtr src) {
    // cvtpi2ps need to keep top 64-bits of the xmm dst
    SSERegPtr tmp = getTmpSSE();
    compiler.scvtf(toSseS2(tmp), toMmxS2(src));
    compiler.ins(toSseD2(dst, 0), toSseD2(tmp, 0)); // need to maintain the high 64-bits
}

void JitArmV8CodeGen::cvtps2piMmxXmm(MMXRegPtr dst, SSERegPtr src) {
    cvtps2pi(std::make_shared<SSERegInternal>(dst->hardwareReg(), 0xff), src, false);
}

void JitArmV8CodeGen::cvtsi2ssXmmR32(SSERegPtr dst, RegPtr src) {
    SSERegPtr tmp = getTmpSSE();
    compiler.ins(toSseS4(tmp, 0), R32(src));
    compiler.scvtf(toSseS(tmp), toSseS(tmp));
    compiler.ins(toSseS4(dst, 0), toSseS4(tmp, 0)); // need to maintain the high 96-bits
}

void JitArmV8CodeGen::cvtss2siR32Xmm(RegPtr dst, SSERegPtr src) {
    SSERegPtr tmp = getTmpSSE();
    cvtps2pi(tmp, src, false);
    compiler.umov(R32(dst), toSseS4(tmp, 0));
}

void JitArmV8CodeGen::cvttps2piMmxXmm(MMXRegPtr dst, SSERegPtr src) {
    cvtps2pi(std::make_shared<SSERegInternal>(dst->hardwareReg(), 0xff), src, true);
}

void JitArmV8CodeGen::cvttss2siR32Xmm(RegPtr dst, SSERegPtr src) {
    SSERegPtr tmp = getTmpSSE();
    cvtps2pi(tmp, src, true);
    compiler.umov(R32(dst), toSseS4(tmp, 0));
}

void JitArmV8CodeGen::movhlpsXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.ins(toSseD2(dst, 0), toSseD2(src, 1));
}

void JitArmV8CodeGen::movlhpsXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.ins(toSseD2(dst, 1), toSseD2(src, 0));
}

void JitArmV8CodeGen::movssXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.ins(toSseS4(dst, 0), toSseS4(src, 0));
}

void JitArmV8CodeGen::shufpsXmmXmm(SSERegPtr dst, SSERegPtr src, U32 mask) {
    SSERegPtr tmp = getTmpSSE();
    
    compiler.ins(toSseS4(tmp, 0), toSseS4(dst, mask & 3));
    compiler.ins(toSseS4(tmp, 1), toSseS4(dst, (mask >> 2) & 3));
    compiler.ins(toSseS4(tmp, 2), toSseS4(src, (mask >> 4) & 3));
    compiler.ins(toSseS4(tmp, 3), toSseS4(src, (mask >> 6) & 3));

    compiler.mov(toSse128(dst), toSse128(tmp));
}

void JitArmV8CodeGen::cmppsXmmXmm(SSERegPtr dst, SSERegPtr src, U32 imm) {
    sseCmp(dst, dst, src, imm, toSseS4);
}

void JitArmV8CodeGen::cmpssXmmXmm(SSERegPtr dst, SSERegPtr src, U32 imm) {
    SSERegPtr tmp = getTmpSSE();
    sseCmp(tmp, dst, src, imm, toSseS);
    compiler.ins(toSseS4(dst, 0), toSseS4(tmp, 0));
}

void JitArmV8CodeGen::setFlags(RegPtr flags, U32 mask) {
    RegPtr maskedFlags = getTmpReg();
    if (asmjit::a64::Utils::is_logical_imm(mask, 32)) {
        compiler.and_(R32(maskedFlags), R32(flags), mask);
        compiler.and_(xFLAGS, xFLAGS, ~mask);
    } else {
        RegPtr maskReg = loadConst(mask);
        compiler.and_(R32(maskedFlags), R32(flags), R32(maskReg));
        compiler.mvn_(R32(maskReg), R32(maskReg));
        compiler.and_(xFLAGS, xFLAGS, R32(maskReg));
    } 
    compiler.orr(xFLAGS, xFLAGS, R32(maskedFlags));
    currentLazyFlags = FLAGS_NONE;
    storeLazyFlagType(FLAGS_NONE);
}

void JitArmV8CodeGen::writeFlags(RegPtr flags) {
    compiler.mov(xFLAGS, R32(flags));
}

RegPtr JitArmV8CodeGen::getFlagsInTmp(RegPtr tmp) {
    if (!tmp) {
        tmp = getTmpReg();
    }
    compiler.mov(R32(tmp), xFLAGS);
    return tmp;
}

RegPtr JitArmV8CodeGen::getFlagDestReadOnly(RegPtr result) {
    return std::make_shared<JitReg>(regDst, 0xff);
}

RegPtr JitArmV8CodeGen::getFlagDestTmp(RegPtr result) {
    if (!result) {
        result = getTmpReg();
    }
    compiler.mov(R32(result), xDst);
    return result;
}

RegPtr JitArmV8CodeGen::getFlagSrcReadOnly(RegPtr result) {
    return std::make_shared<JitReg>(regSrc, 0xff);
}

RegPtr JitArmV8CodeGen::getFlagSrcTmp(RegPtr result) {
    if (!result) {
        result = getTmpReg();
    }
    compiler.mov(R32(result), xSrc);
    return result;
}

RegPtr JitArmV8CodeGen::getFlagResultReadOnly(RegPtr result) {
    return std::make_shared<JitReg>(regResult, 0xff);
}

RegPtr JitArmV8CodeGen::getFlagResultTmp(RegPtr result) {
    if (!result) {
        result = getTmpReg();
    }
    compiler.mov(R32(result), xResult);
    return result;
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

void JitArmV8CodeGen::storeLazyFlagsResult(RegPtr reg) {
    if (reg->isHigh) {
        compiler.ubfx(xResult, R32(reg), 8, 8);
    } else {
        compiler.mov(xResult, R32(reg));
    }
}

void JitArmV8CodeGen::storeLazyFlagsDest(RegPtr reg) {
    if (reg->isHigh) {
        compiler.ubfx(xDst, R32(reg), 8, 8);
    } else {
        compiler.mov(xDst, R32(reg));
    }
}

void JitArmV8CodeGen::storeLazyFlagsSrc(RegPtr reg) {
    if (reg->isHigh) {
        compiler.ubfx(xSrc, R32(reg), 8, 8);
    } else {
        compiler.mov(xSrc, R32(reg));
    }
}

void JitArmV8CodeGen::storeLazyFlagsSrc(U32 value) {
    compiler.mov(xSrc, value);
}


RegPtr JitArmV8CodeGen::getLazyFlagType() {
    return std::make_shared<JitReg>(regFlagsType, 0xff);
}

RegPtr JitArmV8CodeGen::getLazyFlagTypeInTmp() {
    RegPtr reg = getTmpReg();
    compiler.mov(R32(reg), xFlagsType);
    return reg;
}

RegPtr JitArmV8CodeGen::getReadOnlyFlags(RegPtr tmp) {
    return std::make_shared<JitReg>(regFlags, 0xff);
}

void JitArmV8CodeGen::comissXmmXmm(SSERegPtr dst, SSERegPtr src) {
    comis(dst, src, toSseS);
}

void JitArmV8CodeGen::ucomissXmmXmm(SSERegPtr dst, SSERegPtr src) {
    comis(dst, src, toSseS);
}

void JitArmV8CodeGen::stmxcsr(RegPtr address) {
    RegPtr tmp = getTmpReg();
    compiler.ldr(R32(tmp), createMem(regCPU, offsetof(CPU, mxcsr)));
    compiler.str(R32(tmp), createMem(address, 0));
}

void JitArmV8CodeGen::ldmxcsr(RegPtr address) {
    RegPtr tmp = getTmpReg();
    compiler.ldr(R32(tmp), createMem(address, 0));
    compiler.str(R32(tmp), createMem(regCPU, offsetof(CPU, mxcsr)));
}

void JitArmV8CodeGen::sfence() {
    compiler.dmb(asmjit::a64::Predicate::DB::kISH);
}

void JitArmV8CodeGen::unpckhpsXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.zip2(toSseS4(dst), toSseS4(dst), toSseS4(src));
}

void JitArmV8CodeGen::unpcklpsXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.zip1(toSseS4(dst), toSseS4(dst), toSseS4(src));
}

void JitArmV8CodeGen::movmskpsR32Xmm(RegPtr dst, SSERegPtr src) {
    // cpu->reg[reg].u32 = (cpu->xmm[rm].pd.u32[0] >> 31) | ((cpu->xmm[rm].pd.u32[1] >> 31) << 1) | ((cpu->xmm[rm].pd.u32[2] >> 31) << 2) | ((cpu->xmm[rm].pd.u32[3] >> 31) << 3)
    SSERegPtr vTmpReg = getTmpSSE();
    SSERegPtr bitMaskReg = loadSSEConst(SSE_INT32_BIT_MASK);
    compiler.sshr(toSseS4(vTmpReg), toSseS4(src), 31);
    compiler.and_(toSse128(vTmpReg), toSse128(vTmpReg), toSse128(bitMaskReg));
    compiler.addv(toSseS(vTmpReg), toSseS4(vTmpReg));
    compiler.umov(R32(dst), toSseS4(vTmpReg, 0));
}

MMXRegPtr JitArmV8CodeGen::loadMMXFromReg(RegPtr src) {
    MMXRegPtr tmp = getTmpMMX();
    compiler.fmov(toMmxS(tmp), R32(src));
    return tmp;
}

void JitArmV8CodeGen::storeCpuMMXReg(MMXRegPtr reg, U32 index) {
    compiler.str(toMmxD1(reg), createMem(regCPU, index * cpu->fpu.sizeofRegInRegsArray() + offsetof(CPU, fpu.regs[0].signif)));
}

void JitArmV8CodeGen::storeMMXToReg(MMXRegPtr src, RegPtr dst) {
    compiler.umov(R32(dst), toMmxS4(src, 0));
}

MMXRegPtr JitArmV8CodeGen::loadCpuMMXReg(U8 index) {
    MMXRegPtr tmp = getTmpMMX();
    compiler.ldr(toMmxD(tmp), createMem(regCPU, index * cpu->fpu.sizeofRegInRegsArray() + offsetof(CPU, fpu.regs[0].signif)));
    return tmp;
}

MMXRegPtr JitArmV8CodeGen::loadMMXFromMem32(U8 index, RegPtr rm, RegPtr sib) {
    MMXRegPtr tmp = getTmpMMX();
    compiler.ldr(toMmxS(tmp), createMem(rm, sib, 0, 0));
    return tmp;
}

MMXRegPtr JitArmV8CodeGen::loadMMXFromMem64(U8 index, RegPtr rm, RegPtr sib) {
    MMXRegPtr tmp = getTmpMMX();
    compiler.ldr(toMmxD(tmp), createMem(rm, sib, 0, 0));
    return tmp;
}

void JitArmV8CodeGen::storeMMXToMem32(MMXRegPtr reg, RegPtr rm, RegPtr sib) {
    compiler.str(toMmxS2(reg), createMem(rm, sib, 0, 0));
}

void JitArmV8CodeGen::storeMMXToMem64(MMXRegPtr reg, RegPtr rm, RegPtr sib) {
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

void JitArmV8CodeGen::cvtsi2sdXmmR64(SSERegPtr dst, RegPtr src) {
    SSERegPtr tmpReg = getTmpSSE();

    compiler.ins(toSseS4(tmpReg, 0), R32(src));
    compiler.sxtl(toSseD2(tmpReg), toSseS2(tmpReg)); // convert S32 to S64
    compiler.scvtf(toSseD(tmpReg), toSseD(tmpReg)); // convert S64 to Double
    compiler.mov(toSseD2(dst, 0), toSseD2(tmpReg, 0));
}

void JitArmV8CodeGen::addpdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.fadd(toSseD2(dst), toSseD2(dst), toSseD2(src));
}

void JitArmV8CodeGen::addsdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    SSERegPtr tmp = getTmpSSE();
    compiler.fadd(toSseD(tmp), toSseD(dst), toSseD(src));
    compiler.ins(toSseD2(dst, 0), toSseD2(tmp, 0)); // need to maintain the high 64-bits
}

void JitArmV8CodeGen::subpdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.fsub(toSseD2(dst), toSseD2(dst), toSseD2(src));
}

void JitArmV8CodeGen::subsdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    SSERegPtr tmp = getTmpSSE();
    compiler.fsub(toSseD(tmp), toSseD(dst), toSseD(src));
    compiler.ins(toSseD2(dst, 0), toSseD2(tmp, 0)); // need to maintain the high 64-bits
}

void JitArmV8CodeGen::mulpdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.fmul(toSseD2(dst), toSseD2(dst), toSseD2(src));
}

void JitArmV8CodeGen::mulsdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    SSERegPtr tmp = getTmpSSE();
    compiler.fmul(toSseD(tmp), toSseD(dst), toSseD(src));
    compiler.ins(toSseD2(dst, 0), toSseD2(tmp, 0)); // need to maintain the high 64-bits
}

void JitArmV8CodeGen::divpdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.fdiv(toSseD2(dst), toSseD2(dst), toSseD2(src));
}

void JitArmV8CodeGen::divsdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    SSERegPtr tmp = getTmpSSE();
    compiler.fdiv(toSseD(tmp), toSseD(dst), toSseD(src));
    compiler.ins(toSseD2(dst, 0), toSseD2(tmp, 0)); // need to maintain the high 64-bits
}

void JitArmV8CodeGen::maxpdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    fmax(dst, dst, src, toSseD2);
}

void JitArmV8CodeGen::maxsdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    SSERegPtr tmp = getTmpSSE();
    fmax(tmp, dst, src, toSseD);
    compiler.ins(toSseD2(dst, 0), toSseD2(tmp, 0)); // need to maintain the high 64-bits
}

void JitArmV8CodeGen::minpdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    fmin(dst, dst, src, toSseD2);
}

void JitArmV8CodeGen::minsdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    SSERegPtr tmp = getTmpSSE();
    fmin(tmp, dst, src, toSseD);
    compiler.ins(toSseD2(dst, 0), toSseD2(tmp, 0)); // need to maintain the high 64-bits
}

void JitArmV8CodeGen::paddbXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.add(toSseB16(dst), toSseB16(dst), toSseB16(src));
}

void JitArmV8CodeGen::paddwXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.add(toSseH8(dst), toSseH8(dst), toSseH8(src));
}

void JitArmV8CodeGen::padddXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.add(toSseS4(dst), toSseS4(dst), toSseS4(src));
}

void JitArmV8CodeGen::paddqXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.add(toSseD2(dst), toSseD2(dst), toSseD2(src));
}

void JitArmV8CodeGen::paddsbXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.sqadd(toSseB16(dst), toSseB16(dst), toSseB16(src));
}

void JitArmV8CodeGen::paddswXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.sqadd(toSseH8(dst), toSseH8(dst), toSseH8(src));
}

void JitArmV8CodeGen::paddusbXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.uqadd(toSseB16(dst), toSseB16(dst), toSseB16(src));
}

void JitArmV8CodeGen::padduswXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.uqadd(toSseH8(dst), toSseH8(dst), toSseH8(src));
}

void JitArmV8CodeGen::psubbXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.sub(toSseB16(dst), toSseB16(dst), toSseB16(src));
}

void JitArmV8CodeGen::psubwXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.sub(toSseH8(dst), toSseH8(dst), toSseH8(src));
}

void JitArmV8CodeGen::psubdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.sub(toSseS4(dst), toSseS4(dst), toSseS4(src));
}

void JitArmV8CodeGen::psubqXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.sub(toSseD2(dst), toSseD2(dst), toSseD2(src));
}

void JitArmV8CodeGen::psubsbXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.sqsub(toSseB16(dst), toSseB16(dst), toSseB16(src));
}

void JitArmV8CodeGen::psubswXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.sqsub(toSseH8(dst), toSseH8(dst), toSseH8(src));
}

void JitArmV8CodeGen::psubusbXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.uqsub(toSseB16(dst), toSseB16(dst), toSseB16(src));
}

void JitArmV8CodeGen::psubuswXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.uqsub(toSseH8(dst), toSseH8(dst), toSseH8(src));
}

void JitArmV8CodeGen::pmaddwdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    SSERegPtr vTmpReg1 = getTmpSSE();
    SSERegPtr vTmpReg2 = getTmpSSE();

    // multiply bottom 4x 16-bit numbers and put them into 4x 32-bit number in vTmpReg1
    compiler.smull(toSseS4(vTmpReg1), toSseH4(dst), toSseH4(src));

    // multiply top 4x 16-bit numbers and put them into 4x 32-bit number in vTmpReg2
    compiler.smull2(toSseS4(vTmpReg2), toSseH8(dst), toSseH8(src));

    compiler.addp(toSseS4(dst), toSseS4(vTmpReg1), toSseS4(vTmpReg2));
}

void JitArmV8CodeGen::pmulhwXmmXmm(SSERegPtr dst, SSERegPtr src) {
    SSERegPtr vTmpReg1 = getTmpSSE();
    SSERegPtr vTmpReg2 = getTmpSSE();

    // high result of H0*H0, H1*H1, H2*H2 and H3*H3 will be in H1, H3, H5, H7
    compiler.smull(toSseS4(vTmpReg1), toSseH4(dst), toSseH4(src));

    // high result of H4*H4, H5*H5, H6*H6 and H7*H7 will be in H1, H3, H5, H7
    compiler.smull2(toSseS4(vTmpReg2), toSseH8(dst), toSseH8(src));

    compiler.uzp2(toSseH8(dst), toSseH8(vTmpReg1), toSseH8(vTmpReg2));
}

void JitArmV8CodeGen::pmullwXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.mul(toSseH8(dst), toSseH8(dst), toSseH8(src));
}

void JitArmV8CodeGen::pmuludqXmmXmm(SSERegPtr dst, SSERegPtr src) {
    // cpu->xmm[r1].pi.u64[0] = cpu->xmm[r1].pi.u32[0] * cpu->xmm[r2].pi.u32[0]
    // cpu->xmm[r1].pi.u64[1] = cpu->xmm[r1].pi.u32[2] * cpu->xmm[r2].pi.u32[2]
    SSERegPtr vTmpReg1 = getTmpSSE();
    SSERegPtr vTmpReg2 = getTmpSSE();

    compiler.xtn(toSseS2(vTmpReg1), toSseS2(dst)); // mov index 2 to index 1 so that they are packed
    compiler.xtn(toSseS2(vTmpReg2), toSseS2(src));
    compiler.umull(toSseD2(dst), toSseS2(vTmpReg1), toSseS2(vTmpReg2));
}

void JitArmV8CodeGen::sqrtpdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.fsqrt(toSseD2(dst), toSseD2(src));
}

void JitArmV8CodeGen::sqrtsdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    SSERegPtr tmp = getTmpSSE();
    compiler.fsqrt(toSse64(tmp), toSse64(src));
    compiler.ins(toSseD2(dst, 0), toSseD2(tmp, 0));
}

void JitArmV8CodeGen::andnpdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.bic(toSse128(dst), toSse128(src), toSse128(dst));
}

void JitArmV8CodeGen::andpdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.and_(toSse128(dst), toSse128(dst), toSse128(src));
}

void JitArmV8CodeGen::pandXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.and_(toSse128(dst), toSse128(dst), toSse128(src));
}

void JitArmV8CodeGen::pandnXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.bic(toSse128(dst), toSse128(src), toSse128(dst));
}

void JitArmV8CodeGen::porXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.orr(toSse128(dst), toSse128(dst), toSse128(src));
}

// Shift xmm1 left by imm8 BYTES while shifting in 0s.
void JitArmV8CodeGen::pslldqXmm(SSERegPtr dst, U32 imm) {
    if (imm > 15) {
        compiler.movi(toSseD2(dst), 0);
    } else {
        SSERegPtr vTmpReg = getTmpSSE();
        compiler.movi(toSseD2(vTmpReg), 0);
        compiler.ext(toSseB16(dst), toSseB16(vTmpReg), toSseB16(dst), 16 - imm);
    }
}

void JitArmV8CodeGen::psllqXmm(SSERegPtr dst, U32 imm) {
    compiler.shl(toSseD2(dst), toSseD2(dst), imm);
}

void JitArmV8CodeGen::pslldXmm(SSERegPtr dst, U32 imm) {
    compiler.shl(toSseS4(dst), toSseS4(dst), imm);
}

void JitArmV8CodeGen::psllwXmm(SSERegPtr dst, U32 imm) {
    compiler.shl(toSseH8(dst), toSseH8(dst), imm);
}

void JitArmV8CodeGen::psradXmm(SSERegPtr dst, U32 imm) {
    compiler.sshr(toSseS4(dst), toSseS4(dst), imm);
}

void JitArmV8CodeGen::psrawXmm(SSERegPtr dst, U32 imm) {
    compiler.sshr(toSseH8(dst), toSseH8(dst), imm);
}

void JitArmV8CodeGen::psrldqXmm(SSERegPtr dst, U32 imm) {
    if (imm > 15) {
        compiler.movi(toSseD2(dst), 0);
    } else {
        SSERegPtr vTmpReg = getTmpSSE();
        compiler.movi(toSseD2(vTmpReg), 0);
        compiler.ext(toSseB16(dst), toSseB16(dst), toSseB16(vTmpReg), imm);
    }
}

void JitArmV8CodeGen::psrlqXmm(SSERegPtr dst, U32 imm) {
    compiler.ushr(toSseD2(dst), toSseD2(dst), imm);
}

void JitArmV8CodeGen::psrldXmm(SSERegPtr dst, U32 imm) {
    compiler.ushr(toSseS4(dst), toSseS4(dst), imm);
}

void JitArmV8CodeGen::psrlwXmm(SSERegPtr dst, U32 imm) {
    compiler.ushr(toSseH8(dst), toSseH8(dst), imm);
}

void JitArmV8CodeGen::psllqXmmXmm(SSERegPtr dst, SSERegPtr src) {
    SSERegPtr tmp = getTmpSSE();
    compiler.dup(toSseD2(tmp), toSseD2(src, 0));
    compiler.ushl(toSseD2(dst), toSseD2(dst), toSseD2(tmp));
}

void JitArmV8CodeGen::pslldXmmXmm(SSERegPtr dst, SSERegPtr src) {
    SSERegPtr tmp = getTmpSSE();
    compiler.dup(toSseS4(tmp), toSseS4(src, 0));
    compiler.ushl(toSseS4(dst), toSseS4(dst), toSseS4(tmp));
}

void JitArmV8CodeGen::psllwXmmXmm(SSERegPtr dst, SSERegPtr src) {
    SSERegPtr tmp = getTmpSSE();
    compiler.dup(toSseH8(tmp), toSseH8(src, 0));
    compiler.ushl(toSseH8(dst), toSseH8(dst), toSseH8(tmp));
}

void JitArmV8CodeGen::psradXmmXmm(SSERegPtr dst, SSERegPtr src) {
    // negative values in src means shift right for sshl
    SSERegPtr tmp = getTmpSSE();
    compiler.dup(toSseS4(tmp), toSseS4(src, 0));
    compiler.neg(toSseS4(tmp), toSseS4(tmp));
    compiler.sshl(toSseS4(dst), toSseS4(dst), toSseS4(tmp));
}

void JitArmV8CodeGen::psrawXmmXmm(SSERegPtr dst, SSERegPtr src) {
    // negative values in src means shift right for sshl
    SSERegPtr tmp = getTmpSSE();
    compiler.dup(toSseH8(tmp), toSseH8(src, 0));
    compiler.neg(toSseH8(tmp), toSseH8(tmp));
    compiler.sshl(toSseH8(dst), toSseH8(dst), toSseH8(tmp));
}

void JitArmV8CodeGen::psrlqXmmXmm(SSERegPtr dst, SSERegPtr src) {
    // negative values in src means shift right for ushl
    SSERegPtr tmp = getTmpSSE();
    compiler.dup(toSseD2(tmp), toSseD2(src, 0));
    compiler.neg(toSseD2(tmp), toSseD2(tmp));
    compiler.ushl(toSseD2(dst), toSseD2(dst), toSseD2(tmp));
}

void JitArmV8CodeGen::psrldXmmXmm(SSERegPtr dst, SSERegPtr src) {
    // negative values in src means shift right for ushl
    SSERegPtr tmp = getTmpSSE();
    compiler.dup(toSseS4(tmp), toSseS4(src, 0));
    compiler.neg(toSseS4(tmp), toSseS4(tmp));
    compiler.ushl(toSseS4(dst), toSseS4(dst), toSseS4(tmp));
}

void JitArmV8CodeGen::psrlwXmmXmm(SSERegPtr dst, SSERegPtr src) {
    // negative values in src means shift right for ushl
    SSERegPtr tmp = getTmpSSE();
    compiler.dup(toSseH8(tmp), toSseH8(src, 0));
    compiler.neg(toSseH8(tmp), toSseH8(tmp));
    compiler.ushl(toSseH8(dst), toSseH8(dst), toSseH8(tmp));
}

void JitArmV8CodeGen::pxorXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.eor(toSse128(dst), toSse128(dst), toSse128(src));
}

void JitArmV8CodeGen::orpdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.orr(toSse128(dst), toSse128(dst), toSse128(src));
}

void JitArmV8CodeGen::xorpdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.eor(toSse128(dst), toSse128(dst), toSse128(src));
}

void JitArmV8CodeGen::cmppdXmmXmm(SSERegPtr dst, SSERegPtr src, U32 imm) {
    sseCmp(dst, dst, src, imm, toSseD2);
}

void JitArmV8CodeGen::cmpsdXmmXmm(SSERegPtr dst, SSERegPtr src, U32 imm) {
    SSERegPtr tmp = getTmpSSE();
    sseCmp(tmp, dst, src, imm, toSseD);
    compiler.ins(toSseD2(dst, 0), toSseD2(tmp, 0));
}

void JitArmV8CodeGen::comisdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    comis(dst, src, toSseD);
}

void JitArmV8CodeGen::ucomisdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    comis(dst, src, toSseD);
}

void JitArmV8CodeGen::pcmpgtbXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.cmgt(toSseB16(dst), toSseB16(dst), toSseB16(src));
}

void JitArmV8CodeGen::pcmpgtwXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.cmgt(toSseH8(dst), toSseH8(dst), toSseH8(src));
}

void JitArmV8CodeGen::pcmpgtdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.cmgt(toSseS4(dst), toSseS4(dst), toSseS4(src));
}

void JitArmV8CodeGen::pcmpeqbXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.cmeq(toSseB16(dst), toSseB16(dst), toSseB16(src));
}

void JitArmV8CodeGen::pcmpeqwXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.cmeq(toSseH8(dst), toSseH8(dst), toSseH8(src));
}

void JitArmV8CodeGen::pcmpeqdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.cmeq(toSseS4(dst), toSseS4(dst), toSseS4(src));
}

void JitArmV8CodeGen::cvtdq2pdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    // cpu->xmm[reg].pd.f64[0] = (double)cpu->xmm[rm].pi.i32[0];
    // cpu->xmm[reg].pd.f64[1] = (double)cpu->xmm[rm].pi.i32[1];
    compiler.sxtl(toSseD2(dst), toSseS2(src)); // convert 32-bit to 64-bit integer
    compiler.scvtf(toSseD2(dst), toSseD2(dst)); // convert 64-bit integer to Double
}

void JitArmV8CodeGen::cvtdq2psXmmXmm(SSERegPtr dst, SSERegPtr src) {
    // cpu->xmm[reg].ps.f32[0] = (float)cpu->xmm[rm].pi.i32[0];
    // cpu->xmm[reg].ps.f32[1] = (float)cpu->xmm[rm].pi.i32[1];
    // cpu->xmm[reg].ps.f32[2] = (float)cpu->xmm[rm].pi.i32[2];
    // cpu->xmm[reg].ps.f32[3] = (float)cpu->xmm[rm].pi.i32[3];
    compiler.scvtf(toSseS4(dst), toSseS4(src));
}

void JitArmV8CodeGen::cvtpd2piMmxXmm(MMXRegPtr dst, SSERegPtr src) {
    cvtpd2pi(std::make_shared<SSERegInternal>(dst->hardwareReg(), 0xff), src, false);
}

void JitArmV8CodeGen::cvtpi2pdXmmMmx(SSERegPtr dst, MMXRegPtr src) {
    // cpu->xmm[reg].pd.f64[0] = cpu->reg_mmx[reg].sd.d0;
    // cpu->xmm[reg].pd.f64[1] = cpu->reg_mmx[reg].sd.d1;   
    
    compiler.sxtl(toSseD2(dst), toMmxS2(src));
    compiler.scvtf(toSseD2(dst), toSseD2(dst));
}

void JitArmV8CodeGen::cvtpd2dqXmmXmm(SSERegPtr dst, SSERegPtr src) {
    cvtpd2pi(dst, src, false);
}

void JitArmV8CodeGen::cvtpd2psXmmXmm(SSERegPtr dst, SSERegPtr src) {
    // cpu->xmm[reg].ps.f32[0] = (float)cpu->xmm[rm].pd.f64[0];
    // cpu->xmm[reg].ps.f32[1] = (float)cpu->xmm[rm].pd.f64[1];
    // cpu->xmm[reg].pi.u32[2] = 0
    // cpu->xmm[reg].pi.u32[3] = 0
    compiler.fcvtn(toSseS2(dst), toSseD2(src)); // :TODO: what about rounding
}

void JitArmV8CodeGen::cvttpd2piMmxXmm(MMXRegPtr dst, SSERegPtr src) {
    cvtpd2pi(std::make_shared<SSERegInternal>(dst->hardwareReg(), 0xff), src, true);
}

void JitArmV8CodeGen::cvtps2dqXmmXmm(SSERegPtr dst, SSERegPtr src) {
    cvtps2pi(dst, src, false);
}

void JitArmV8CodeGen::cvtps2pdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.fcvtl(toSseD2(dst), toSseS2(src));
}

void JitArmV8CodeGen::cvtsd2siR32Xmm(RegPtr dst, SSERegPtr src) {
    cvtsd2si(dst, src, false);
}

void JitArmV8CodeGen::cvtsd2ssXmmXmm(SSERegPtr dst, SSERegPtr src) {
    SSERegPtr tmpReg = getTmpSSE();
    compiler.fcvtn(toSseS2(tmpReg), toSseD2(src));
    compiler.ins(toSseS4(dst, 0), toSseS4(tmpReg, 0));
}

void JitArmV8CodeGen::cvtsi2sdXmmR32(SSERegPtr dst, RegPtr src) {
    // cpu->xmm[reg].pd.f64[0] = cpu->reg[rm].u32;
    SSERegPtr tmpReg = getTmpSSE();
    compiler.ins(toSseS4(tmpReg, 0), R32(src));
    compiler.sxtl(toSseD2(tmpReg), toSseS2(tmpReg));
    compiler.scvtf(toSseD(tmpReg), toSseD(tmpReg));
    compiler.ins(toSseD2(dst, 0), toSseD2(tmpReg, 0));
}

void JitArmV8CodeGen::cvtss2sdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    // cpu->xmm[reg].pd.f64[0] = (float)cpu->xmm[rm].ps.f32[0];
    SSERegPtr tmpReg = getTmpSSE(); 
    compiler.fcvt(toSseD(tmpReg), toSseS(src));
    compiler.ins(toSseD2(dst, 0), toSseD2(tmpReg, 0));
}

void JitArmV8CodeGen::cvttpd2dqXmmXmm(SSERegPtr dst, SSERegPtr src) {
    // cpu->xmm[reg].pi.s32[0] = (S32)cpu->xmm[rm].pd.f64[0];
    // cpu->xmm[reg].pi.s32[1] = (S32)cpu->xmm[rm].pd.f64[1];
    // cpu->xmm[reg].pi.u32[2] = 0
    // cpu->xmm[reg].pi.u32[3] = 0
    cvtpd2pi(dst, src, true);
}

void JitArmV8CodeGen::cvttps2dqXmmXmm(SSERegPtr dst, SSERegPtr src) {
    cvtps2pi(dst, src, true);
}

void JitArmV8CodeGen::cvttsd2siR32Xmm(RegPtr dst, SSERegPtr src) {
    cvtsd2si(dst, src, true);
}

void JitArmV8CodeGen::movsdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.ins(toSseD2(dst, 0), toSseD2(src, 0));
}

void JitArmV8CodeGen::movupdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.mov(toSseD2(dst), toSseD2(src));
}

void JitArmV8CodeGen::movmskpd(RegPtr dst, SSERegPtr src) {
    // cpu->reg[reg].u32 = (cpu->xmm[rm].pd.u64[0] >> 63) | ((cpu->xmm[rm].pd.u64[1] >> 63) << 1)
    SSERegPtr vTmpReg = getTmpSSE();
    RegPtr tmpReg = getTmpReg();

    compiler.ushr(toSseD2(vTmpReg), toSseD2(src), 63);
    compiler.umov(R32(dst), toSseS4(vTmpReg, 0));
    compiler.umov(R32(tmpReg), toSseS4(vTmpReg, 2));
    compiler.lsl(R32(tmpReg), R32(tmpReg), 1);
    compiler.orr(R32(dst), R32(dst), R32(tmpReg));
}

void JitArmV8CodeGen::movd(RegPtr dst, SSERegPtr src) {
    compiler.umov(R32(dst), toSseS4(src, 0));
}

void JitArmV8CodeGen::movd(SSERegPtr dst, RegPtr src) {
    // couldn't see a way to do this in 1 instruction, "mov s0, w0" is not valid
    compiler.ins(toSseS4(dst, 0), R32(src));
    compiler.mov(toSseS(dst), toSseS4(dst, 0));
}

void JitArmV8CodeGen::movdq2q(MMXRegPtr dst, SSERegPtr src) {
    compiler.mov(toMmxD(dst), toSseD2(src, 0));
}

void JitArmV8CodeGen::movq2dq(SSERegPtr dst, MMXRegPtr src) {
    compiler.mov(toSseD(dst), toMmxD1(src));
}

void JitArmV8CodeGen::movq(SSERegPtr dst, SSERegPtr src) {
    // cpu->xmm[reg].pi.u64[0] = cpu->xmm[rm].pi.u64[0];
    // cpu->xmm[reg].pi.u64[1] = 0;
    compiler.mov(toSseD(dst), toSseD2(src, 0));
}

void JitArmV8CodeGen::maskmovdqu(SSERegPtr src, SSERegPtr mask, RegPtr address) {
    // maskmovdqu xmm1, xmm2
    // this will mov xmm1[i] to DS:EDI[i] if (xmm2[i] & 80)
    SSERegPtr vTmpReg = getTmpSSE();
    SSERegPtr vTmpRegMask = getTmpSSE();

    compiler.ldr(toSse128(vTmpReg), Mem(R64(address)));
    compiler.sshr(toSseB16(vTmpRegMask), toSseB16(mask), 7);
    compiler.bsl(toSseB16(vTmpRegMask), toSseB16(src), toSseB16(vTmpReg));
    compiler.str(toSse128(vTmpRegMask), Mem(R64(address)));
}

void JitArmV8CodeGen::pshufdXmmXmm(SSERegPtr dst, SSERegPtr src, U32 mask) {
    SSERegPtr tmp = dst;

    if (dst->hardwareReg() == src->hardwareReg()) {
        tmp = getTmpSSE();
    }
    compiler.ins(toSseS4(tmp, 0), toSseS4(src, mask & 3));
    compiler.ins(toSseS4(tmp, 1), toSseS4(src, (mask >> 2) & 3));
    compiler.ins(toSseS4(tmp, 2), toSseS4(src, (mask >> 4) & 3));
    compiler.ins(toSseS4(tmp, 3), toSseS4(src, (mask >> 6) & 3));
    if (dst->hardwareReg() == src->hardwareReg()) {
        compiler.mov(toSse128(dst), toSse128(tmp));
    }
}

// from spec: The low quadword of the source operand is copied to the low quadword of the destination operand, for each 128-bit lane.
void JitArmV8CodeGen::pshufhwXmmXmm(SSERegPtr dst, SSERegPtr src, U32 mask) {
    SSERegPtr tmp = dst;

    if (dst->hardwareReg() == src->hardwareReg()) {
        tmp = getTmpSSE();
    }
    compiler.ins(toSseH8(tmp, 4), toSseH8(src, 4 + (mask & 3)));
    compiler.ins(toSseH8(tmp, 5), toSseH8(src, 4 + ((mask >> 2) & 3)));
    compiler.ins(toSseH8(tmp, 6), toSseH8(src, 4 + ((mask >> 4) & 3)));
    compiler.ins(toSseH8(tmp, 7), toSseH8(src, 4 + ((mask >> 6) & 3)));
    if (dst->hardwareReg() == src->hardwareReg()) {
        compiler.ins(toSseD2(dst, 1), toSseD2(tmp, 1));
    } else {
        compiler.ins(toSseD2(dst, 0), toSseD2(src, 0));
    }
}

// from spec: The high quadword of the source operand is copied to the high quadword of the destination operand
void JitArmV8CodeGen::pshuflwXmmXmm(SSERegPtr dst, SSERegPtr src, U32 mask) {
    SSERegPtr tmp = dst;

    if (dst->hardwareReg() == src->hardwareReg()) {
        tmp = getTmpSSE();
    }
    compiler.ins(toSseH8(tmp, 0), toSseH8(src, (mask & 3)));
    compiler.ins(toSseH8(tmp, 1), toSseH8(src, (mask >> 2) & 3));
    compiler.ins(toSseH8(tmp, 2), toSseH8(src, (mask >> 4) & 3));
    compiler.ins(toSseH8(tmp, 3), toSseH8(src, (mask >> 6) & 3));
    if (dst->hardwareReg() == src->hardwareReg()) {
        compiler.ins(toSseD2(dst, 0), toSseD2(tmp, 0));
    } else {
        compiler.ins(toSseD2(dst, 1), toSseD2(src, 1));
    }
}

void JitArmV8CodeGen::shufpdXmmXmm(SSERegPtr dst, SSERegPtr src, U32 mask) {
    // r.f64[0] = ((imm8 & 1) == 0) ? a.f64[0] : a.f64[1];
    // r.f64[1] = ((imm8 & 2) == 0) ? b.f64[0] : b.f64[1];
    SSERegPtr tmp = dst;

    if (dst->hardwareReg() == src->hardwareReg()) {
        tmp = getTmpSSE();
    }
    compiler.ins(toSseD2(tmp, 0), toSseD2(dst, mask & 1));
    compiler.ins(toSseD2(tmp, 1), toSseD2(src, (mask >> 1) & 1));
    if (dst->hardwareReg() == src->hardwareReg()) {
        compiler.mov(toSse128(dst), toSse128(tmp));
    }
}

void JitArmV8CodeGen::unpckhpdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.zip2(toSseD2(dst), toSseD2(dst), toSseD2(src));
}

void JitArmV8CodeGen::unpcklpdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    // cpu->xmm[reg].pd.u64[0] = cpu->xmm[reg].pd.u64[0];
    // cpu->xmm[reg].pd.u64[1] = cpu->xmm[rm].pd.u64[0];
    compiler.ins(toSseD2(dst, 1), toSseD2(src, 0));
}

void JitArmV8CodeGen::punpckhbwXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.zip2(toSseB16(dst), toSseB16(dst), toSseB16(src));
}

void JitArmV8CodeGen::punpckhwdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.zip2(toSseH8(dst), toSseH8(dst), toSseH8(src));
}

void JitArmV8CodeGen::punpckhdqXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.zip2(toSseS4(dst), toSseS4(dst), toSseS4(src));
}

void JitArmV8CodeGen::punpckhqdqXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.zip2(toSseD2(dst), toSseD2(dst), toSseD2(src));
}

void JitArmV8CodeGen::punpcklbwXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.zip1(toSseB16(dst), toSseB16(dst), toSseB16(src));
}

void JitArmV8CodeGen::punpcklwdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.zip1(toSseH8(dst), toSseH8(dst), toSseH8(src));
}

void JitArmV8CodeGen::punpckldqXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.zip1(toSseS4(dst), toSseS4(dst), toSseS4(src));
}

void JitArmV8CodeGen::punpcklqdqXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.zip1(toSseD2(dst), toSseD2(dst), toSseD2(src));
}

void JitArmV8CodeGen::packssdwXmmXmm(SSERegPtr dst, SSERegPtr src) {
    SSERegPtr tmp = dst;

    if (dst->hardwareReg() == src->hardwareReg()) {
        tmp = getTmpSSE();
    }
    compiler.sqxtn(toSseH4(tmp), toSseS4(dst));
    compiler.sqxtn2(toSseH8(tmp), toSseS4(src));
    if (dst->hardwareReg() == src->hardwareReg()) {
        compiler.mov(toSse128(dst), toSse128(tmp));
    }
}

void JitArmV8CodeGen::packsswbXmmXmm(SSERegPtr dst, SSERegPtr src) {
    SSERegPtr tmp = dst;

    if (dst->hardwareReg() == src->hardwareReg()) {
        tmp = getTmpSSE();
    }
    compiler.sqxtn(toSseB8(tmp), toSseH8(dst));
    compiler.sqxtn2(toSseB16(tmp), toSseH8(src));
    if (dst->hardwareReg() == src->hardwareReg()) {
        compiler.mov(toSse128(dst), toSse128(tmp));
    }
}

void JitArmV8CodeGen::packuswbXmmXmm(SSERegPtr dst, SSERegPtr src) {
    SSERegPtr tmp = dst;

    if (dst->hardwareReg() == src->hardwareReg()) {
        tmp = getTmpSSE();
    }
    compiler.sqxtun(toSseB8(tmp), toSseH8(dst));
    compiler.sqxtun2(toSseB16(tmp), toSseH8(src));
    if (dst->hardwareReg() == src->hardwareReg()) {
        compiler.mov(toSse128(dst), toSse128(tmp));
    }
}

void JitArmV8CodeGen::pavgbXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.urhadd(toSseB16(dst), toSseB16(dst), toSseB16(src));
}

void JitArmV8CodeGen::pavgwXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.urhadd(toSseH8(dst), toSseH8(dst), toSseH8(src));
}

void JitArmV8CodeGen::psadbwXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.uabd(toSseB16(dst), toSseB16(dst), toSseB16(src));
    compiler.uaddlp(toSseH8(dst), toSseB16(dst));
    compiler.uaddlp(toSseS4(dst), toSseH8(dst));
    compiler.uaddlp(toSseD2(dst), toSseS4(dst));
}

void JitArmV8CodeGen::pmaxswXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.smax(toSseH8(dst), toSseH8(dst), toSseH8(src));
}

void JitArmV8CodeGen::pmaxubXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.umax(toSseB16(dst), toSseB16(dst), toSseB16(src));
}

void JitArmV8CodeGen::pminswXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.smin(toSseH8(dst), toSseH8(dst), toSseH8(src));
}

void JitArmV8CodeGen::pminubXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.umin(toSseB16(dst), toSseB16(dst), toSseB16(src));
}

void JitArmV8CodeGen::pmulhuwXmmXmm(SSERegPtr dst, SSERegPtr src) {
    SSERegPtr vTmpReg1 = getTmpSSE();
    SSERegPtr vTmpReg2 = getTmpSSE();
    // high result of H0*H0, H1*H1, H2*H2 and H3*H3 will be in H1, H3, H5, H7
    compiler.umull(toSseS4(vTmpReg1), toSseH4(dst), toSseH4(src));
    // high result of H4*H4, H5*H5, H6*H6 and H7*H7 will be in H1, H3, H5, H7
    compiler.umull2(toSseS4(vTmpReg2), toSseH8(dst), toSseH8(src));
    compiler.uzp2(toSseH8(dst), toSseH8(vTmpReg1), toSseH8(vTmpReg2));
}

void JitArmV8CodeGen::lfence() {
    // :TODO: is a full barrier necessary (dmb ishld)
    compiler.dmb(asmjit::a64::Predicate::DB::kISH);
}

void JitArmV8CodeGen::mfence() {
    compiler.dmb(asmjit::a64::Predicate::DB::kISH);
}

void JitArmV8CodeGen::clflush(RegPtr address) {  
    // might need some work on this
    compiler.dc(asmjit::a64::Predicate::DC::kCVAC, R64(address));
}

void JitArmV8CodeGen::pause() {
    compiler.yield();
}

void JitArmV8CodeGen::pextrwR32Xmm(RegPtr dst, SSERegPtr src, U32 imm) {
    compiler.umov(R32(dst), toSseH8(src, imm & 7));
}

void JitArmV8CodeGen::pinsrwXmmR32(SSERegPtr dst, RegPtr src, U32 imm) {
    compiler.ins(toSseH8(dst, imm & 7), R32(src));
}

void JitArmV8CodeGen::pmovmskbR32Xmm(RegPtr dst, SSERegPtr src) {
    // for all 16 bytes set a bit in a mask if it is signed
    SSERegPtr vTmpReg = getTmpSSE();

    SSERegPtr bitMaskReg = loadSSEConst(SSE_BYTE8_BIT_MASK);
    // turn all the bits to 1 if signed
    compiler.sshr(toSseB16(vTmpReg), toSseB16(src), 7);
    // mask out the bit that should be set, so index 0 will set bit 0, index 1 will set bit 1, etc
    compiler.and_(toSseB16(vTmpReg), toSseB16(vTmpReg), toSseB16(bitMaskReg));

    SSERegPtr vTmpReg2 = getTmpSSE();
    compiler.dup(toSseD2(vTmpReg2), toSseD2(vTmpReg, 1));

    compiler.addv(toSseB(vTmpReg), toSseB8(vTmpReg));
    compiler.addv(toSseB(vTmpReg2), toSseB8(vTmpReg2));
    
    RegPtr tmpReg = getTmpReg();
    RegPtr tmpReg2 = getTmpReg();

    compiler.umov(R32(tmpReg), toSseB16(vTmpReg, 0));
    compiler.umov(R32(tmpReg2), toSseB16(vTmpReg2, 0));
    compiler.add(R32(dst), R32(tmpReg), R32(tmpReg2), asmjit::Imm(asmjit::a64::Shift(asmjit::a64::ShiftOp::kLSL, 8)));
}

Vec toVec(FPURegPtr reg) {
    return Vec::make_d(reg->hardwareReg());
}

Vec toVec32(FPURegPtr reg) {
    return Vec::make_s(reg->hardwareReg());
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
    return fpuRegToInt(fpuRegSrc, truncate, false);
}

RegPtr JitArmV8CodeGen::fpuRegToInt(FPURegPtr fpuRegSrc, bool truncate, bool is64) {
    RegPtr result = getTmpReg();
    asmjit::a64::Gp resultReg;

    if (is64) {
        resultReg = R64(result);
    } else {
        resultReg = R32(result);
    }
    if (truncate) {
        compiler.fcvtzs(resultReg, toVec(fpuRegSrc));
    } else {
        RegPtr round = readCPU(JitWidth::b32, offsetof(CPU, fpu.round));
        IfNot(JitWidth::b32, round); { // ROUND_Nearest
            compiler.fcvtns(resultReg, toVec(fpuRegSrc));
        } StartElse(); {
            IfEqual(JitWidth::b32, round, ROUND_Down); {
                compiler.fcvtms(resultReg, toVec(fpuRegSrc));
            } StartElse(); {
                IfEqual(JitWidth::b32, round, ROUND_Up); {
                    compiler.fcvtps(resultReg, toVec(fpuRegSrc));
                } StartElse(); {
                    compiler.fcvtzs(resultReg, toVec(fpuRegSrc));
                } EndIf();
            } EndIf();
        } EndIf();
    }
    return result;
}

void JitArmV8CodeGen::storeFPUToInt64(FPURegPtr src, RegPtr address, RegPtr offset, bool truncate) {
    RegPtr result = fpuRegToInt(src, truncate, true);
    writeHost(JitWidth::b64, createMemPtr(address, offset, 0, 0), result);
}

void JitArmV8CodeGen::roundFPUToInt64(FPURegPtr src) {
    RegPtr round = readCPU(JitWidth::b32, offsetof(CPU, fpu.round));
    IfNot(JitWidth::b32, round); { // ROUND_Nearest
        compiler.frintn(Vec::make_v64(src->hardwareReg()), toVec(src));
    } StartElse(); {
        IfEqual(JitWidth::b32, round, ROUND_Down); {
            compiler.frintm(Vec::make_v64(src->hardwareReg()), toVec(src));
        } StartElse(); {
            IfEqual(JitWidth::b32, round, ROUND_Up); {
                compiler.frintp(Vec::make_v64(src->hardwareReg()), toVec(src));
            } StartElse(); {
                compiler.frintz(Vec::make_v64(src->hardwareReg()), toVec(src));
            } EndIf();
        } EndIf();
    } EndIf();
}

void JitArmV8CodeGen::storeFpuReg(FPURegPtr reg, RegPtr rm, RegPtr sib, DynFpuWidth width) {
    if (width == DYN_FPU_64_BIT) {
        compiler.str(toVec(reg), Mem(R64(rm), R64(sib)));
    } else {
        compiler.str(toVec32(reg), Mem(R64(rm), R64(sib)));
    }
}

void JitArmV8CodeGen::loadFpuReg(FPURegPtr reg, RegPtr rm, RegPtr sib, DynFpuWidth width) {
    if (width == DYN_FPU_64_BIT) {
        compiler.ldr(toVec(reg), Mem(R64(rm), R64(sib)));
    } else {
        compiler.ldr(toVec32(reg), Mem(R64(rm), R64(sib)));
    }
}

void JitArmV8CodeGen::fpuRegExtend32To64(FPURegPtr dst, FPURegPtr src) {
    compiler.fcvt(toVec(dst), toVec32(src));
}

void JitArmV8CodeGen::fpuReg64To32(FPURegPtr dst, FPURegPtr src) {
    compiler.fcvt(toVec32(dst), toVec(src));
    //compiler.fcvtn(Vec::make_v64_with_element_type(VecElementType::kS, dst->hardwareReg()), Vec::make_v128_with_element_type(VecElementType::kD, src->hardwareReg()));
}

void JitArmV8CodeGen::loadFpuRegFromInt(FPURegPtr reg, RegPtr rm, RegPtr sib) {
    RegPtr tmp = getTmpReg();
    readHost(JitWidth::b32, createMemPtr(rm, sib, 0, 0), tmp);
    compiler.scvtf(toVec(reg), R32(tmp)); // convert int64 to double
}

void JitArmV8CodeGen::regToFpuReg(FPURegPtr dst, RegPtr src) {
    compiler.scvtf(toVec(dst), R32(src));
}

#ifdef BOXEDWINE_64
void JitArmV8CodeGen::regToFpuReg64(FPURegPtr dst, RegPtr src) {
    compiler.scvtf(toVec(dst), R64(src));
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

void JitArmV8CodeGen::IfLessThan() {
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
                IfLessThan(); {
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

void JitArmV8CodeGen::dynamic_rdtsc(DecodedOp* op) {
    compiler.mrs(xEAX, asmjit::a64::Predicate::SysReg::kCNTVCT_EL0);
    compiler.lsr(xEDX, xEAX, 32);
    compiler.mov(wEAX, wEAX); // zero out top 32-bits
}

void JitArmV8CodeGen::dynamic_cmpxchg8b_lock(DecodedOp* op) {
    JitCodeGen::write(JitWidth::b64, calculateEaa(op), nullptr, [op, this](RegPtr addressReg, RegPtr offsetReg) {
        U32 neededFlags = currentOp->needsToSetFlags(cpu) & ZF;
        if (neededFlags && currentOp->getNeededFlagsAfter(PF | SF | AF | CF | OF)) { // The ZF flag is set if the destination operand and EDX:EAX are equal; otherwise it is cleared. The CF, PF, AF, SF, and OF flags are unaffected.
            fillFlags();
        }
        compiler.add(R64(addressReg), R64(addressReg), R64(offsetReg));
        compiler.and_(R32(offsetReg), R32(offsetReg), 7);
        If(JitWidth::b32, offsetReg); {
            emulateSingleOp();
        } EndIf();
        if (rt.cpu_features().has(asmjit::CpuFeatures::ARM::kLRCPC)) {
            if (neededFlags) {
                // we need to make a copy of eax/edx so that we can compare the original values to what was in memory
                RegPtr tmp = getTmpReg();

                // must be sequential and the first one must be even
                if (regUsed[tmps[6]] || regUsed[tmps[7]] || (tmps[6] & 1)) {
                    kpanic("");
                }
                compiler.mov(wTmp7, wEAX);
                compiler.mov(wTmp8, wEDX);
                compiler.caspal(wTmp7, wTmp8, wEBX, wECX, Mem(R64(addressReg)));
                compiler.cmp(wEAX, wTmp7);
                compiler.ccmp(wEDX, wTmp8, 15, asmjit::a64::CondCode::kEQ); // if prev cmp was equal, then cmp edx,tmp2, else set flags                
                compiler.cset(R32(tmp), asmjit::a64::CondCode::kEQ); // eq means z flag is set
                compiler.bfi(xFLAGS, R32(tmp), 6, 1);
                compiler.mov(wEAX, wTmp7);
                compiler.mov(wEDX, wTmp8);
            } else {
                compiler.caspal(wEAX, wEDX, wEBX, wECX, Mem(R64(addressReg)));
            }
        } else {
            Label label = compiler.new_label();
            RegPtr tmp1 = getTmpReg();
            RegPtr tmp2 = getTmpReg();
            RegPtr tmp3 = getTmpReg();

            if (neededFlags) {
                compiler.bfc(xFLAGS, 6, 1); // clear ZF
            }
            compiler.bind(label);
            compiler.ldaxp(R32(tmp1), R32(tmp2), Mem(R64(addressReg)));
            compiler.cmp(wEAX, R32(tmp1));
            compiler.ccmp(wEDX, R32(tmp2), 15, asmjit::a64::CondCode::kEQ); // if prev cmp was equal, then cmp edx,tmp2, else set flags                

            IfEqual(); {
                compiler.stlxp(R32(tmp3), wEBX, wECX, Mem(R64(addressReg)));
                compiler.cbnz(R32(tmp3), label);
                if (neededFlags) {
                    compiler.orr(xFLAGS, xFLAGS, ZF);
                }
            } StartElse(); {
                compiler.clrex(15);
                compiler.mov(wEAX, R32(tmp1));
                compiler.mov(wEDX, R32(tmp2));
            } EndIf();
        }
    });
}

void JitArmV8CodeGen::dynamic_cmpxchg_lock(JitWidth width, DecodedOp* op) {
    JitCodeGen::write(JitWidth::b16, calculateEaa(op), nullptr, [width, op, this](RegPtr address, RegPtr offset) {
        U32 needsToSetFlags = 0;
        LazyFlagType flagType;
        
        if (width == JitWidth::b32) {
            flagType = FLAGS_CMP32;
        } else if (width == JitWidth::b16) {
            flagType = FLAGS_CMP16;
        } else if (width == JitWidth::b8) {
            flagType = FLAGS_CMP8;
        } else {
            kpanic("JitArmV8CodeGen::dynamic_cmpxchg_lock");
        }
        const LazyFlags* flags = lazyFlags[flagType];

        arithSetup(op, needsToSetFlags, flagType, nullptr);
        
        RegPtr tmp = getTmpReg();
        RegPtr tmp2 = getTmpReg();
        RegPtr reg = getReadOnlyRegInLower(width, op->reg);
        RegPtr eax = getReg(0);


        if (flags && flags->usesDst(needsToSetFlags)) {
            storeLazyFlagsDest(eax);
        }
        compiler.add(R64(address), R64(address), R64(offset));

        RegPtr tmpEax = eax;
        bool needToCopyEaxBack = false;
        if (width != JitWidth::b32 || (flags && (flags->usesSrc(needsToSetFlags) || flags->usesResult(needsToSetFlags)))) {
            tmpEax = getTmpReg();
            movzx(JitWidth::b32, tmpEax, width, eax);
            needToCopyEaxBack = true;
        }

        if (rt.cpu_features().has(asmjit::CpuFeatures::ARM::kLSE)) {
            if (width == JitWidth::b32) {
                compiler.casal(R32(tmpEax), R32(reg), Mem(R64(address)));
            } else if (width == JitWidth::b16) {
                compiler.casalh(R32(tmpEax), R32(reg), Mem(R64(address)));
            } else if (width == JitWidth::b8) {
                compiler.casalb(R32(tmpEax), R32(reg), Mem(R64(address)));
            }
            if (flags && flags->usesSrc(needsToSetFlags)) {
                storeLazyFlagsSrc(tmpEax);
            }
            if (flags && flags->usesResult(needsToSetFlags)) {
                compiler.sub(R32(tmp2), wEAX, R32(tmpEax));
                storeLazyFlagsResult(tmp2);
            }
            if (needToCopyEaxBack) {
                mov(width, eax, tmpEax);
            }
        } else {
            Label label = compiler.new_label();
            compiler.bind(label);
            if (width == JitWidth::b32) {
                compiler.ldaxr(R32(tmp), Mem(R64(address)));
            } else if (width == JitWidth::b16) {
                compiler.ldaxrh(R32(tmp), Mem(R64(address)));
            } else if (width == JitWidth::b8) {
                compiler.ldaxrb(R32(tmp), Mem(R64(address)));
            }
            IfEqual(JitWidth::b32, tmpEax, tmp); {
                if (width == JitWidth::b32) {
                    compiler.stlxr(R32(tmp2), R32(reg), Mem(R64(address)));
                } else if (width == JitWidth::b16) {
                    compiler.stlxrh(R32(tmp2), R32(reg), Mem(R64(address)));
                } else if (width == JitWidth::b8) {
                    compiler.stlxrb(R32(tmp2), R32(reg), Mem(R64(address)));
                }              
                compiler.cbnz(R32(tmp2), label);
            } StartElse(); {
                compiler.clrex(15);
                mov(width, eax, tmp);
            } EndIf();
            if (flags && flags->usesSrc(needsToSetFlags)) {
                storeLazyFlagsSrc(eax);
            }
            if (flags && flags->usesResult(needsToSetFlags)) {
                compiler.sub(R32(tmp2), R32(tmpEax), wEAX);
                storeLazyFlagsResult(tmp2);
            }
        }                                
    });
}

void JitArmV8CodeGen::dynamic_cmpxchge32r32_lock(DecodedOp* op) {
    dynamic_cmpxchg_lock(JitWidth::b32, op);
}

void JitArmV8CodeGen::dynamic_cmpxchge16r16_lock(DecodedOp* op) {
    dynamic_cmpxchg_lock(JitWidth::b16, op);
}

void JitArmV8CodeGen::dynamic_cmpxchge8r8_lock(DecodedOp* op) {
    dynamic_cmpxchg_lock(JitWidth::b8, op);
}

void JitArmV8CodeGen::ldaxr(JitWidth width, asmjit::a64::Gp reg, Mem mem) {
    switch (width) {
    case JitWidth::b8:
        compiler.ldaxrb(reg, mem);
        break;
    case JitWidth::b16:
        compiler.ldaxrh(reg, mem);
        break;
    case JitWidth::b32:
        compiler.ldaxr(reg, mem);
        break;
    default:
        kpanic("JitArmV8CodeGen::ldaxr");
    }
}

void JitArmV8CodeGen::stlxr(JitWidth width, asmjit::a64::Gp cond, asmjit::a64::Gp reg, Mem mem) {
    switch (width) {
    case JitWidth::b8:
        compiler.stlxrb(cond, reg, mem);
        break;
    case JitWidth::b16:
        compiler.stlxrh(cond, reg, mem);
        break;
    case JitWidth::b32:
        compiler.stlxr(cond, reg, mem);
        break;
    default:
        kpanic("JitArmV8CodeGen::stlxr");
    }
}

void JitArmV8CodeGen::casal(JitWidth width, asmjit::a64::Gp src, asmjit::a64::Gp dst, Mem mem) {
    switch (width) {
    case JitWidth::b8:
        compiler.casalb(src, dst, mem);
        break;
    case JitWidth::b16:
        compiler.casalh(src, dst, mem);
        break;
    case JitWidth::b32:
        compiler.casal(src, dst, mem);
        break;
    default:
        kpanic("JitArmV8CodeGen::casal");
    }
}

void JitArmV8CodeGen::dynamic_xchge32r32_lock(DecodedOp* op) {
    JitCodeGen::write(JitWidth::b32, calculateEaa(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        compiler.add(R64(address), R64(address), R64(offset));
        if (rt.cpu_features().has(asmjit::CpuFeatures::ARM::kLSE)) {            
            RegPtr reg = getReg(op->reg);
            compiler.swpal(R32(reg), R32(reg), Mem(R64(address)));
        } else {
            RegPtr tmp = getTmpReg(op->reg);
            RegPtr reg = getReg(op->reg);
            RegPtr cond = getTmpReg();
            Label label = compiler.new_label();

            compiler.bind(label);            

            compiler.ldaxr(R32(reg), Mem(R64(address)));
            compiler.stlxr(R32(cond), R32(tmp), Mem(R64(address)));
            compiler.cbnz(R32(cond), label);
        }
    });
}

void JitArmV8CodeGen::dynamic_xchge16r16_lock(DecodedOp* op) {
    JitCodeGen::write(JitWidth::b16, calculateEaa(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        compiler.add(R64(address), R64(address), R64(offset));
        if (rt.cpu_features().has(asmjit::CpuFeatures::ARM::kLSE)) {            
            RegPtr tmp = getTmpReg(op->reg);
            RegPtr reg = getReg(op->reg);
            compiler.swpalh(R32(tmp), R32(tmp), Mem(R64(address))); // will zero extend, which is why its a tmp
            mov(JitWidth::b16, reg, tmp);
        } else {
            RegPtr reg = getReg(op->reg);
            RegPtr cond = getTmpReg();
            RegPtr src = getTmpReg();
            Label label = compiler.new_label();

            compiler.bind(label);

            compiler.ldaxrh(R32(src), Mem(R64(address)));
            compiler.stlxrh(R32(cond), R32(reg), Mem(R64(address)));
            compiler.cbnz(R32(cond), label);
            mov(JitWidth::b16, reg, src);
        }
    });
}

void JitArmV8CodeGen::dynamic_xchge8r8_lock(DecodedOp* op) {
    JitCodeGen::write(JitWidth::b8, calculateEaa(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        compiler.add(R64(address), R64(address), R64(offset));
        if (rt.cpu_features().has(asmjit::CpuFeatures::ARM::kLSE)) {            
            RegPtr reg = getReg8(op->reg);
            RegPtr reg8 = getTmpIfNotTmp(getReg8InLowByte(reg));

            compiler.swpalb(R32(reg8), R32(reg8), Mem(R64(address)));
            mov(JitWidth::b8, reg, reg8);
        } else {
            RegPtr reg = getReg8(op->reg);
            RegPtr reg8 = getTmpIfNotTmp(getReg8InLowByte(reg));
            RegPtr cond = getTmpReg();
            RegPtr src = getTmpReg();
            Label label = compiler.new_label();

            compiler.bind(label);

            compiler.ldaxrb(R32(src), Mem(R64(address)));
            compiler.stlxrb(R32(cond), R32(reg8), Mem(R64(address)));
            compiler.cbnz(R32(cond), label);
            mov(JitWidth::b8, reg, src);
        }
    });
}

void JitArmV8CodeGen::dynamic_arith_lock(JitWidth width, DecodedOp* op, LazyFlagType flagsType, std::function<void(RegPtr src, RegPtr dst, RegPtr address)> atomicCallback, std::function<void(RegPtr result, RegPtr dstMem, RegPtr srcReg)> callback, bool writebackReg, bool addCF, bool immSrc, bool hasSrc) {
    JitCodeGen::write(width, calculateEaa(op), nullptr, [hasSrc, immSrc, addCF, atomicCallback, writebackReg, width, flagsType, op, callback, this](RegPtr address, RegPtr offset) {
        compiler.add(R64(address), R64(address), R64(offset));
        U32 needsToSetFlags = 0;
        const LazyFlags* flags = lazyFlags[flagsType];
        bool direct = width == JitWidth::b32 && !(flags && (flags->usesResult(needsToSetFlags) || flags->usesDst(needsToSetFlags)));
        RegPtr reg;
        
        if (hasSrc) {
            reg = immSrc ? loadConst(op->imm) : direct ? getReg(width, op->reg) : getReadOnlyRegInLower(width, op->reg);
        }
        RegPtr cf = addCF ? getCF() : nullptr;

        arithSetup(op, needsToSetFlags, flagsType, cf);

        if (hasSrc && flags && flags->usesSrc(needsToSetFlags)) {
            storeLazyFlagsSrc(reg);
        }

        RegPtr dst = getTmpReg();
        RegPtr result = getTmpReg();

        if (addCF) {
            reg = getTmpIfNotTmp(reg);
            addReg(width, reg, cf);
        }
#ifdef BOXEDWINE_HOST_EXCEPTIONS
        if (atomicCallback && rt.cpu_features().has(asmjit::CpuFeatures::ARM::kLSE)) {
            if (width != JitWidth::b32 || (flags && (flags->usesResult(needsToSetFlags) || flags->usesDst(needsToSetFlags)))) {
                atomicCallback(reg, dst, address);
                if (flags && flags->usesResult(needsToSetFlags)) {
                    callback(result, dst, reg);
                }
                if (writebackReg) {
                    reg = getReg(width, op->reg);
                    mov(width, reg, dst);
                }
            } else {
                atomicCallback(reg, reg, address);
            }
        } else 
#endif
        if (rt.cpu_features().has(asmjit::CpuFeatures::ARM::kLSE)) {
            Label label = compiler.new_label();
            RegPtr cond = getTmpReg();
            RegPtr prvMemSrc = getTmpReg();

            readHost(width, createMemPtr(address, 0), dst, false);
            
            compiler.bind(label);
            
            compiler.mov(R32(prvMemSrc), R32(dst));
            callback(result, dst, reg);
            casal(width, R32(dst), R32(result), Mem(R64(address)));
            compiler.sub(R32(cond), R32(dst), R32(prvMemSrc));
            compiler.cbnz(R32(cond), label);
            dst = prvMemSrc;
            if (writebackReg) {
                reg = getReg(width, op->reg);
                mov(width, reg, dst);
            }
        } else {
            Label label = compiler.new_label();
            RegPtr cond = getTmpReg();

            compiler.bind(label);
            ldaxr(width, R32(dst), Mem(R64(address)));
            callback(result, dst, reg);
            stlxr(width, R32(cond), R32(result), Mem(R64(address)));
            compiler.cbnz(R32(cond), label);

            if (writebackReg) {
                reg = width == JitWidth::b8 ? getReg8(op->reg) : getReg(op->reg);
                mov(width, reg, dst);
            }
        }
        // this is a bit backwards to neg flags, perhaps, should switch to using dst instead of src for neg flags
        if (!hasSrc && flags && flags->usesSrc(needsToSetFlags)) {
            storeLazyFlagsSrc(dst);
        } else if (flags && flags->usesDst(needsToSetFlags)) {
            storeLazyFlagsDest(dst);
        }
        if (flags && flags->usesResult(needsToSetFlags)) {
            storeLazyFlagsResult(result);
        }
    });
}

void JitArmV8CodeGen::dynamic_arith_value_lock(JitWidth width, DecodedOp* op, U32 value, LazyFlagType flagsType, std::function<void(RegPtr src, RegPtr dst, RegPtr address)> atomicCallback, std::function<void(RegPtr result, RegPtr dst, U32 src)> callback) {
    JitCodeGen::write(width, calculateEaa(op), nullptr, [value, atomicCallback, width, flagsType, op, callback, this](RegPtr address, RegPtr offset) {
        compiler.add(R64(address), R64(address), R64(offset));
        U32 needsToSetFlags = 0;
        const LazyFlags* flags = lazyFlags[flagsType];
        bool direct = width == JitWidth::b32 && !(flags && (flags->usesResult(needsToSetFlags) || flags->usesDst(needsToSetFlags)));

        arithSetup(op, needsToSetFlags, flagsType, nullptr);

        if (flags && flags->usesSrc(needsToSetFlags)) {
            storeLazyFlagsSrc(value);
        }

        RegPtr dst = getTmpReg();
        RegPtr result = getTmpReg();
        
#ifdef BOXEDWINE_HOST_EXCEPTIONS
        if (rt.cpu_features().has(asmjit::CpuFeatures::ARM::kLSE)) {
            RegPtr reg = loadConst(value);
            if (width != JitWidth::b32 || (flags && (flags->usesResult(needsToSetFlags) || flags->usesDst(needsToSetFlags)))) {
                atomicCallback(reg, dst, address);
                if (flags && flags->usesResult(needsToSetFlags)) {
                    callback(result, dst, value);
                }
            } else {
                atomicCallback(reg, reg, address);
            }
        } else 
#endif
        if (rt.cpu_features().has(asmjit::CpuFeatures::ARM::kLSE)) {
            Label label = compiler.new_label();
            RegPtr cond = getTmpReg();
            RegPtr prvMemSrc = getTmpReg();

            readHost(width, createMemPtr(address, 0), dst, false);

            compiler.bind(label);

            compiler.mov(R32(prvMemSrc), R32(dst));
            callback(result, dst, value);
            casal(width, R32(dst), R32(result), Mem(R64(address)));
            compiler.sub(R32(cond), R32(dst), R32(prvMemSrc));
            compiler.cbnz(R32(cond), label);
            dst = prvMemSrc;
        } else {
            Label label = compiler.new_label();
            RegPtr cond = getTmpReg();

            compiler.bind(label);
            ldaxr(width, R32(dst), Mem(R64(address)));
            callback(result, dst, value);
            stlxr(width, R32(cond), R32(result), Mem(R64(address)));
            compiler.cbnz(R32(cond), label);
        }
        if (flags && flags->usesDst(needsToSetFlags)) {
            storeLazyFlagsDest(dst);
        }
        if (flags && flags->usesResult(needsToSetFlags)) {
            storeLazyFlagsResult(result);
        }
    });
}
/*
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
*/

void JitArmV8CodeGen::dynamic_xaddr32e32_lock(DecodedOp* op) {
    dynamic_arith_lock(JitWidth::b32, op, FLAGS_ADD32, [this](RegPtr src, RegPtr dst, RegPtr address) {
        compiler.ldaddal(R32(src), R32(dst), Mem(R64(address)));
    }, [this](RegPtr result, RegPtr dstMem, RegPtr srcReg) {
        compiler.add(R32(result), R32(dstMem), R32(srcReg));
    }, true);
}
void JitArmV8CodeGen::dynamic_xaddr16e16_lock(DecodedOp* op) {
    dynamic_arith_lock(JitWidth::b16, op, FLAGS_ADD16, [this](RegPtr src, RegPtr dst, RegPtr address) {
        compiler.ldaddalh(R32(src), R32(dst), Mem(R64(address)));
    }, [this](RegPtr result, RegPtr dstMem, RegPtr srcReg) {
        compiler.add(R32(result), R32(dstMem), R32(srcReg));
    }, true);
}
void JitArmV8CodeGen::dynamic_xaddr8e8_lock(DecodedOp* op) {
    dynamic_arith_lock(JitWidth::b8, op, FLAGS_ADD8, [this](RegPtr src, RegPtr dst, RegPtr address) {
        compiler.ldaddalb(R32(src), R32(dst), Mem(R64(address)));
    }, [this](RegPtr result, RegPtr dstMem, RegPtr srcReg) {
        compiler.add(R32(result), R32(dstMem), R32(srcReg));
    }, true);
}
void JitArmV8CodeGen::dynamic_adde32r32_lock(DecodedOp* op) {
    dynamic_arith_lock(JitWidth::b32, op, FLAGS_ADD32, [this](RegPtr src, RegPtr dst, RegPtr address) {
        compiler.ldaddal(R32(src), R32(dst), Mem(R64(address)));
    }, [this](RegPtr result, RegPtr dstMem, RegPtr srcReg) {
        compiler.add(R32(result), R32(dstMem), R32(srcReg));
    });
}
void JitArmV8CodeGen::dynamic_adde16r16_lock(DecodedOp* op) {
    dynamic_arith_lock(JitWidth::b16, op, FLAGS_ADD16, [this](RegPtr src, RegPtr dst, RegPtr address) {
        compiler.ldaddalh(R32(src), R32(dst), Mem(R64(address)));
    }, [this](RegPtr result, RegPtr dstMem, RegPtr srcReg) {
        compiler.add(R32(result), R32(dstMem), R32(srcReg));
    });
}
void JitArmV8CodeGen::dynamic_adde8r8_lock(DecodedOp* op) {
    dynamic_arith_lock(JitWidth::b8, op, FLAGS_ADD8, [this](RegPtr src, RegPtr dst, RegPtr address) {
        compiler.ldaddalb(R32(src), R32(dst), Mem(R64(address)));
    }, [this](RegPtr result, RegPtr dstMem, RegPtr srcReg) {
        compiler.add(R32(result), R32(dstMem), R32(srcReg));
    });
}

void JitArmV8CodeGen::dynamic_add32_mem_lock(DecodedOp* op) {
    dynamic_arith_value_lock(JitWidth::b32, op, op->imm, FLAGS_ADD32, [this](RegPtr src, RegPtr dst, RegPtr address) {
        compiler.ldaddal(R32(src), R32(dst), Mem(R64(address)));
    }, [this](RegPtr result, RegPtr dstMem, U32 src) {
        if (asmjit::a64::Utils::is_add_sub_imm(src)) {
            compiler.add(R32(result), R32(dstMem), src);
        } else {
            compiler.add(R32(result), R32(dstMem), R32(loadConst(src)));
        }
    });
}
void JitArmV8CodeGen::dynamic_add16_mem_lock(DecodedOp* op) {
    dynamic_arith_value_lock(JitWidth::b16, op, op->imm, FLAGS_ADD16, [this](RegPtr src, RegPtr dst, RegPtr address) {
        compiler.ldaddalh(R32(src), R32(dst), Mem(R64(address)));
    }, [this](RegPtr result, RegPtr dstMem, U32 src) {
        if (asmjit::a64::Utils::is_add_sub_imm(src)) {
            compiler.add(R32(result), R32(dstMem), src);
        } else {
            compiler.add(R32(result), R32(dstMem), R32(loadConst(src)));
        }
    });
}
void JitArmV8CodeGen::dynamic_add8_mem_lock(DecodedOp* op) {
    dynamic_arith_value_lock(JitWidth::b8, op, op->imm, FLAGS_ADD8, [this](RegPtr src, RegPtr dst, RegPtr address) {
        compiler.ldaddalb(R32(src), R32(dst), Mem(R64(address)));
    }, [this](RegPtr result, RegPtr dstMem, U32 src) {
        if (asmjit::a64::Utils::is_add_sub_imm(src)) {
            compiler.add(R32(result), R32(dstMem), src);
        } else {
            compiler.add(R32(result), R32(dstMem), R32(loadConst(src)));
        }
    });
}
void JitArmV8CodeGen::dynamic_adce32r32_lock(DecodedOp* op) {
    dynamic_arith_lock(JitWidth::b32, op, FLAGS_ADC32, [this](RegPtr src, RegPtr dst, RegPtr address) {
        compiler.ldaddal(R32(src), R32(dst), Mem(R64(address)));
    }, [this](RegPtr result, RegPtr dstMem, RegPtr srcReg) {
        compiler.add(R32(result), R32(dstMem), R32(srcReg));
    }, false, true);
}
void JitArmV8CodeGen::dynamic_adce16r16_lock(DecodedOp* op) {
    dynamic_arith_lock(JitWidth::b16, op, FLAGS_ADC16, [this](RegPtr src, RegPtr dst, RegPtr address) {
        compiler.ldaddalh(R32(src), R32(dst), Mem(R64(address)));
    }, [this](RegPtr result, RegPtr dstMem, RegPtr srcReg) {
        compiler.add(R32(result), R32(dstMem), R32(srcReg));
    }, false, true);
}
void JitArmV8CodeGen::dynamic_adce8r8_lock(DecodedOp* op) {
    dynamic_arith_lock(JitWidth::b8, op, FLAGS_ADC8, [this](RegPtr src, RegPtr dst, RegPtr address) {
        compiler.ldaddalb(R32(src), R32(dst), Mem(R64(address)));
    }, [this](RegPtr result, RegPtr dstMem, RegPtr srcReg) {
        compiler.add(R32(result), R32(dstMem), R32(srcReg));
    }, false, true);
}

void JitArmV8CodeGen::dynamic_adc32_mem_lock(DecodedOp* op) {
    dynamic_arith_lock(JitWidth::b32, op, FLAGS_ADC32, [this](RegPtr src, RegPtr dst, RegPtr address) {
        compiler.ldaddal(R32(src), R32(dst), Mem(R64(address)));
    }, [this](RegPtr result, RegPtr dstMem, RegPtr srcReg) {
        compiler.add(R32(result), R32(dstMem), R32(srcReg));
    }, false, true, true);
}
void JitArmV8CodeGen::dynamic_adc16_mem_lock(DecodedOp* op) {
    dynamic_arith_lock(JitWidth::b16, op, FLAGS_ADC16, [this](RegPtr src, RegPtr dst, RegPtr address) {
        compiler.ldaddalh(R32(src), R32(dst), Mem(R64(address)));
    }, [this](RegPtr result, RegPtr dstMem, RegPtr srcReg) {
        compiler.add(R32(result), R32(dstMem), R32(srcReg));
    }, false, true, true);
}
void JitArmV8CodeGen::dynamic_adc8_mem_lock(DecodedOp* op) {
    dynamic_arith_lock(JitWidth::b8, op, FLAGS_ADC8, [this](RegPtr src, RegPtr dst, RegPtr address) {
        compiler.ldaddalb(R32(src), R32(dst), Mem(R64(address)));
    }, [this](RegPtr result, RegPtr dstMem, RegPtr srcReg) {
        compiler.add(R32(result), R32(dstMem), R32(srcReg));
    }, false, true, true);
}
void JitArmV8CodeGen::dynamic_sube32r32_lock(DecodedOp* op) {
    dynamic_arith_lock(JitWidth::b32, op, FLAGS_SUB32, [this](RegPtr src, RegPtr dst, RegPtr address) {
        if (src->hardwareReg() != dst->hardwareReg()) {
            RegPtr tmp = getTmpReg();
            compiler.neg(R32(tmp), R32(src));
            src = tmp;
        } else {
            src = getTmpIfNotTmp(src);
            compiler.neg(R32(src), R32(src));
        }                
        compiler.ldaddal(R32(src), R32(dst), Mem(R64(address)));
    }, [this](RegPtr result, RegPtr dstMem, RegPtr srcReg) {
        compiler.sub(R32(result), R32(dstMem), R32(srcReg));
    });
}
void JitArmV8CodeGen::dynamic_sube16r16_lock(DecodedOp* op) {
    dynamic_arith_lock(JitWidth::b16, op, FLAGS_SUB16, [this](RegPtr src, RegPtr dst, RegPtr address) {
        RegPtr tmp = getTmpReg();
        movsx(JitWidth::b32, tmp, JitWidth::b16, src);
        compiler.neg(R32(tmp), R32(tmp));
        compiler.ldaddalh(R32(tmp), R32(dst), Mem(R64(address)));
    }, [this](RegPtr result, RegPtr dstMem, RegPtr srcReg) {
        compiler.sub(R32(result), R32(dstMem), R32(srcReg));
    });
}
void JitArmV8CodeGen::dynamic_sube8r8_lock(DecodedOp* op) {
    dynamic_arith_lock(JitWidth::b8, op, FLAGS_SUB8, [this](RegPtr src, RegPtr dst, RegPtr address) {
        RegPtr tmp = getTmpReg();
        movsx(JitWidth::b32, tmp, JitWidth::b8, src);
        compiler.neg(R32(tmp), R32(tmp));
        compiler.ldaddalb(R32(tmp), R32(dst), Mem(R64(address)));
    }, [this](RegPtr result, RegPtr dstMem, RegPtr srcReg) {
        compiler.sub(R32(result), R32(dstMem), R32(srcReg));
    });
}
void JitArmV8CodeGen::dynamic_sub32_mem_lock(DecodedOp* op) {
    dynamic_arith_value_lock(JitWidth::b32, op, op->imm, FLAGS_SUB32, [this](RegPtr src, RegPtr dst, RegPtr address) {
        if (src->hardwareReg() != dst->hardwareReg()) {
            RegPtr tmp = getTmpReg();
            compiler.neg(R32(tmp), R32(src));
            src = tmp;
        } else {
            src = getTmpIfNotTmp(src);
            compiler.neg(R32(src), R32(src));
        }
        compiler.ldaddal(R32(src), R32(dst), Mem(R64(address)));
    }, [this](RegPtr result, RegPtr dstMem, U32 src) {
        if (asmjit::a64::Utils::is_add_sub_imm(src)) {
            compiler.sub(R32(result), R32(dstMem), src);
        } else {
            compiler.sub(R32(result), R32(dstMem), R32(loadConst(src)));
        }
    });
}
void JitArmV8CodeGen::dynamic_sub16_mem_lock(DecodedOp* op) {
    dynamic_arith_value_lock(JitWidth::b16, op, op->imm, FLAGS_SUB16, [this](RegPtr src, RegPtr dst, RegPtr address) {
        RegPtr tmp = getTmpReg();
        movsx(JitWidth::b32, tmp, JitWidth::b16, src);
        compiler.neg(R32(tmp), R32(tmp));
        compiler.ldaddalh(R32(tmp), R32(dst), Mem(R64(address)));
    }, [this](RegPtr result, RegPtr dstMem, U32 src) {
        if (asmjit::a64::Utils::is_add_sub_imm(src)) {
            compiler.sub(R32(result), R32(dstMem), src);
        } else {
            compiler.sub(R32(result), R32(dstMem), R32(loadConst(src)));
        }
    });
}
void JitArmV8CodeGen::dynamic_sub8_mem_lock(DecodedOp* op) {
    dynamic_arith_value_lock(JitWidth::b8, op, op->imm, FLAGS_SUB8, [this](RegPtr src, RegPtr dst, RegPtr address) {
        RegPtr tmp = getTmpReg();
        movsx(JitWidth::b32, tmp, JitWidth::b8, src);
        compiler.neg(R32(tmp), R32(tmp));
        compiler.ldaddalb(R32(tmp), R32(dst), Mem(R64(address)));
    }, [this](RegPtr result, RegPtr dstMem, U32 src) {
        if (asmjit::a64::Utils::is_add_sub_imm(src)) {
            compiler.sub(R32(result), R32(dstMem), src);
        } else {
            compiler.sub(R32(result), R32(dstMem), R32(loadConst(src)));
        }
    });
}
void JitArmV8CodeGen::dynamic_sbbe32r32_lock(DecodedOp* op) {
    dynamic_arith_lock(JitWidth::b32, op, FLAGS_SBB32, [this](RegPtr src, RegPtr dst, RegPtr address) {
        if (src->hardwareReg() != dst->hardwareReg()) {
            RegPtr tmp = getTmpReg();
            compiler.neg(R32(tmp), R32(src));
            src = tmp;
        } else {
            src = getTmpIfNotTmp(src);
            compiler.neg(R32(src), R32(src));
        }
        compiler.ldaddal(R32(src), R32(dst), Mem(R64(address)));
    }, [this](RegPtr result, RegPtr dstMem, RegPtr srcReg) {
        compiler.sub(R32(result), R32(dstMem), R32(srcReg));
    }, false, true);
}
void JitArmV8CodeGen::dynamic_sbbe16r16_lock(DecodedOp* op) {
    dynamic_arith_lock(JitWidth::b16, op, FLAGS_SBB16, [this](RegPtr src, RegPtr dst, RegPtr address) {
        RegPtr tmp = getTmpReg();
        movsx(JitWidth::b32, tmp, JitWidth::b16, src);
        compiler.neg(R32(tmp), R32(tmp));
        compiler.ldaddalh(R32(tmp), R32(dst), Mem(R64(address)));
    }, [this](RegPtr result, RegPtr dstMem, RegPtr srcReg) {
        compiler.sub(R32(result), R32(dstMem), R32(srcReg));
    }, false, true);
}
void JitArmV8CodeGen::dynamic_sbbe8r8_lock(DecodedOp* op) {
    dynamic_arith_lock(JitWidth::b8, op, FLAGS_SBB8, [this](RegPtr src, RegPtr dst, RegPtr address) {
        RegPtr tmp = getTmpReg();
        movsx(JitWidth::b32, tmp, JitWidth::b8, src);
        compiler.neg(R32(tmp), R32(tmp));
        compiler.ldaddalb(R32(tmp), R32(dst), Mem(R64(address)));
    }, [this](RegPtr result, RegPtr dstMem, RegPtr srcReg) {
        compiler.sub(R32(result), R32(dstMem), R32(srcReg));
    }, false, true);
}
void JitArmV8CodeGen::dynamic_sbb32_mem_lock(DecodedOp* op) {
    dynamic_arith_lock(JitWidth::b32, op, FLAGS_SBB32, [this](RegPtr src, RegPtr dst, RegPtr address) {
        if (src->hardwareReg() != dst->hardwareReg()) {
            RegPtr tmp = getTmpReg();
            compiler.neg(R32(tmp), R32(src));
            src = tmp;
        } else {
            src = getTmpIfNotTmp(src);
            compiler.neg(R32(src), R32(src));
        }
        compiler.ldaddal(R32(src), R32(dst), Mem(R64(address)));
    }, [this](RegPtr result, RegPtr dstMem, RegPtr srcReg) {
        compiler.sub(R32(result), R32(dstMem), R32(srcReg));
    }, false, true, true);
}
void JitArmV8CodeGen::dynamic_sbb16_mem_lock(DecodedOp* op) {
    dynamic_arith_lock(JitWidth::b16, op, FLAGS_SBB16, [this](RegPtr src, RegPtr dst, RegPtr address) {
        RegPtr tmp = getTmpReg();
        movsx(JitWidth::b32, tmp, JitWidth::b16, src);
        compiler.neg(R32(tmp), R32(tmp));
        compiler.ldaddalh(R32(tmp), R32(dst), Mem(R64(address)));
    }, [this](RegPtr result, RegPtr dstMem, RegPtr srcReg) {
        compiler.sub(R32(result), R32(dstMem), R32(srcReg));
    }, false, true, true);
}
void JitArmV8CodeGen::dynamic_sbb8_mem_lock(DecodedOp* op) {
    dynamic_arith_lock(JitWidth::b8, op, FLAGS_SBB8, [this](RegPtr src, RegPtr dst, RegPtr address) {
        RegPtr tmp = getTmpReg();
        movsx(JitWidth::b32, tmp, JitWidth::b8, src);
        compiler.neg(R32(tmp), R32(tmp));
        compiler.ldaddalb(R32(tmp), R32(dst), Mem(R64(address)));
    }, [this](RegPtr result, RegPtr dstMem, RegPtr srcReg) {
        compiler.sub(R32(result), R32(dstMem), R32(srcReg));
    }, false, true, true);
}
void JitArmV8CodeGen::dynamic_ore32r32_lock(DecodedOp* op) {
    dynamic_arith_lock(JitWidth::b32, op, FLAGS_OR32, [this](RegPtr src, RegPtr dst, RegPtr address) {
        compiler.ldsetal(R32(src), R32(dst), Mem(R64(address)));
    }, [this](RegPtr result, RegPtr dstMem, RegPtr srcReg) {
        compiler.orr(R32(result), R32(dstMem), R32(srcReg));
    });
}
void JitArmV8CodeGen::dynamic_ore16r16_lock(DecodedOp* op) {
    dynamic_arith_lock(JitWidth::b16, op, FLAGS_OR16, [this](RegPtr src, RegPtr dst, RegPtr address) {
        compiler.ldsetalh(R32(src), R32(dst), Mem(R64(address)));
    }, [this](RegPtr result, RegPtr dstMem, RegPtr srcReg) {
        compiler.orr(R32(result), R32(dstMem), R32(srcReg));
    });
}
void JitArmV8CodeGen::dynamic_ore8r8_lock(DecodedOp* op) {
    dynamic_arith_lock(JitWidth::b8, op, FLAGS_OR8, [this](RegPtr src, RegPtr dst, RegPtr address) {
        compiler.ldsetalb(R32(src), R32(dst), Mem(R64(address)));
    }, [this](RegPtr result, RegPtr dstMem, RegPtr srcReg) {
        compiler.orr(R32(result), R32(dstMem), R32(srcReg));
    });
}
void JitArmV8CodeGen::dynamic_or32_mem_lock(DecodedOp* op) {
    dynamic_arith_value_lock(JitWidth::b32, op, op->imm, FLAGS_OR32, [this](RegPtr src, RegPtr dst, RegPtr address) {
        compiler.ldsetal(R32(src), R32(dst), Mem(R64(address)));
    }, [this](RegPtr result, RegPtr dstMem, U32 src) {
        if (asmjit::a64::Utils::is_logical_imm(src, 32)) {
            compiler.orr(R32(result), R32(dstMem), src);
        } else {
            compiler.orr(R32(result), R32(dstMem), R32(loadConst(src)));
        }
    });
}
void JitArmV8CodeGen::dynamic_or16_mem_lock(DecodedOp* op) {
    dynamic_arith_value_lock(JitWidth::b16, op, op->imm, FLAGS_OR16, [this](RegPtr src, RegPtr dst, RegPtr address) {
        compiler.ldsetalh(R32(src), R32(dst), Mem(R64(address)));
    }, [this](RegPtr result, RegPtr dstMem, U32 src) {
        if (asmjit::a64::Utils::is_logical_imm(src, 32)) {
            compiler.orr(R32(result), R32(dstMem), src);
        } else {
            compiler.orr(R32(result), R32(dstMem), R32(loadConst(src)));
        }
    });
}
void JitArmV8CodeGen::dynamic_or8_mem_lock(DecodedOp* op) {
    dynamic_arith_value_lock(JitWidth::b8, op, op->imm, FLAGS_OR8, [this](RegPtr src, RegPtr dst, RegPtr address) {
        compiler.ldsetalb(R32(src), R32(dst), Mem(R64(address)));
    }, [this](RegPtr result, RegPtr dstMem, U32 src) {
        if (asmjit::a64::Utils::is_logical_imm(src, 32)) {
            compiler.orr(R32(result), R32(dstMem), src);
        } else {
            compiler.orr(R32(result), R32(dstMem), R32(loadConst(src)));
        }
    });
}
void JitArmV8CodeGen::dynamic_ande32r32_lock(DecodedOp* op) {
    dynamic_arith_lock(JitWidth::b32, op, FLAGS_AND32, [this](RegPtr src, RegPtr dst, RegPtr address) {
        if (src->hardwareReg() != dst->hardwareReg()) {
            RegPtr tmp = getTmpReg();
            compiler.mvn_(R32(tmp), R32(src));
            src = tmp;
        } else {
            src = getTmpIfNotTmp(src);
            compiler.mvn_(R32(src), R32(src));
        }
        compiler.ldclral(R32(src), R32(dst), Mem(R64(address)));
    }, [this](RegPtr result, RegPtr dstMem, RegPtr srcReg) {
        compiler.and_(R32(result), R32(dstMem), R32(srcReg));
    });
}
void JitArmV8CodeGen::dynamic_ande16r16_lock(DecodedOp* op) {
    dynamic_arith_lock(JitWidth::b16, op, FLAGS_AND16, [this](RegPtr src, RegPtr dst, RegPtr address) {
        RegPtr tmp = getTmpReg();
        compiler.mvn_(R32(tmp), R32(src));
        compiler.ldclralh(R32(tmp), R32(dst), Mem(R64(address)));
    }, [this](RegPtr result, RegPtr dstMem, RegPtr srcReg) {
        compiler.and_(R32(result), R32(dstMem), R32(srcReg));
    });
}
void JitArmV8CodeGen::dynamic_ande8r8_lock(DecodedOp* op) {
    dynamic_arith_lock(JitWidth::b8, op, FLAGS_AND8, [this](RegPtr src, RegPtr dst, RegPtr address) {
        RegPtr tmp = getTmpReg();
        compiler.mvn_(R32(tmp), R32(src));
        compiler.ldclralb(R32(tmp), R32(dst), Mem(R64(address)));
    }, [this](RegPtr result, RegPtr dstMem, RegPtr srcReg) {
        compiler.and_(R32(result), R32(dstMem), R32(srcReg));
    });
}
void JitArmV8CodeGen::dynamic_and32_mem_lock(DecodedOp* op) {
    dynamic_arith_value_lock(JitWidth::b32, op, op->imm, FLAGS_AND32, [this](RegPtr src, RegPtr dst, RegPtr address) {
        if (src->hardwareReg() != dst->hardwareReg()) {
            RegPtr tmp = getTmpReg();
            compiler.mvn_(R32(tmp), R32(src));
            src = tmp;
        } else {
            src = getTmpIfNotTmp(src);
            compiler.mvn_(R32(src), R32(src));
        }
        compiler.ldclral(R32(src), R32(dst), Mem(R64(address)));
    }, [this](RegPtr result, RegPtr dstMem, U32 src) {
        if (asmjit::a64::Utils::is_logical_imm(src, 32)) {
            compiler.and_(R32(result), R32(dstMem), src);
        } else {
            compiler.and_(R32(result), R32(dstMem), R32(loadConst(src)));
        }
    });
}
void JitArmV8CodeGen::dynamic_and16_mem_lock(DecodedOp* op) {
    dynamic_arith_value_lock(JitWidth::b16, op, op->imm, FLAGS_AND16, [this](RegPtr src, RegPtr dst, RegPtr address) {
        RegPtr tmp = getTmpReg();
        compiler.mvn_(R32(tmp), R32(src));
        compiler.ldclralh(R32(tmp), R32(dst), Mem(R64(address)));
    }, [this](RegPtr result, RegPtr dstMem, U32 src) {
        if (asmjit::a64::Utils::is_logical_imm(src, 32)) {
            compiler.and_(R32(result), R32(dstMem), src);
        } else {
            compiler.and_(R32(result), R32(dstMem), R32(loadConst(src)));
        }
    });
}
void JitArmV8CodeGen::dynamic_and8_mem_lock(DecodedOp* op) {
    dynamic_arith_value_lock(JitWidth::b8, op, op->imm, FLAGS_AND8, [this](RegPtr src, RegPtr dst, RegPtr address) {
        RegPtr tmp = getTmpReg();
        compiler.mvn_(R32(tmp), R32(src));
        compiler.ldclralb(R32(tmp), R32(dst), Mem(R64(address)));
    }, [this](RegPtr result, RegPtr dstMem, U32 src) {
        if (asmjit::a64::Utils::is_logical_imm(src, 32)) {
            compiler.and_(R32(result), R32(dstMem), src);
        } else {
            compiler.and_(R32(result), R32(dstMem), R32(loadConst(src)));
        }
    });
}
void JitArmV8CodeGen::dynamic_xore32r32_lock(DecodedOp* op) {
    dynamic_arith_lock(JitWidth::b32, op, FLAGS_XOR32, [this](RegPtr src, RegPtr dst, RegPtr address) {
        compiler.ldeoral(R32(src), R32(dst), Mem(R64(address)));
    }, [this](RegPtr result, RegPtr dstMem, RegPtr srcReg) {
        compiler.eor(R32(result), R32(dstMem), R32(srcReg));
    });
}
void JitArmV8CodeGen::dynamic_xore16r16_lock(DecodedOp* op) {
    dynamic_arith_lock(JitWidth::b16, op, FLAGS_XOR16, [this](RegPtr src, RegPtr dst, RegPtr address) {
        compiler.ldeoralh(R32(src), R32(dst), Mem(R64(address)));
    }, [this](RegPtr result, RegPtr dstMem, RegPtr srcReg) {
        compiler.eor(R32(result), R32(dstMem), R32(srcReg));
    });
}
void JitArmV8CodeGen::dynamic_xore8r8_lock(DecodedOp* op) {
    dynamic_arith_lock(JitWidth::b8, op, FLAGS_XOR8, [this](RegPtr src, RegPtr dst, RegPtr address) {
        compiler.ldeoralb(R32(src), R32(dst), Mem(R64(address)));
    }, [this](RegPtr result, RegPtr dstMem, RegPtr srcReg) {
        compiler.eor(R32(result), R32(dstMem), R32(srcReg));
    });
}
void JitArmV8CodeGen::dynamic_xor32_mem_lock(DecodedOp* op) {
    dynamic_arith_value_lock(JitWidth::b32, op, op->imm, FLAGS_XOR32, [this](RegPtr src, RegPtr dst, RegPtr address) {
        compiler.ldeoral(R32(src), R32(dst), Mem(R64(address)));
    }, [this](RegPtr result, RegPtr dstMem, U32 src) {
        if (asmjit::a64::Utils::is_logical_imm(src, 32)) {
            compiler.eor(R32(result), R32(dstMem), src);
        } else {
            compiler.eor(R32(result), R32(dstMem), R32(loadConst(src)));
        }
    });
}
void JitArmV8CodeGen::dynamic_xor16_mem_lock(DecodedOp* op) {
    dynamic_arith_value_lock(JitWidth::b16, op, op->imm, FLAGS_XOR16, [this](RegPtr src, RegPtr dst, RegPtr address) {
        compiler.ldeoralh(R32(src), R32(dst), Mem(R64(address)));
    }, [this](RegPtr result, RegPtr dstMem, U32 src) {
        if (asmjit::a64::Utils::is_logical_imm(src, 32)) {
            compiler.eor(R32(result), R32(dstMem), src);
        } else {
            compiler.eor(R32(result), R32(dstMem), R32(loadConst(src)));
        }
    });
}
void JitArmV8CodeGen::dynamic_xor8_mem_lock(DecodedOp* op) {
    dynamic_arith_value_lock(JitWidth::b8, op, op->imm, FLAGS_XOR8, [this](RegPtr src, RegPtr dst, RegPtr address) {
        compiler.ldeoralb(R32(src), R32(dst), Mem(R64(address)));
    }, [this](RegPtr result, RegPtr dstMem, U32 src) {
        if (asmjit::a64::Utils::is_logical_imm(src, 32)) {
            compiler.eor(R32(result), R32(dstMem), src);
        } else {
            compiler.eor(R32(result), R32(dstMem), R32(loadConst(src)));
        }
    });
}

void JitArmV8CodeGen::dynamic_inc32_mem32_lock(DecodedOp* op) {
    dynamic_arith_value_lock(JitWidth::b32, op, 1, FLAGS_INC32, [this](RegPtr src, RegPtr dst, RegPtr address) {
        compiler.ldaddal(R32(src), R32(dst), Mem(R64(address)));
    }, [this](RegPtr result, RegPtr dstMem, U32 src) {
        compiler.add(R32(result), R32(dstMem), src);
    });
}
void JitArmV8CodeGen::dynamic_inc16_mem16_lock(DecodedOp* op) {
    dynamic_arith_value_lock(JitWidth::b16, op, 1, FLAGS_INC16, [this](RegPtr src, RegPtr dst, RegPtr address) {
        compiler.ldaddalh(R32(src), R32(dst), Mem(R64(address)));
    }, [this](RegPtr result, RegPtr dstMem, U32 src) {
        compiler.add(R32(result), R32(dstMem), src);
    });
}
void JitArmV8CodeGen::dynamic_inc8_mem8_lock(DecodedOp* op) {
    dynamic_arith_value_lock(JitWidth::b8, op, 1, FLAGS_INC8, [this](RegPtr src, RegPtr dst, RegPtr address) {
        compiler.ldaddalb(R32(src), R32(dst), Mem(R64(address)));
    }, [this](RegPtr result, RegPtr dstMem, U32 src) {
        compiler.add(R32(result), R32(dstMem), src);
    });
}
void JitArmV8CodeGen::dynamic_dec32_mem32_lock(DecodedOp* op) {
    dynamic_arith_value_lock(JitWidth::b32, op, -1, FLAGS_DEC32, [this](RegPtr src, RegPtr dst, RegPtr address) {
        compiler.ldaddal(R32(src), R32(dst), Mem(R64(address)));
    }, [this](RegPtr result, RegPtr dstMem, U32 src) {
        compiler.sub(R32(result), R32(dstMem), 1);
    });
}
void JitArmV8CodeGen::dynamic_dec16_mem16_lock(DecodedOp* op) {
    dynamic_arith_value_lock(JitWidth::b16, op, -1, FLAGS_DEC16, [this](RegPtr src, RegPtr dst, RegPtr address) {
        compiler.ldaddalh(R32(src), R32(dst), Mem(R64(address)));
    }, [this](RegPtr result, RegPtr dstMem, U32 src) {
        compiler.sub(R32(result), R32(dstMem), 1);
    });
}
void JitArmV8CodeGen::dynamic_dec8_mem8_lock(DecodedOp* op) {
    dynamic_arith_value_lock(JitWidth::b8, op, -1, FLAGS_DEC8, [this](RegPtr src, RegPtr dst, RegPtr address) {
    compiler.ldaddalb(R32(src), R32(dst), Mem(R64(address)));
    }, [this](RegPtr result, RegPtr dstMem, U32 src) {
        compiler.sub(R32(result), R32(dstMem), 1);
    });
}

void JitArmV8CodeGen::dynamic_note32_lock(DecodedOp* op) {
    dynamic_arith_lock(JitWidth::b32, op, FLAGS_NULL, nullptr, [this](RegPtr result, RegPtr dstMem, RegPtr srcReg) {
        compiler.mvn_(R32(result), R32(dstMem));
    });
}

void JitArmV8CodeGen::dynamic_note16_lock(DecodedOp* op) {
    dynamic_arith_lock(JitWidth::b16, op, FLAGS_NULL, nullptr, [this](RegPtr result, RegPtr dstMem, RegPtr srcReg) {
        compiler.mvn_(R32(result), R32(dstMem));
    });
}

void JitArmV8CodeGen::dynamic_note8_lock(DecodedOp* op) {
    dynamic_arith_lock(JitWidth::b8, op, FLAGS_NULL, nullptr, [this](RegPtr result, RegPtr dstMem, RegPtr srcReg) {
        compiler.mvn_(R32(result), R32(dstMem));
    });
}

void JitArmV8CodeGen::dynamic_nege32_lock(DecodedOp* op) {
    dynamic_arith_lock(JitWidth::b32, op, FLAGS_NEG32, nullptr, [this](RegPtr result, RegPtr dstMem, RegPtr srcReg) {
        compiler.neg(R32(result), R32(dstMem));
    }, false, false, false, false);
}

void JitArmV8CodeGen::dynamic_nege16_lock(DecodedOp* op) {
    dynamic_arith_lock(JitWidth::b16, op, FLAGS_NEG16, nullptr, [this](RegPtr result, RegPtr dstMem, RegPtr srcReg) {
        compiler.sxth(R32(result), R32(dstMem));
        compiler.neg(R32(result), R32(result));
    }, false, false, false, false);
}

void JitArmV8CodeGen::dynamic_nege8_lock(DecodedOp* op) {
    dynamic_arith_lock(JitWidth::b8, op, FLAGS_NEG8, nullptr, [this](RegPtr result, RegPtr dstMem, RegPtr srcReg) {
        compiler.sxtb(R32(result), R32(dstMem));
        compiler.neg(R32(result), R32(result));
    }, false, false, false, false);
}
/*
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
        this->x86.bts(X86Asm::Mem32(RN(address->hardwareReg()), RN(offset->hardwareReg())), R32(reg));
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
        this->x86.btr(X86Asm::Mem32(RN(address->hardwareReg()), RN(offset->hardwareReg())), R32(reg));
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
        this->x86.btc(X86Asm::Mem32(RN(address->hardwareReg()), RN(offset->hardwareReg())), R32(reg));
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

void JitArmV8CodeGen::direct_cmp(JitWidth width, RegPtr left, RegPtr right) {
    cfInverted = true;
    cmp(width, left, right);
}

void JitArmV8CodeGen::direct_cmp(JitWidth width, RegPtr left, U32 right) {
    cfInverted = true;
    cmp(width, left, right);
}

void JitArmV8CodeGen::direct_test(JitWidth width, RegPtr left, RegPtr right) {
    cfInverted = false;

    // The OF and CF flags are set to 0. The SF, ZF, and PF flags are set according to the result(see the “Operation” section above).The state of the AF flag is undefined.
    if (width == JitWidth::b32) {        
        // sets Z if the result is zero and N if the highest bit is set, while C and V are cleared.
        compiler.ands(asmjit::a64::wzr, R32(left), R32(right));
    } else if (width == JitWidth::b16) {
        compiler.lsl(wBranch, R32(left), 16);
        compiler.lsl(wScratch, R32(right), 16);
        compiler.ands(asmjit::a64::wzr, wBranch, wScratch);
    } else if (width == JitWidth::b8) {
        if (left->isHigh) {
            compiler.lsr(wBranch, R32(left), 8);
            compiler.lsl(wBranch, wBranch, 24);
        } else {
            compiler.lsl(wBranch, R32(left), 24);
        }
        if (right->isHigh) {
            compiler.lsr(wScratch, R32(right), 8);
            compiler.lsl(wScratch, wScratch, 24);
        } else {
            compiler.lsl(wScratch, R32(right), 24);
        }
        compiler.ands(asmjit::a64::wzr, wBranch, wScratch);
    } else {
        kpanic("JitX86CodeGen::direct_test");
    }
}

void JitArmV8CodeGen::direct_test(JitWidth width, RegPtr left, U32 right) {
    U32 rightShifted = right;

    cfInverted = false;
    if (width == JitWidth::b16) {
        rightShifted <<= 16;
    } else if (width == JitWidth::b8) {
        rightShifted <<= 24;
    }

    if (!asmjit::a64::Utils::is_logical_imm(rightShifted, 32)) {
        direct_test(width, left, loadConst(right));
        return;
    }
    // x86: The OF and CF flags are set to 0. The SF, ZF, and PF flags are set according to the result.The state of the AF flag is undefined.
    if (width == JitWidth::b32) {
        // armv8: sets Z if the result is zero and N if the highest bit is set, while C and V are cleared.
        compiler.ands(asmjit::a64::wzr, R32(left), right);
    } else if (width == JitWidth::b16) {
        if (right & 0x8000) {
            compiler.lsl(wBranch, R32(left), 16);
            compiler.ands(asmjit::a64::wzr, wBranch, rightShifted);
        } else {
            compiler.ands(asmjit::a64::wzr, R32(left), right & 0xffff);
        }
    } else if (width == JitWidth::b8) {
        if (right & 0x80) {
            if (left->isHigh) {
                compiler.lsr(wBranch, R32(left), 8);
                compiler.lsl(wBranch, wBranch, 24);
            } else {
                compiler.lsl(wBranch, R32(left), 24);
            }
            compiler.ands(asmjit::a64::wzr, wBranch, rightShifted);
        } else {
            if (left->isHigh) {
                compiler.ands(asmjit::a64::wzr, R32(left), (right & 0xff) << 8);
            } else {
                compiler.ands(asmjit::a64::wzr, R32(left), right & 0xff);
            }
        }
    } else {
        kpanic("JitX86CodeGen::direct_test");
    }
}

void JitArmV8CodeGen::onBlockPreCommit(DecodedOp* op) {
    if (pendingLabels.size()) {
        kpanic("JitX86CodeGen::onBlockPreCommit");
    }
}

bool JitArmV8CodeGen::supportsDirectCondition(JitConditional condition) {
    if (condition == JitConditional::P || condition == JitConditional::NP) {
        return false;
    }
    return true;
}

asmjit::a64::CondCode JitArmV8CodeGen::getCondCode(JitConditional condition) {
    switch (condition) {
    case JitConditional::O: return asmjit::a64::CondCode::kVS;
    case JitConditional::NO: return asmjit::a64::CondCode::kVC;
    case JitConditional::B: return cfInverted ? asmjit::a64::CondCode::kLO : asmjit::a64::CondCode::kHS;
    case JitConditional::NB: return cfInverted ? asmjit::a64::CondCode::kHS : asmjit::a64::CondCode::kLO;
    case JitConditional::Z: return asmjit::a64::CondCode::kEQ;
    case JitConditional::NZ: return asmjit::a64::CondCode::kNE;
    case JitConditional::BE: 
        if (!cfInverted) {
            compiler.cfinv();
        }
        return asmjit::a64::CondCode::kLS;
    case JitConditional::NBE: 
        if (!cfInverted) {
            compiler.cfinv();
        }
        return asmjit::a64::CondCode::kHI;
    case JitConditional::S: return asmjit::a64::CondCode::kMI;
    case JitConditional::NS: return asmjit::a64::CondCode::kPL;
    //case JitConditional::P: return asmjit::a64::CondCode::kP;
    //case JitConditional::NP: return asmjit::a64::CondCode::kNP;
    case JitConditional::L: return asmjit::a64::CondCode::kLT;
    case JitConditional::NL: return asmjit::a64::CondCode::kGE;
    case JitConditional::LE: return asmjit::a64::CondCode::kLE;
    case JitConditional::NLE: return asmjit::a64::CondCode::kGT;
    default:
        kpanic("JitArmV8CodeGen::getCondCode");
        return asmjit::a64::CondCode::kVS;
    }
}

void JitArmV8CodeGen::direct_jump(JitConditional condition, U32 address) {
    Label label;

    if (!opLabels.get(address, label)) {
        label = compiler.new_label();
        opLabels.set(address, label);
        pendingLabels.set(address, label);
    }

    compiler.b(getCondCode(condition), label);
}

void JitArmV8CodeGen::direct_cmov(JitWidth width, JitConditional condition, RegPtr dst, RegPtr src) {
    if (width == JitWidth::b32) {
        compiler.csel(R32(dst), R32(src), R32(dst), getCondCode(condition));
    } else if (width == JitWidth::b16) {
        RegPtr tmpReg = getTmpReg();

        compiler.csel(R32(tmpReg), R32(src), R32(dst), getCondCode(condition));
        mov(JitWidth::b16, dst, tmpReg);
    }
}

void JitArmV8CodeGen::direct_setcc(JitConditional condition, RegPtr dst) {
    RegPtr result = getTmpReg();
    compiler.mov(R32(result->hardwareReg()), 1);
    compiler.csel(R32(result), R32(result), asmjit::a64::wzr, getCondCode(condition));
    compiler.bfi(R32(dst), R32(result), dst->isHigh ? 8 : 0, 8);
}

bool JitArmV8CodeGen::directDoesAffectFlags(DecodedOp* op) {
    // memory instructions call clearMMUPermissionIfSpansPage which calls cmp, 8-bit read/write don't call it
    switch (op->inst) {
    case MovR8R8:
    //case MovE8R8:
    case MovR8E8:
    case MovR8I8:
    //case MovE8I8:
    case MovR16R16:
    //case MovE16R16:
    //case MovR16E16:
    case MovR16I16:
    //case MovE16I16:
    case MovR32R32:
    //case MovE32R32:
    //case MovR32E32:
    case MovR32I32:
    //case MovE32I32:
    case MovAlOb:
    //case MovAxOw:
    //case MovEaxOd:
    case MovObAl:
    //case MovOwAx:
    //case MovOdEax:
    case MovGwXzR8:
    //case MovGwXzE8:
    case MovGwSxR8:
    //case MovGwSxE8:
    case MovGdXzR8:
    //case MovGdXzE8:
    case MovGdSxR8:
    //case MovGdSxE8:
    case MovGdXzR16:
    //case MovGdXzE16:
    case MovGdSxR16:
    //case MovGdSxE16:
    case LeaR16:
    case LeaR32:
        return false;
    default:
        return true;
    }
}

void writeBlockExitForJIT(U32 eip, U8* buffer) {
    asmjit::JitRuntime rt;
    asmjit::CodeHolder code;
    asmjit::a64::Assembler compiler;

    code.init(rt.environment());
    code.attach(&compiler);

    compiler.blr(xWriteCacheToCPU);
    compiler.mov(wTmp7, eip);
    compiler.str(wTmp7, Mem(xCPU, offsetof(CPU, eip.u32)));
    compiler.mov(wTmp7, xCPU);
    for (int i = 19; i < 31; i+=2) {
        compiler.ldp(R64(i), R64(i + 1), Mem(xTmp7, offsetof(CPU, storedRegs) + (i - 19) * 8));
    }
    compiler.add(xTmp7, xCPU, offsetof(CPU, xmm[0]));
    compiler.st1(toSseD2(0), toSseD2(1), toSseD2(2), toSseD2(3), Mem(xTmp7, 64).post());
    compiler.st1(toSseD2(4), toSseD2(5), toSseD2(6), toSseD2(7), Mem(xTmp7));
    compiler.ret(asmjit::a64::x30);

    code.flatten();
    Platform::writeCodeToMemory(buffer, (U32)code.code_size(), [&code, buffer]() {
        code.copy_flattened_data(buffer, code.code_size());
    });
    Platform::clearInstructionCache(buffer, (U32)code.code_size());
    
}

U8* JitArmV8CodeGen::createBlockExit(bool syncRegs) {
    if (syncRegs) {
        compiler.blr(xWriteCacheToCPU);
    }
    compiler.mov(xTmp7, xCPU);
    for (int i = 19; i < 31; i+=2) {
        compiler.ldp(R64(i), R64(i + 1), Mem(xTmp7, offsetof(CPU, storedRegs) + (i - 19) * 8));
    }
    compiler.ret(asmjit::a64::x30);
    return createDynamicExecutableMemory();
}

void JitArmV8CodeGen::blockExit(bool syncCache) {
    if (syncCache) {
        compiler.blr(xWriteCacheToCPU);
    }
    compiler.mov(xTmp7, xCPU);
    for (int i = 19; i < 31; i += 2) {
        compiler.ldp(R64(i), R64(i + 1), Mem(xTmp7, offsetof(CPU, storedRegs) + (i - 19) * 8));
    }
    compiler.ret(asmjit::a64::x30);
}

U8* JitArmV8CodeGen::createStartJITCode() {
    for (int i = 19; i < 31; i+=2) {
        compiler.stp(R64(i), R64(i+1), Mem(xEAX, offsetof(CPU, storedRegs) + (i - 19) * 8));
    }
    compiler.mov(R64(29), asmjit::a64::sp); // mov fp, sp
    compiler.mov(xCPU, R64(0));
    compiler.ldr(xBranch, Mem(R64(1), offsetof(DecodedOp, pfnJitCode)));
    compiler.mov(xMMU, (U64)&getMemData(KThread::currentThread()->memory)->mmu);
#ifdef BOXEDWINE_MEM_CACHE
    static_assert(offsetof(KMemoryData, mmu) + sizeof(void*) * K_NUMBER_OF_PAGES == offsetof(KMemoryData, readCache));
    static_assert(offsetof(KMemoryData, mmu) + sizeof(void*) * K_NUMBER_OF_PAGES * 2 == offsetof(KMemoryData, writeCache));
#endif
    compiler.mov(xLoadCacheFromCPU, (U64)KThread::currentThread()->process->syncToHost);
    compiler.mov(xWriteCacheToCPU, (U64)KThread::currentThread()->process->syncFromHost);
    compiler.mov(xEmulateSingleOp, (DYN_PTR_SIZE)cpu->thread->process->emulateSingleOp);
    loadCacheFromCPU();
    compiler.br(xBranch);

    return createDynamicExecutableMemory();
}

void JitArmV8CodeGen::loadCacheFromCPU() {
    for (int i = 0; i < 8; i+=2) {
        if (regCache[i] != INVALID_REG) {
            if (regCache[i + 1] == INVALID_REG) {
                kpanic("JitArmV8CodeGen::loadCacheFromCPU");
            }
            compiler.ldp(R32(regCache[i]), R32(regCache[i + 1]), Mem(xCPU, (U32)(offsetof(CPU, reg[0].u32) + sizeof(U32) * i)));
        }
    }
    static_assert(offsetof(CPU, flags) + 4 == offsetof(CPU, src));
    compiler.ldp(xFLAGS, xSrc, createMem(regCPU, offsetof(CPU, flags)));
    
    static_assert(offsetof(CPU, dst) + 4 == offsetof(CPU, result));
    compiler.ldp(xDst, xResult, createMem(regCPU, offsetof(CPU, dst)));
    
    compiler.orr(xFLAGS, xFLAGS, 2);
    compiler.ldrb(xFlagsType, createMem(regCPU, offsetof(CPU, lazyFlagType)));

    compiler.add(xTmp9, xCPU, offsetof(CPU, xmm[0]));
    compiler.ld1(toSseD2(0), toSseD2(1), toSseD2(2), toSseD2(3), Mem(xTmp9, 64).post());
    compiler.ld1(toSseD2(4), toSseD2(5), toSseD2(6), toSseD2(7), Mem(xTmp9));
}

void JitArmV8CodeGen::writeCacheToCPU() {
    for (int i = 0; i < 8; i+=2) {
        if (regCache[i] != INVALID_REG) {
            if (regCache[i + 1] == INVALID_REG) {
                kpanic("JitArmV8CodeGen::writeCacheToCPU");
            }
            compiler.stp(R32(regCache[i]), R32(regCache[i+1]), Mem(xCPU, (U32)(offsetof(CPU, reg[0].u32) + sizeof(U32) * i)));
        }
    }
    static_assert(offsetof(CPU, flags) + 4 == offsetof(CPU, src));
    compiler.stp(xFLAGS, xSrc, createMem(regCPU, offsetof(CPU, flags)));

    static_assert(offsetof(CPU, dst) + 4 == offsetof(CPU, result));
    compiler.stp(xDst, xResult, createMem(regCPU, offsetof(CPU, dst)));
    compiler.strb(xFlagsType, createMem(regCPU, offsetof(CPU, lazyFlagType)));

    compiler.add(xTmp9, xCPU, offsetof(CPU, xmm[0]));
    compiler.st1(toSseD2(0), toSseD2(1), toSseD2(2), toSseD2(3), Mem(xTmp9, 64).post());
    compiler.st1(toSseD2(4), toSseD2(5), toSseD2(6), toSseD2(7), Mem(xTmp9));
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
