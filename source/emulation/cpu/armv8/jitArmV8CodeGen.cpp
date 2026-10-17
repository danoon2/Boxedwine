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
#include "../common/common_fpu.h"
#include "../../softmmu/kmemory_soft.h"
#include <array>
#include <cmath>
#include <set>

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
static thread_local bool forceInlinePoll = false;
#ifdef __TEST
static thread_local U32 largePollFallbacks = 0;
U32 armJitLargePollFallbackCount() { return largePollFallbacks; }
static thread_local U32 cachedMathRegions = 0;
U32 armJitCachedMathRegionCount() { return cachedMathRegions; }
static thread_local U32 cachedFpuExamines = 0;
U32 armJitCachedFpuExamineCount() { return cachedFpuExamines; }
static thread_local U32 elidedVectorCopies = 0;
U32 armJitElidedVectorCopyCount() { return elidedVectorCopies; }
#endif
static std::once_flag s_tsoModeInitFlag;
#ifdef __linux__
static thread_local bool s_hardwareTsoChecked = false;
#endif

#define NUMBER_OF_REGS 31
#define NUMBER_OF_VREGS 32
#define NUMBER_OF_TMPS 9
#define NUMBER_OF_VREG_TMPS 9

/*
static bool isVolitile[] = { true,  true,  true,  true,  true,  true,  true,  true,
                             true,  true,  true,  true,  true,  true,  true,  true,
                             true,  true,  true,  false, false, false, false, false,
                             false, false, false, false, false, false, false, false };
*/
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
// Linear-memory code keeps its invariant RAM base in x26. Other memory modes
// retain the emulation helper here.
#define xEmulateSingleOp asmjit::a64::x26
#define xLinearMemoryBase asmjit::a64::x26
#define xWriteCacheToCPU asmjit::a64::x27
#define xLoadCacheFromCPU asmjit::a64::x28
#define xFlatJitBase asmjit::a64::x28
#define xScratch asmjit::a64::x29
#define wScratch asmjit::a64::w29
#define regScratch 29

#define ZERO_EXTEND 1
#define SIGN_EXTEND 2

using MakeSSE = Vec(SSERegPtr reg);

bool enableHardwareTSO();

static bool cacheFlatJitBase(CPU* cpu) {
    // Reserving x28 speeds some dispatch loops, but enlarges the helper
    // reload paths and regressed several narrow arithmetic kernels.
    static const bool enabled = std::getenv("BOXEDWINE_ARM_FLAT_BASE") &&
        !std::getenv("BOXEDWINE_ARM_FLAT_BASE_SLOW");
    return enabled && getMemData(cpu->memory)->opCache.getFlatJitEntries();
}

static bool cacheLinearMemoryBase() {
    static const bool enabled = !std::getenv("BOXEDWINE_ARM_LINEAR_BASE_SLOW");
    return ramPageUseLinearMemory() && enabled;
}

// Process-launch controls retain the previous lowering for diagnosis.
static bool armFpuHelpersEnabled() {
    static const bool enabled = !std::getenv("BOXEDWINE_ARM_FPU_HELPERS_SLOW");
    return enabled;
}

static bool armCachedFpuMathEnabled() {
    static const bool enabled = !std::getenv("BOXEDWINE_ARM_CACHED_FPU_MATH_SLOW");
    return enabled && armFpuHelpersEnabled();
}

static bool armCachedFpuExamineEnabled() {
    static const bool enabled = !std::getenv("BOXEDWINE_ARM_CACHED_FPU_EXAMINE_SLOW");
    return enabled && armFpuHelpersEnabled();
}

static bool armGpScalarConversionEnabled() {
    static const bool enabled = !std::getenv("BOXEDWINE_ARM_SSE_GP_CONVERT_SLOW");
    return enabled;
}

class JitArmV8CodeGen : public JitSSE, asmjit::ErrorHandler {
public:  
    ~JitArmV8CodeGen() override { fpuStackCache = FpuStackCache{}; }
    void handle_error(asmjit::Error err, const char* message, asmjit::BaseEmitter* origin) override {
        kpanic(message);
    }

    JitArmV8CodeGen(CPU* cpu) : JitSSE(cpu) {
        ensureArmV8HardwareTSOForThread();
        code.init(rt.environment());
        code.attach(&compiler);
        code.set_error_handler(this);
    }

    static void initTSOMode();

    // Start with forward guards; backward exits retain their polling path.
    bool supportsConditionalSideExit(U32 sourceEip, U32 targetEip) const override { return targetEip > sourceEip; }
    void preOp(DecodedOp* op) override;
    void preCompile(DecodedOp* op, bool skippedOp = false) override;
    void postCompile(DecodedOp* op) override;
    void compile(DecodedOp* op) override;
    void jumpToEipIfCached(RegPtr eipReg) override;
    void callLoadCacheFromCPU();
    void callRuntimeHelper(void* address);
    void exitToRunLoopIfNeeded(U32 eip) override;
    void exitToRunLoopIfNeeded(RegPtr eip) override;
    bool useColdPollExit() const;
    void emitColdPollExit(const std::function<void()>& writeRestart);
    bool compileOps(DecodedOp* op) override;
    asmjit::Section* pollExitSection = nullptr;
    bool compileNarrowIncDec(DecodedOp* op);
    bool compileCarryChain(DecodedOp* op);
    bool compileNarrowArithmetic(DecodedOp* op);
    bool compileShiftFlags(DecodedOp* op);
    bool compileIntegerParity(DecodedOp* op);
    bool compileNarrowParity(DecodedOp* op);
    void emitIntegerParity(RegPtr result);
    bool compileMultiplyFlags(DecodedOp* op);
    bool compileConditionalCompare(DecodedOp* op);
    bool compileScalarSseRegion(DecodedOp* op);
    bool compileCachedMathRegion(DecodedOp* op);
    bool compileCachedFpuExamine(DecodedOp* op);
    bool compilePureFpuHelper(DecodedOp* op);
    bool isExternalJitEntry(U32 eip) const override {
        return JitCodeGen::isExternalJitEntry(eip) && !fpuInternalEntries.count(eip) && !scalarInternalEntries.count(eip) && !flagInternalEntries.count(eip);
    }
    bool protectKnownLazyFlagType() override;
    bool knownLazyTypeEnabled = std::getenv("BOXEDWINE_ARM_KNOWN_LAZY_TYPE_SLOW") == nullptr;
    std::set<U32> flagInternalEntries;
    bool canTrackScalarZero(DecodedOp* op) const;
    void updateScalarZero(DecodedOp* op);
    bool hasZeroUpper64(SSERegPtr reg) const;
    bool scalarZeroEnabled = std::getenv("BOXEDWINE_ARM_SCALAR_ZERO_SLOW") == nullptr;
    bool scalarZeroCopyEnabled = std::getenv("BOXEDWINE_ARM_SCALAR_ZERO_COPY_SLOW") == nullptr;
    bool scalarZeroLanesEnabled = std::getenv("BOXEDWINE_ARM_SCALAR_ZERO_LANES_SLOW") == nullptr;
    bool scalarZeroDirectEnabled = std::getenv("BOXEDWINE_ARM_SCALAR_ZERO_DIRECT_SLOW") == nullptr;
    U8 scalarZeroMask = 0;
    U8 scalarZeroLane1Mask = 0;
    std::set<U32> scalarInternalEntries;
    bool supportsFpuStackCache() const override {
        static const bool enabled = !std::getenv("BOXEDWINE_DISABLE_NATIVE_FPU_CACHE");
        return enabled;
    }
    bool canCacheFpuOp(DecodedOp* op) const override;
    RegPtr getFpuCacheTopReg() override;
    RegPtr getFpuCacheTagReg(U8 index) override;
    FPURegPtr getFpuCacheReg(U8 index) override;
    void moveFpuReg(FPURegPtr dst, FPURegPtr src) override;
    bool loadFpuValue(FPURegPtr dst, RegPtr index, RegPtr valid, bool nearest = true) override;
    void materializeFpuCache(U8 slotMask = 0xff) override;
    void nativeFpuIndex(U8 index);
    void spillFpuTag(U8 index);
    void preserveFpuCacheTop(RegPtr top) override;
    void spillFpuTop();
    bool spillFpuTmp();
    U8 allocateFpuTag(JitReg* tag, bool load = true);
    U8 getFpuWriteReg(RegPtr tag);
    std::pair<U32, U32> saveFpuTemporaries(U8 result = INVALID_REG);
    void restoreFpuTemporaries(std::pair<U32, U32> saved);
    std::set<U32> fpuInternalEntries;
    std::vector<FpuRegisterLoop> fpuRegisterLoops;
    const FpuRegisterLoop* fpuRegisterLoop = nullptr;
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
    U32 getAvailableTmpRegCount() override;
    bool isTmpRegAvailable() override;    
    void forceSyncBackIfNotCached(RegPtr reg) override;
    RegPtr getConditionCalculationReg(U32 index) override;

    U8 findTmpReg(bool allowInvalidReturn = false);
    void emulateSingleOp() override;
    RegPtr calculateEaa(DecodedOp* op, U32 popEspAmount = 0) override;

    void movFromMemory(DecodedOp* op, JitWidth dstWidth, JitWidth srcWidth, bool signExtend = false);
    void dynamic_movr32e32(DecodedOp* op) override { movFromMemory(op, JitWidth::b32, JitWidth::b32); }
    void dynamic_movGwXzE8(DecodedOp* op) override { movFromMemory(op, JitWidth::b16, JitWidth::b8); }
    void dynamic_movGwSxE8(DecodedOp* op) override { movFromMemory(op, JitWidth::b16, JitWidth::b8, true); }
    void dynamic_movGdXzE8(DecodedOp* op) override { movFromMemory(op, JitWidth::b32, JitWidth::b8); }
    void dynamic_movGdSxE8(DecodedOp* op) override { movFromMemory(op, JitWidth::b32, JitWidth::b8, true); }
    void dynamic_movGdXzE16(DecodedOp* op) override { movFromMemory(op, JitWidth::b32, JitWidth::b16); }
    void dynamic_movGdSxE16(DecodedOp* op) override { movFromMemory(op, JitWidth::b32, JitWidth::b16, true); }

    void direct_cmp(JitWidth width, RegPtr left, RegPtr right) override;
    void direct_cmp(JitWidth width, RegPtr left, U32 right) override;
    void direct_test(JitWidth width, RegPtr left, RegPtr right) override;
    void direct_test(JitWidth width, RegPtr left, U32 right) override;
    void direct_flags_op(JitWidth width, JitFlagOp op, RegPtr dst, RegPtr src) override;
    void direct_flags_op(JitWidth width, JitFlagOp op, RegPtr dst, U32 src) override;
    void direct_flags_op_with_cf(JitWidth width, JitCarryOp op, RegPtr dst, RegPtr src, RegPtr cf) override;
    void direct_flags_op_with_cf(JitWidth width, JitCarryOp op, RegPtr dst, U32 src, RegPtr cf) override;
    void direct_neg(JitWidth width, RegPtr dst) override;
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
    void andValueWithDest(JitWidth regWidth, RegPtr dst, RegPtr reg, U32 value) override;
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

    void readMMU(RegPtr dest, RegPtr index, U32 offset) override;
    void readMMU(RegPtr dest, U32 index) override;
    RegPtr getLinearMemoryBase(RegPtr tmp = nullptr) override;
    virtual void readHost(JitWidth width, MemPtr address, RegPtr result, bool emlulatedMemory = true) override;
    void readHostValue(JitWidth width, MemPtr address, RegPtr result, bool emulatedMemory, bool signExtend);
    virtual void writeHost(JitWidth width, MemPtr address, RegPtr src, bool emlulatedMemory = true) override;
    virtual void writeHost(JitWidth width, MemPtr address, U32 imm, bool emlulatedMemory = true) override;

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
    void IfLessThan(JitWidth regWidth, ComparisonType type, RegPtr reg, U32 value) override;
    void IfLessThan(JitWidth regWidth, ComparisonType type, RegPtr reg1, RegPtr reg2) override;
    void IfGreaterThanOrEqual(JitWidth regWidth, ComparisonType type, RegPtr reg1, RegPtr reg2) override;
    void IfGreaterThanOrEqual(JitWidth regWidth, ComparisonType type, RegPtr reg1, U32) override;
    void IfGreaterThan(JitWidth regWidth, ComparisonType type, RegPtr reg1, RegPtr reg2) override;
    void IfGreaterThan(JitWidth regWidth, ComparisonType type, RegPtr reg1, U32 value) override;
    void IfNot(JitWidth regWidth, RegPtr reg) override;
    void IfNotCPU(JitWidth regWidth, RegPtr sib, U8 lsl, U32 offset) override;    
    void JumpIfCondition(JitConditional condition, U32 address) override;
    void clearMMUPermissionIfSpansPage(JitWidth width, RegPtr offset, RegPtr reg) override;
    void clearIfSpansPage(JitWidth width, RegPtr offset, RegPtr reg) override;

    U32 MarkJumpLocation() override;
    void Goto(U32 location) override;
    void jmpHost(RegPtr reg) override;
    void jmpHost(DYN_PTR_SIZE address) override;
    RegPtr getReadOnlyFlags(RegPtr tmp = nullptr) override;
    U32 ramAccessAlignment(JitWidth width) override;
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
    void blockExit() override;

    // FPU
    RegPtr fpuRegToInt(FPURegPtr fpuRegSrc, bool truncate, bool is64);

    FPURegPtr getFPUTmp() override;
    void storeCpuFpuReg(FPURegPtr reg, RegPtr index) override;
    void loadCpuFpuReg(FPURegPtr reg, RegPtr index) override;
    void loadCpuFpuRegConst(FPURegPtr reg, U32 offset) override;
    void loadFpuRegFromInt(FPURegPtr reg, MemPtr address) override;
    void storeFpuReg(FPURegPtr reg, MemPtr address, DynFpuWidth width = DYN_FPU_64_BIT) override;
    RegPtr fpuRegToInt32(FPURegPtr fpuRegSrc, bool truncate) override;
    void loadFpuReg(FPURegPtr reg, MemPtr address, DynFpuWidth width = DYN_FPU_64_BIT) override;
    void fpuRegExtend32To64(FPURegPtr dst, FPURegPtr src) override;
    void fpuReg64To32(FPURegPtr dst, FPURegPtr src) override;
    void regToFpuReg(FPURegPtr dst, RegPtr src) override;
#ifdef BOXEDWINE_64
    void regToFpuReg64(FPURegPtr dst, RegPtr src) override;
#endif
    void updateFPURounding() override;
    void restoreFPURounding() override;
    void roundFPUToInt64(FPURegPtr src) override;
    void storeFPUToInt64(FPURegPtr src, MemPtr address, bool truncate) override;

    void fpuAdd(FPURegPtr dst, FPURegPtr src) override;
    void fpuMul(FPURegPtr dst, FPURegPtr src) override;
    void fpuSub(FPURegPtr dst, FPURegPtr src) override;
    void fpuDiv(FPURegPtr dst, FPURegPtr src) override;
    void fpuXor(FPURegPtr dst, FPURegPtr src) override;
    void fpuAnd(FPURegPtr dst, FPURegPtr src) override;
    void fpuAbs(FPURegPtr dst) override;
    void fpuNeg(FPURegPtr dst) override;
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
    MMXRegPtr loadMMXFromMem32(U8 index, MemPtr address) override;
    MMXRegPtr loadMMXFromMem64(U8 index, MemPtr address) override;
    void storeMMXToMem32(MMXRegPtr reg, MemPtr address) override;
    void storeMMXToMem64(MMXRegPtr reg, MemPtr address) override;
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
    void maskmovq(MMXRegPtr src, MMXRegPtr mask, MemPtr destAddress) override;

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
    SSERegPtr loadXMMFromMem128(U8 reg, MemPtr address, SSERegPtr result = nullptr) override;
    SSERegPtr loadXMMFromMem64(U8 reg, MemPtr address) override;
    SSERegPtr loadLowXMMFromMem64(U8 reg, MemPtr address) override;
    SSERegPtr loadHighXMMFromMem64(U8 reg, MemPtr address) override;
    SSERegPtr loadXMMFromMem32(U8 reg, MemPtr address) override;
    void storeXMMToMem128(SSERegPtr reg, MemPtr address) override;
    void storeXMMToMem64(SSERegPtr reg, MemPtr address) override;
    void storeXMMToMem32(SSERegPtr reg, MemPtr address) override;
    void storeHighXMMToMem64(SSERegPtr reg, MemPtr address) override;
    void fmax(SSERegPtr dst, SSERegPtr src1, SSERegPtr src2, MakeSSE vMake);
    void fmin(SSERegPtr dst, SSERegPtr src1, SSERegPtr src2, MakeSSE vMake);
    void cvtps2pi(SSERegPtr dst, SSERegPtr src, bool truncate);
    void cvtsd2si(RegPtr dst, SSERegPtr src, bool truncate);
    void cvtpd2pi(SSERegPtr dst, SSERegPtr src, bool truncate);
    void sseConvertFloatToInt(Vec dst, Vec src, bool truncate);
    void sseConvertFloatToInt(RegPtr dst, Vec src, bool truncate);
    void truncateScalarToInt32(RegPtr dst, SSERegPtr src, bool doublePrecision);
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
    bool shuffleDwords(SSERegPtr dst, SSERegPtr src, U32 mask);
    void shuffleWords(SSERegPtr dst, SSERegPtr src, U32 mask, bool high);
    void shufpsXmmXmm(SSERegPtr dst, SSERegPtr src, U32 imm) override;
    void unpckhpsXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void unpcklpsXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void cmppsXmmXmm(SSERegPtr dst, SSERegPtr src, U32 imm) override;
    void cmpssXmmXmm(SSERegPtr dst, SSERegPtr src, U32 imm) override;
    void comissXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void ucomissXmmXmm(SSERegPtr dst, SSERegPtr src) override;
    void sfence() override;
    void stmxcsr(MemPtr address) override;
    void ldmxcsr(MemPtr address) override;

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

    void maskmovdqu(SSERegPtr dst, SSERegPtr src, MemPtr address) override;
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
    void clflush(MemPtr address) override;
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
    U8* createBlockExit() override;

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
    Mem createMem(MemPtr address);
    Mem createMem1(MemPtr address);
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
    bool directFloatFlags = false;
    bool directIntegerParity = false;
    RegPtr directParityValue; // immutable even-parity bit across a fused region
    bool directMultiplyFlags = false;
};

asmjit::JitRuntime JitArmV8CodeGen::rt;

void JitArmV8CodeGen::initTSOMode() {
    std::call_once(s_tsoModeInitFlag, []() {
#ifdef BOXEDWINE_MSVC
        U64 features = get_ID_AA64ISAR0_EL1();
        U64 atomicLevel = ((int64_t)(features << (60 - 20)) >> 60);
        if (atomicLevel >= 1) {
            JitArmV8CodeGen::rt._cpu_features.add(asmjit::CpuFeatures::ARM::kLSE);
        }

        features = get_ID_AA64ISAR1_EL1();
        atomicLevel = ((int64_t)(features << (60 - 20)) >> 60);
        if (atomicLevel >= 1) {
            JitArmV8CodeGen::rt._cpu_features.add(asmjit::CpuFeatures::ARM::kLRCPC);
        }
        if (atomicLevel >= 2) {
            JitArmV8CodeGen::rt._cpu_features.add(asmjit::CpuFeatures::ARM::kLRCPC2);
        }
        if (atomicLevel >= 3) {
            JitArmV8CodeGen::rt._cpu_features.add(asmjit::CpuFeatures::ARM::kLRCPC3);
        }
#endif
        if (tsoMode == TSOMode::Automatic) {
#ifdef __linux__
            if (enableHardwareTSO()) {
                tsoMode = TSOMode::Hardware;
                s_hardwareTsoChecked = true;
            } else
#endif
            if (JitArmV8CodeGen::rt.cpu_features().has(asmjit::CpuFeatures::ARM::kLRCPC2)) {
                tsoMode = TSOMode::FEAT_LRCPC2;
            } else {
                tsoMode = TSOMode::None;
            }
        }
    });
}

void ensureArmV8HardwareTSOForThread() {
    JitArmV8CodeGen::initTSOMode();
#ifdef __linux__
    if (tsoMode == TSOMode::Hardware && !s_hardwareTsoChecked) {
        s_hardwareTsoChecked = enableHardwareTSO();
    }
#endif
}

bool JitArmV8CodeGen::canCacheFpuOp(DecodedOp* op) const {
    if (supportsFpuStackCache() && !op->lock) {
        if (op->inst == FXAM && armCachedFpuExamineEnabled()) return true;
        if (armCachedFpuMathEnabled()) switch (op->inst) {
        case FSIN: case FCOS: case F2XM1: case FPTAN: case FSINCOS:
        case FYL2X: case FYL2XP1: case FPATAN: return true;
        default: break;
        }
    }
    return canCacheNativeFpuOp(op);
}


bool JitArmV8CodeGen::protectKnownLazyFlagType() {
    return knownLazyTypeEnabled && protectCmpLazyFlagType();
}

bool JitArmV8CodeGen::canTrackScalarZero(DecodedOp* op) const {
    // A small whitelist keeps the proof local. Helpers and instructions that
    // can overwrite packed lanes terminate it until another zeroing producer.
    switch (op->inst) {
    case Cvtps2pdXmmXmm: case Cvtps2pdXmmE64:
        return scalarZeroLanesEnabled;
    case Nop:
    case MovR8R8: case MovR8I8: case MovR8E8: case MovE8R8: case MovE8I8:
    case MovR16R16: case MovR16I16: case MovR16E16: case MovE16R16: case MovE16I16:
    case MovR32R32: case MovR32I32: case MovR32E32: case MovE32R32: case MovE32I32:
    case LeaR16: case LeaR32:
    case MovsdXmmXmm: case MovsdXmmE64: case MovsdE64Xmm:
    case MovqXmmXmm: case MovqXmmE64: case MovqE64Xmm:
    case MovssXmmXmm: case MovssXmmE32: case MovssE32Xmm:
    case MovdXmmR32: case MovdXmmE32: case MovdR32Xmm: case MovdE32Xmm:
    case AddsdXmmXmm: case AddsdXmmE64:
    case SubsdXmmXmm: case SubsdXmmE64:
    case MulsdXmmXmm: case MulsdXmmE64:
    case DivsdXmmXmm: case DivsdXmmE64:
    case SqrtsdXmmXmm: case SqrtsdXmmE64:
        return true;
    case MovapsXmmXmm: case MovupsXmmXmm: case MovapdXmmXmm: case MovupdXmmXmm:
    case MovdqaXmmXmm: case MovdquXmmXmm:
    case Cvtsi2sdXmmR32: case Cvtsi2sdXmmE32: case Cvtss2sdXmmXmm: case Cvtss2sdXmmE32:
    case Cvtsi2ssXmmR32: case Cvtsi2ssXmmE32: case Cvtsd2ssXmmXmm: case Cvtsd2ssXmmE64:
    case Cvtsd2siR32Xmm: case Cvtsd2siR32E64: case Cvttsd2siR32Xmm: case Cvttsd2siR32E64:
    case Cvtss2siR32Xmm: case Cvtss2siR32E32: case Cvttss2siR32Xmm: case Cvttss2siR32E32:
    case ComisdXmmXmm: case ComisdXmmE64: case UcomisdXmmXmm: case UcomisdXmmE64:
    case AddR32E32: case AddE32R32: case AddR32R32: case AddR32I32: case AddE32I32:
    case SubR32E32: case SubE32R32: case SubR32R32: case SubR32I32: case SubE32I32:
    case AndR32E32: case AndE32R32: case AndR32R32: case AndR32I32: case AndE32I32:
    case OrR32E32: case OrE32R32: case OrR32R32: case OrR32I32: case OrE32I32:
    case XorR32E32: case XorE32R32: case XorR32R32: case XorR32I32: case XorE32I32:
    case CmpR32E32: case CmpE32R32: case CmpR32R32: case CmpR32I32: case CmpE32I32:
    case TestE32R32: case TestR32R32: case TestR32I32: case TestE32I32:
        return scalarZeroCopyEnabled;
    default:
        return false;
    }
}

void JitArmV8CodeGen::updateScalarZero(DecodedOp* op) {
    if (!scalarZeroEnabled) return;
    if (scalarZeroLanesEnabled) {
        const U8 dst = op->reg < 8 ? 1u << op->reg : 0;
        const bool sourceLane1Zero = op->rm < 8 && (scalarZeroLane1Mask & (1u << op->rm)) != 0;
        switch (op->inst) {
        case MovapsXmmXmm: case MovupsXmmXmm: case MovapdXmmXmm: case MovupdXmmXmm:
        case MovdqaXmmXmm: case MovdquXmmXmm:
        case MovqXmmXmm: case MovsdXmmXmm:
            scalarZeroLane1Mask &= ~dst;
            if (sourceLane1Zero) scalarZeroLane1Mask |= dst;
            break;
        case MovssXmmE32: case MovdXmmR32: case MovdXmmE32:
            scalarZeroLane1Mask |= dst;
            break;
        case XorpdXmmXmm: case XorpsXmmXmm: case PxorXmmXmm:
            if (op->reg == op->rm) scalarZeroLane1Mask |= dst;
            break;
        case Cvtps2pdXmmXmm: case Cvtps2pdXmmE64:
            // Positive zero in float lane 1 becomes an all-zero high double.
            // Unknown values, including negative zero, cannot use this proof.
            scalarZeroMask &= ~dst;
            if (op->inst == Cvtps2pdXmmXmm && sourceLane1Zero) scalarZeroMask |= dst;
            scalarZeroLane1Mask &= ~dst;
            break;
        case MovsdXmmE64: case MovqXmmE64:
        case AddsdXmmXmm: case AddsdXmmE64:
        case SubsdXmmXmm: case SubsdXmmE64:
        case MulsdXmmXmm: case MulsdXmmE64:
        case DivsdXmmXmm: case DivsdXmmE64:
        case SqrtsdXmmXmm: case SqrtsdXmmE64:
        case Cvtsi2sdXmmR32: case Cvtsi2sdXmmE32:
        case Cvtss2sdXmmXmm: case Cvtss2sdXmmE32:
            scalarZeroLane1Mask &= ~dst;
            break;
        default:
            break;
        }
    }
    switch (op->inst) {
    case MovapsXmmXmm: case MovupsXmmXmm: case MovapdXmmXmm: case MovupdXmmXmm:
    case MovdqaXmmXmm: case MovdquXmmXmm:
        if (scalarZeroCopyEnabled) {
            const bool sourceZero = (scalarZeroMask & (1u << op->rm)) != 0;
            scalarZeroMask &= ~(1u << op->reg);
            if (sourceZero) scalarZeroMask |= 1u << op->reg;
        }
        break;
    case MovsdXmmE64: case MovssXmmE32:
    case MovqXmmXmm: case MovqXmmE64:
    case MovdXmmR32: case MovdXmmE32:
        scalarZeroMask |= 1u << op->reg;
        break;
    case XorpdXmmXmm: case XorpsXmmXmm: case PxorXmmXmm:
        // These are not on the whitelist, so preCompile already discarded
        // older proofs. A self XOR establishes a fresh zero value.
        if (op->reg == op->rm) scalarZeroMask |= 1u << op->reg;
        break;
    default:
        break;
    }
}

bool JitArmV8CodeGen::hasZeroUpper64(SSERegPtr reg) const {
    return reg->emulatedReg < 8 && (scalarZeroMask & (1u << reg->emulatedReg));
}

void JitArmV8CodeGen::preCompile(DecodedOp* op, bool skippedOp) {
    if (!scalarZeroEnabled || !blockOpCount || op->isBranch() ||
        (op->flags2 & OP_FLAG2_JUMP_TARGET) || !canTrackScalarZero(op)) {
        scalarZeroMask = 0;
        scalarZeroLane1Mask = 0;
    }
    // Every interior that carries a proof must reject independent entry, even
    // if this particular instruction does not use it. Keep real boundaries
    // for host fault recovery; an external dispatch can promote and recompile.
    if (scalarZeroMask || scalarZeroLane1Mask) scalarInternalEntries.insert(currentEip);
    if (!blockOpCount) fpuRegisterLoops = findFpuRegisterLoops(8);
    if (fpuRegisterLoop && currentEip > fpuRegisterLoop->backedge) {
        flushFpuCache();
        fpuRegisterLoop = nullptr;
    }
    bool loopHeader = false;
    if (!skippedOp) for (const auto& loop : fpuRegisterLoops) {
        if (loop.lazyInputs || loop.backedge >= lastOpEip) continue;
        if (currentEip == loop.header) {
            flushFpuCache();
            fpuRegisterLoop = &loop;
            loopHeader = true;
            break;
        }
    }
    const bool cachedBackedge = fpuRegisterLoop && currentEip == fpuRegisterLoop->backedge;
    // D8-D15 carry only the proved fixed-slot loop state. Other entries and
    // branch paths retain the normal published CPU-state contract.
    if ((op->isBranch() && !cachedBackedge) || !canKeepFpuCache(op) ||
        ((op->flags2 & OP_FLAG2_JUMP_TARGET) && !loopHeader)) flushFpuCache();
    else if (instructionInfo[op->inst].readMemWidth || instructionInfo[op->inst].writeMemWidth) {
        materializeFpuCache();
        for (auto& slot : fpuStackCache.slots) slot.dirty = slot.tagDirty = false;
    }
    if (fpuStackCache.entryTop) fpuInternalEntries.insert(currentEip);
    JitCodeGen::preCompile(op, skippedOp);
}

bool armJitHasFlagM() {
    return asmjit::CpuInfo::host().features().arm().has_flagm();
}

bool armJitHasFlagM2() {
    return asmjit::CpuInfo::host().features().arm().has_flagm2();
}

bool JitArmV8CodeGen::compileCarryChain(DecodedOp* op) {
    const bool subtract = op->inst == SubR32R32 || op->inst == SubR32I32;
    if (op->lock || (!subtract && op->inst != AddR32R32 && op->inst != AddR32I32) ||
            std::getenv("BOXEDWINE_ARM_CARRY_CHAIN_SLOW")) return false;
    auto isCarry = [subtract](DecodedOp* next) {
        return next && !next->lock && (subtract ?
            (next->inst == SbbR32R32 || next->inst == SbbR32I32) :
            (next->inst == AdcR32R32 || next->inst == AdcR32I32));
    };
    if (!isCarry(op->next)) return false;
    std::array<DecodedOp*, 8> chain{op};
    U32 count = 1, eip = currentEip + op->len, lastProducerEip = currentEip;
    for (DecodedOp* next = op->next; isCarry(next) && count < chain.size(); eip += next->len, next = next->next) {
        if (eip > lastOpEip || (next->flags2 & OP_FLAG2_JUMP_TARGET)) break;
        chain[count++] = next;
        lastProducerEip = eip;
        if (!next->getNeededFlagsAfter(FMASK_TEST)) break;
    }
    if (count < 2) return false;
    auto emit = [this, &chain, count, subtract]() {
        for (U32 i = 0; i < count; ++i) {
            DecodedOp* cur = chain[i];
            if (i) preCompile(cur, true);
            {
                RegPtr dst = getReg(cur->reg);
                const bool immediate = cur->inst == AddR32I32 || cur->inst == SubR32I32 ||
                    cur->inst == AdcR32I32 || cur->inst == SbbR32I32;
                RegPtr src = immediate ? getTmpReg() : getReadOnlyReg(cur->rm);
                if (immediate) movValue(JitWidth::b32, src, cur->imm);
                if (!i) {
                    if (subtract) compiler.subs(R32(dst), R32(dst), R32(src));
                    else compiler.adds(R32(dst), R32(dst), R32(src));
                } else {
                    if (subtract) compiler.sbcs(R32(dst), R32(dst), R32(src));
                    else compiler.adcs(R32(dst), R32(dst), R32(src));
                }
            }
            if (i + 1 < count) postCompile(cur);
        }
        cfInverted = subtract;
    };
    DecodedOp* last = chain[count - 1];
    if (last->getNeededFlagsAfter(FMASK_TEST)) {
        if (std::getenv("BOXEDWINE_ARM_CARRY_CONSUMERS_SLOW")) return false;
        return tryDirectConsumers(last, emit, FMASK_TEST, lastProducerEip - currentEip, 1);
    }
    emit();
    // Every interior is register-only and has no independent incoming edge.
    // Final flags are either dead here or consumed directly above.
    skipToOp = last;
    return true;
}

bool JitArmV8CodeGen::compileNarrowArithmetic(DecodedOp* op) {
    U32 bits;
    bool immediate;
    switch (op->inst) {
    case AddR8R8: bits = 8; immediate = false; break;
    case AddR8I8: bits = 8; immediate = true; break;
    case AddR16R16: bits = 16; immediate = false; break;
    case AddR16I16: bits = 16; immediate = true; break;
    default: return false;
    }
    if (op->lock || (bits == 8 && op->reg >= 4) || !op->next ||
            std::getenv("BOXEDWINE_ARM_NARROW_ARITH_SLOW")) return false;
    DecodedOp* next = op->next;
    JitConditional condition;
    if (next->isJumpCC()) condition = getJumpConditionFromOp(next);
    else if (next->isCMovCC()) condition = getCmovConditionFromOp(next);
    else if (next->isSetCC()) condition = getSetConditionFromOp(next);
    else return false;
    if (condition != JitConditional::L && condition != JitConditional::NL &&
            condition != JitConditional::LE && condition != JitConditional::NLE) return false;
    // Native NZCV saves the compound SF/OF calculation. For simpler conditions,
    // subtraction, or high-byte destinations, operand alignment cost dominated
    // on the measured ARM host; retain the existing lazy-flag lowering there.
    tryDirect(op, [this, op, bits, immediate]() {
        RegPtr dst = bits == 8 ? getReg8(op->reg) : getReg(op->reg);
        RegPtr left = getTmpReg();
        compiler.ubfiz(R32(left), R32(dst), 32 - bits, bits);
        cfInverted = false;
        if (!immediate && !(bits == 8 && op->rm >= 4) &&
                !std::getenv("BOXEDWINE_ARM_NARROW_SHIFTED_SLOW")) {
            RegPtr src = bits == 8 ? getReadOnlyReg8(op->rm) : getReadOnlyReg(op->rm);
            compiler.adds(R32(left), R32(left), R32(src), Shift(ShiftOp::kLSL, 32 - bits));
        } else {
            RegPtr right = getTmpReg();
            if (immediate) movValue(JitWidth::b32, right, op->imm << (32 - bits));
            else {
                RegPtr src = bits == 8 ? getReadOnlyReg8(op->rm) : getReadOnlyReg(op->rm);
                if (src->isHigh) {
                    compiler.ubfx(R32(right), R32(src), 8, 8);
                    compiler.lsl(R32(right), R32(right), 24);
                } else compiler.ubfiz(R32(right), R32(src), 32 - bits, bits);
            }
            compiler.adds(R32(left), R32(left), R32(right));
        }
        compiler.bfxil(R32(dst), R32(left), 32 - bits, bits);
    }, [this, op]() { JitCodeGen::compile(op); });
    return true;
}

bool JitArmV8CodeGen::compileNarrowIncDec(DecodedOp* op) {
    bool decrement;
    U32 bits;
    switch (op->inst) {
    case IncR8: bits = 8; decrement = false; break;
    case DecR8: bits = 8; decrement = true; break;
    case IncR16: bits = 16; decrement = false; break;
    case DecR16: bits = 16; decrement = true; break;
    default: return false;
    }
    if (!armJitHasFlagM() || std::getenv("BOXEDWINE_ARM_NARROW_FLAGS_SLOW")) return false;
    tryDirect(op, [this, op, bits, decrement]() {
        RegPtr dest = bits == 8 ? getReg8(op->reg) : getReg(op->reg);
        RegPtr result = getTmpReg();
        U32 shift = dest->isHigh ? 8 : 0;
        // SETF observes the sign bit and the bit above it. Sign extension
        // before arithmetic makes that pair encode narrow signed overflow.
        compiler.sbfx(R32(result), R32(dest), shift, bits);
        if (decrement) compiler.sub(R32(result), R32(result), 1);
        else compiler.add(R32(result), R32(result), 1);
        if (bits == 8) compiler.setf8(R32(result));
        else compiler.setf16(R32(result));
        compiler.bfi(R32(dest), R32(result), shift, bits);
        // Native carry is irrelevant: fused consumers may only read NZV.
    }, [this, op]() { JitCodeGen::compile(op); }, FMASK_TEST & ~CF);
    return true;
}

bool JitArmV8CodeGen::compileConditionalCompare(DecodedOp* op) {
    if (!std::getenv("BOXEDWINE_ARM_CCMP")) return false;
    auto supportedCompare = [](DecodedOp* candidate) {
        return candidate && !candidate->lock &&
            (candidate->inst == CmpR32R32 || candidate->inst == CmpR32I32);
    };
    if (!supportedCompare(op)) return false;
    DecodedOp* firstBranch = op->next;
    DecodedOp* second = firstBranch ? firstBranch->next : nullptr;
    DecodedOp* last = second ? second->next : nullptr;
    if (!firstBranch || !firstBranch->isJumpCC() || !supportedCompare(second) ||
            !last || !last->isJumpCC()) return false;
    const U32 firstBranchEip = currentEip + op->len;
    const U32 secondEip = firstBranchEip + firstBranch->len;
    const U32 lastEip = secondEip + second->len;
    const U32 target = firstBranchEip + firstBranch->len + firstBranch->imm;
    if (lastEip > lastOpEip || target != lastEip + last->len + last->imm ||
            ((firstBranch->flags2 | second->flags2 | last->flags2) & OP_FLAG2_JUMP_TARGET) ||
            ((instructionInfo[firstBranch->inst].flagsUsed | instructionInfo[last->inst].flagsUsed) & PF) ||
            !canJumpInBlock(firstBranchEip, firstBranch) || !canJumpInBlock(lastEip, last) ||
            firstBranch->getNeededFlagsAfter(FMASK_TEST) || last->getNeededFlagsAfter(FMASK_TEST)) return false;
    const JitConditional firstCondition = getJumpConditionFromOp(firstBranch);
    const JitConditional lastCondition = getJumpConditionFromOp(last);
    // CMP leaves ARM C as the inverse of x86 borrow. Choose an immediate NZCV
    // state that takes the final failure branch when the first check failed.
    auto accepts = [lastCondition](U32 flags) {
        const bool n = (flags & 8) != 0, z = (flags & 4) != 0;
        const bool c = (flags & 2) != 0, v = (flags & 1) != 0;
        switch (lastCondition) {
        case JitConditional::O: return v;
        case JitConditional::NO: return !v;
        case JitConditional::B: return !c;
        case JitConditional::NB: return c;
        case JitConditional::Z: return z;
        case JitConditional::NZ: return !z;
        case JitConditional::BE: return !c || z;
        case JitConditional::NBE: return c && !z;
        case JitConditional::S: return n;
        case JitConditional::NS: return !n;
        case JitConditional::L: return n != v;
        case JitConditional::NL: return n == v;
        case JitConditional::LE: return z || n != v;
        case JitConditional::NLE: return !z && n == v;
        default: return false;
        }
    };
    U32 fallback = 0;
    while (fallback < 16 && !accepts(fallback)) ++fallback;
    if (fallback == 16) return false;
    if (target <= lastEip) exitToRunLoopIfNeeded(currentEip);
    cfInverted = true;
    const auto firstNativeCondition = getCondCode(firstCondition);
    const bool useImmediate = std::getenv("BOXEDWINE_ARM_CCMP_IMMEDIATE") != nullptr;
    {
        RegPtr left = getReadOnlyReg(op->reg);
        if (useImmediate && op->inst == CmpR32I32) {
            cmp(JitWidth::b32, left, op->imm);
        } else {
            RegPtr right = op->inst == CmpR32I32 ? loadConst(op->imm) : getReadOnlyReg(op->rm);
            compiler.cmp(R32(left), R32(right));
        }
    }
    postCompile(op);
    preCompile(firstBranch, true);
    postCompile(firstBranch);
    preCompile(second, true);
    {
        RegPtr left = getReadOnlyReg(second->reg);
        if (useImmediate && second->inst == CmpR32I32 && second->imm <= 31) {
            compiler.ccmp(R32(left), second->imm, fallback, asmjit::arm::negate_cond(firstNativeCondition));
        } else {
            RegPtr right = second->inst == CmpR32I32 ? loadConst(second->imm) : getReadOnlyReg(second->rm);
            compiler.ccmp(R32(left), R32(right), fallback, asmjit::arm::negate_cond(firstNativeCondition));
        }
    }
    postCompile(second);
    preCompile(last, true);
    direct_jump(lastCondition, target);
    skipToOp = last;
    return true;
}

bool JitArmV8CodeGen::compileMultiplyFlags(DecodedOp* op) {
    if (op->lock || std::getenv("BOXEDWINE_ARM_MULTIPLY_FLAGS_SLOW")) return false;
    const bool oneOperand = op->inst == MulR32 || op->inst == IMulR32;
    const bool unsignedMultiply = op->inst == MulR32;
    const bool immediate = op->inst == ImulR32R32;
    if (!oneOperand && !immediate && op->inst != DimulR32R32) return false;
    if (op->getNeededFlagsAfter(FMASK_TEST & ~(CF | OF))) return false;
    bool fused = false;
    directMultiplyFlags = true;
    tryDirect(op, [this, op, oneOperand, unsignedMultiply, immediate, &fused]() {
        RegPtr left = getReadOnlyReg(oneOperand ? 0 : immediate ? op->rm : op->reg);
        RegPtr right = immediate ? loadConst(op->imm) : getReadOnlyReg(oneOperand ? op->reg : op->rm);
        RegPtr product = getTmpReg();
        if (unsignedMultiply) compiler.umull(R64(product), R32(left), R32(right));
        else compiler.smull(R64(product), R32(left), R32(right));
        RegPtr upperOrExtended = getTmpReg();
        if (unsignedMultiply) {
            compiler.lsr(R64(upperOrExtended), R64(product), 32);
            compiler.cmp(R32(upperOrExtended), 0);
        } else {
            compiler.sxtw(R64(upperOrExtended), R32(product));
            compiler.cmp(R64(product), R64(upperOrExtended));
        }
        // CF and OF are the same overflow predicate for MUL/IMUL. Keep that
        // predicate in Z; the other x86 arithmetic flags are undefined and
        // cannot be consumed by this direct path.
        if (oneOperand) compiler.lsr(R64(getReg(2)), R64(product), 32);
        compiler.mov(R32(getReg(oneOperand ? 0 : op->reg)), R32(product));
        cfInverted = false;
        fused = true;
    }, []() {}, CF | OF);
    directMultiplyFlags = false;
    return fused;
}

void JitArmV8CodeGen::emitIntegerParity(RegPtr result) {
    const char* mode = std::getenv("BOXEDWINE_ARM_PARITY_VALUE");
    if (!mode && !std::getenv("BOXEDWINE_ARM_PARITY_FUSION")) mode = "table";
    if (mode && !strcmp(mode, "table")) {
        directParityValue = getTmpReg();
        compiler.and_(R32(directParityValue), R32(result), 0xff);
        readCPU(JitWidth::b8, directParityValue, 0, offsetof(CPU, flagParityLookup), directParityValue);
        return;
    }
    RegPtr folded = getTmpReg();
    compiler.eor(R32(folded), R32(result), R32(result), Shift(ShiftOp::kLSR, 4));
    compiler.eor(R32(folded), R32(folded), R32(folded), Shift(ShiftOp::kLSR, 2));
    compiler.eor(R32(folded), R32(folded), R32(folded), Shift(ShiftOp::kLSR, 1));
    if (mode && !strcmp(mode, "xor")) {
        compiler.eor(R32(folded), R32(folded), 1);
        compiler.and_(R32(folded), R32(folded), 1);
        directParityValue = folded;
    } else {
        // V holds odd parity when no explicit value is retained.
        compiler.embed_uint32(0xba000401u | (R64(folded).id() << 5));
    }
}

bool JitArmV8CodeGen::compileIntegerParity(DecodedOp* op) {
    if (op->lock || !armJitHasFlagM() || std::getenv("BOXEDWINE_ARM_INTEGER_PARITY_SLOW")) return false;
    enum class ParityOp { Add, Sub, Cmp, Test, And, Or, Xor };
    ParityOp arithmetic;
    bool immediate;
    switch (op->inst) {
    case AddR32R32: arithmetic = ParityOp::Add; immediate = false; break;
    case AddR32I32: arithmetic = ParityOp::Add; immediate = true; break;
    case SubR32R32: arithmetic = ParityOp::Sub; immediate = false; break;
    case SubR32I32: arithmetic = ParityOp::Sub; immediate = true; break;
    case CmpR32R32: arithmetic = ParityOp::Cmp; immediate = false; break;
    case CmpR32I32: arithmetic = ParityOp::Cmp; immediate = true; break;
    case TestR32R32: arithmetic = ParityOp::Test; immediate = false; break;
    case TestR32I32: arithmetic = ParityOp::Test; immediate = true; break;
    case AndR32R32: arithmetic = ParityOp::And; immediate = false; break;
    case AndR32I32: arithmetic = ParityOp::And; immediate = true; break;
    case OrR32R32: arithmetic = ParityOp::Or; immediate = false; break;
    case OrR32I32: arithmetic = ParityOp::Or; immediate = true; break;
    case XorR32R32: arithmetic = ParityOp::Xor; immediate = false; break;
    case XorR32I32: arithmetic = ParityOp::Xor; immediate = true; break;
    default: return false;
    }
    if (!op->getNeededFlagsAfter(PF)) return false;
    bool fused = false;
    directIntegerParity = true;
    tryDirect(op, [this, op, arithmetic, immediate, &fused]() {
        const bool noWrite = arithmetic == ParityOp::Cmp || arithmetic == ParityOp::Test;
        RegPtr left = noWrite ? getReadOnlyReg(op->reg) : getReg(op->reg);
        RegPtr right = immediate ? loadConst(op->imm) : getReadOnlyReg(op->rm);
        RegPtr result = noWrite ? getTmpReg() : left;
        cfInverted = false;
        switch (arithmetic) {
        case ParityOp::Add: compiler.adds(R32(result), R32(left), R32(right)); break;
        case ParityOp::Sub: case ParityOp::Cmp:
            compiler.subs(R32(result), R32(left), R32(right)); cfInverted = true; break;
        case ParityOp::And: case ParityOp::Test:
            compiler.ands(R32(result), R32(left), R32(right)); break;
        case ParityOp::Or:
            compiler.orr(R32(result), R32(left), R32(right));
            compiler.ands(asmjit::a64::wzr, R32(result), R32(result)); break;
        case ParityOp::Xor:
            compiler.eor(R32(result), R32(left), R32(right));
            compiler.ands(asmjit::a64::wzr, R32(result), R32(result)); break;
        }
        emitIntegerParity(result);
        fused = true;
    }, []() {}, CF | ZF | SF | PF);
    directIntegerParity = false;
    directParityValue.reset();
    return fused;
}

bool JitArmV8CodeGen::compileNarrowParity(DecodedOp* op) {
    if (op->lock || !armJitHasFlagM() || std::getenv("BOXEDWINE_ARM_NARROW_PARITY_SLOW")) return false;
    enum class ParityOp { Add, Sub, Cmp, Test, And, Or, Xor };
    ParityOp operation;
    U32 bits;
    bool immediate;
    switch (op->inst) {
#define NARROW_PARITY_CASES(prefix, kind) \
    case prefix##R8R8: operation = ParityOp::kind; bits = 8; immediate = false; break; \
    case prefix##R8I8: operation = ParityOp::kind; bits = 8; immediate = true; break; \
    case prefix##R16R16: operation = ParityOp::kind; bits = 16; immediate = false; break; \
    case prefix##R16I16: operation = ParityOp::kind; bits = 16; immediate = true; break;
    NARROW_PARITY_CASES(Add, Add)
    NARROW_PARITY_CASES(Sub, Sub)
    NARROW_PARITY_CASES(Cmp, Cmp)
    NARROW_PARITY_CASES(Test, Test)
    NARROW_PARITY_CASES(And, And)
    NARROW_PARITY_CASES(Or, Or)
    NARROW_PARITY_CASES(Xor, Xor)
#undef NARROW_PARITY_CASES
    default: return false;
    }
    // Terminal ops have no successor. Filter candidates before looking ahead,
    // and leave truncated arithmetic chains on the established lowering.
    if (!op->next || !op->getNeededFlagsAfter(PF)) return false;
    // A standalone high-byte SETP/SETNP measured faster on the established
    // path in changing-input tests. Retain that lowering;
    // high-byte branch and CMOV consumers still benefit from fusion.
    if (bits == 8 && op->reg >= 4 && op->next && op->next->isSetCC() &&
            !op->next->getNeededFlagsAfter(FMASK_TEST) &&
            !std::getenv("BOXEDWINE_ARM_NARROW_PARITY_HIGH_SET")) return false;
    bool fused = false;
    directIntegerParity = true;
    tryDirect(op, [this, op, bits, immediate, operation, &fused]() {
        const bool noWrite = operation == ParityOp::Cmp || operation == ParityOp::Test;
        RegPtr dst = bits == 8 ? getReadOnlyReg8(op->reg) : getReadOnlyReg(op->reg);
        RegPtr result = getTmpReg();
        const bool lowBits = !std::getenv("BOXEDWINE_ARM_NARROW_PARITY_LOW_BITS_SLOW");
        RegPtr left = dst;
        if (!lowBits || dst->isHigh) {
            compiler.ubfx(R32(result), R32(dst), dst->isHigh ? 8 : 0, bits);
            left = result;
        }
        RegPtr right;
        if (immediate) right = loadConst(op->imm);
        else {
            RegPtr src = bits == 8 ? getReadOnlyReg8(op->rm) : getReadOnlyReg(op->rm);
            if (lowBits && !src->isHigh) right = src;
            else {
                right = getTmpReg();
                compiler.ubfx(R32(right), R32(src), src->isHigh ? 8 : 0, bits);
            }
        }
        // With CF/OF excluded, upper operand bits cannot affect the low
        // result, its parity, or SETF8/16's N/Z. High-byte operands still
        // need extraction; BFI below preserves the destination's other bits.
        switch (operation) {
        case ParityOp::Add: compiler.add(R32(result), R32(left), R32(right)); break;
        case ParityOp::Sub: case ParityOp::Cmp: compiler.sub(R32(result), R32(left), R32(right)); break;
        case ParityOp::Test: case ParityOp::And: compiler.and_(R32(result), R32(left), R32(right)); break;
        case ParityOp::Or: compiler.orr(R32(result), R32(left), R32(right)); break;
        case ParityOp::Xor: compiler.eor(R32(result), R32(left), R32(right)); break;
        }
        // SETF obtains N/Z from the narrow result without aligning operands.
        // Carry and overflow consumers remain on the established lowering.
        if (op->getNeededFlagsAfter(ZF | SF)) {
            if (bits == 8) compiler.setf8(R32(result));
            else compiler.setf16(R32(result));
        }
        if (!noWrite) {
            RegPtr write = bits == 8 ? getReg8(op->reg) : getReg(op->reg);
            compiler.bfi(R32(write), R32(result), write->isHigh ? 8 : 0, bits);
        }
        emitIntegerParity(result);
        fused = true;
    }, []() {}, PF | ZF | SF);
    directIntegerParity = false;
    directParityValue.reset();
    return fused;
}

bool JitArmV8CodeGen::compileShiftFlags(DecodedOp* op) {
    const bool left = op->inst == ShlR32I8;
    const bool arithmetic = op->inst == SarR32I8;
    if ((!left && !arithmetic && op->inst != ShrR32I8) || op->lock ||
            !armJitHasFlagM() || std::getenv("BOXEDWINE_ARM_SHIFT_FLAGS_SLOW")) return false;
    const U32 count = op->imm & 31;
    if (!count) return false;
    const bool parity = !std::getenv("BOXEDWINE_ARM_SHIFT_PARITY_SLOW") &&
        op->getNeededFlagsAfter(PF) && !op->getNeededFlagsAfter(OF);
    const U32 supported = CF | ZF | SF | (parity ? PF : count == 1 ? OF : 0);
    const bool needCarry = op->getNeededFlagsAfter(CF) != 0;
    const bool needOverflow = count == 1 && op->getNeededFlagsAfter(OF);
    bool fused = false;
    directIntegerParity = parity;
    tryDirect(op, [this, op, count, left, arithmetic, needCarry, needOverflow, parity, &fused]() {
        RegPtr dst = getReg(op->reg);
        if (left && count == 1) {
            compiler.adds(R32(dst), R32(dst), R32(dst));
        } else {
            RegPtr original;
            if (needCarry || (needOverflow && !arithmetic)) {
                original = getTmpReg();
                compiler.mov(R32(original), R32(dst));
            }
            if (left) compiler.lsl(R32(dst), R32(dst), count);
            else if (arithmetic) compiler.asr(R32(dst), R32(dst), count);
            else compiler.lsr(R32(dst), R32(dst), count);
            // ANDS supplies N/Z and clears V. RMIF changes only the selected
            // flag bits; x86 carry is the last bit shifted out of the source.
            compiler.ands(asmjit::a64::wzr, R32(dst), R32(dst));
            // This AsmJit version has no RMIF emitter. Encoding is the Arm
            // FEAT_FlagM RMIF Xn,#shift,#mask form (validated with clang).
            auto rmif = [&](U32 shift, U32 mask) {
                compiler.embed_uint32(0xba000400u | (shift << 15) |
                    (R64(original).id() << 5) | mask);
            };
            if (needCarry) rmif(left ? 31 - count : (count + 62) & 63, 2);
            if (needOverflow && !arithmetic) rmif(31, 1);
        }
        if (parity) emitIntegerParity(dst);
        cfInverted = false;
        fused = true;
    }, []() {}, supported);
    directIntegerParity = false;
    directParityValue.reset();
    if (!fused) JitCodeGen::compile(op);
    return true;
}

bool JitArmV8CodeGen::useColdPollExit() const {
    return !forceInlinePoll && !std::getenv("BOXEDWINE_ARM_INLINE_POLL");
}

void JitArmV8CodeGen::exitToRunLoopIfNeeded(U32 eip) {
    if (!useColdPollExit() || std::getenv("BOXEDWINE_ARM_POLL_EIP_INLINE")) {
        JitCodeGen::exitToRunLoopIfNeeded(eip);
        return;
    }
    emitColdPollExit([this, eip]() { writeEip(eip - cpu->seg[CS].address); });
}

void JitArmV8CodeGen::exitToRunLoopIfNeeded(RegPtr eip) {
    if (!useColdPollExit()) {
        JitCodeGen::exitToRunLoopIfNeeded(eip);
        return;
    }
    emitColdPollExit([this, eip]() { writeEip(eip); });
}

bool JitArmV8CodeGen::compileOps(DecodedOp* op) {
    if (!JitCodeGen::compileOps(op)) return false;
    if (pollExitSection && (code.text_section()->buffer().size() + pollExitSection->buffer().size() > 512 * 1024 ||
            pollExitSection->buffer().size() > 32 * 1024)) {
        // Retry before publishing anything. Bound both CBZ/CBNZ displacement
        // and the cold tail included in the final instruction's U16 jitLen.
        struct Reset { bool previous; ~Reset() { forceInlinePoll = previous; } } reset{forceInlinePoll};
        forceInlinePoll = true;
#ifdef __TEST
        ++largePollFallbacks;
#endif
        startNewJIT(cpu, startingEip, op);
        return false;
    }
    return true;
}

void JitArmV8CodeGen::emitColdPollExit(const std::function<void()>& writeRestart) {
    // Emit now, while the exact GP and x87 cache state at the poll is available.
    // compileOps retries oversized blocks using inline exits before publishing.
    if (!pollExitSection) {
        auto error = code.new_section(asmjit::Out(pollExitSection), ".poll-exits", SIZE_MAX,
            asmjit::SectionFlags::kExecutable, 4, 1);
        if (error != asmjit::Error::kOk) kpanic("ARM JIT could not allocate poll exits");
    }
    Label exit = compiler.new_label();
#ifdef BOXEDWINE_MULTI_THREADED
    RegPtr pending = readCPU(JitWidth::b32, offsetof(CPU, jitSignalPending));
    compiler.cbnz(R32(pending), exit);
#else
    RegPtr budget = readCPU(JitWidth::b32, offsetof(CPU, jitBranchBudget));
    subValue(JitWidth::b32, budget, 1);
    writeCPU(JitWidth::b32, offsetof(CPU, jitBranchBudget), budget);
    compiler.cbz(R32(budget), exit);
#endif
    auto* hot = compiler.current_section();
    compiler.section(pollExitSection);
    compiler.bind(exit);
    writeRestart();
    writeCPUValue(DYN_PTR, offsetof(CPU, nextOp), 0);
    blockExit();
    compiler.section(hot);
}

void JitArmV8CodeGen::jumpToEipIfCached(RegPtr eipReg) {
    auto& cache = getMemData(cpu->memory)->opCache;
    // Flat lookup helps jumps and returns, but regressed the measured
    // multi-target indirect-call loops. Keep calls on the page-tree path.
    const bool flatCalls = std::getenv("BOXEDWINE_ARM_FLAT_CALL") &&
        !std::getenv("BOXEDWINE_ARM_FLAT_CALL_SLOW");
    if (!cache.getFlatJitEntries() ||
            (currentOp && currentOp->isCall() && !flatCalls) ||
            (currentOp && currentOp->isRet() && std::getenv("BOXEDWINE_ARM_FLAT_RETURN_SLOW"))) {
        JitCodeGen::jumpToEipIfCached(eipReg);
        return;
    }
    if (!eipReg) eipReg = readEip();
    if (cpu->thread->process->hasSetSeg[CS]) addReg(JitWidth::b32, eipReg, getReadOnlySegAddress(CS));
    RegPtr index = eipReg;
    if (cache.hasFlatJitHash()) {
        index = getTmpReg();
        compiler.eor(R32(index), R32(eipReg), R32(eipReg), Shift(ShiftOp::kLSR, cache.getFlatJitHashShift()));
        if (cache.hasFlatJitPageHash()) {
            RegPtr high = getTmpReg();
            compiler.and_(R32(high), R32(eipReg), 0xffff0000u);
            compiler.eor(R32(index), R32(index), R32(high), Shift(ShiftOp::kLSR, 4));
        }
    }
    // Diagnostic controls for separating index dependencies from code-size
    // effects. Both leave the selected lookup mapping unchanged.
    if (const char* padding = std::getenv("BOXEDWINE_ARM_HASH_DIAGNOSTIC")) {
        if (!strcmp(padding, "nop")) {
            compiler.nop(); compiler.nop();
        } else if (!strcmp(padding, "unused")) {
            RegPtr unused = getTmpReg();
            compiler.and_(R32(unused), R32(eipReg), 0xffff0000u);
            compiler.eor(R32(unused), R32(index), R32(unused), Shift(ShiftOp::kLSR, 4));
        }
    }
    RegPtr target;
    const bool preserveCallLookup = std::getenv("BOXEDWINE_ARM_FLAT_BASE_JUMPS_ONLY") &&
        currentOp && (currentOp->isCall() || currentOp->isRet());
    if (cacheFlatJitBase(cpu) && !preserveCallLookup) {
        target = getTmpReg();
        compiler.ldr(R64(target), Mem(xFlatJitBase, R64(index), Shift(ShiftOp::kLSL, DYN_PTR_LSL)));
    } else {
        target = readCPU(DYN_PTR, offsetof(CPU, jitFlatEntries));
        readHost(DYN_PTR, createMemPtr(target, index, DYN_PTR_LSL, 0), target, false);
    }
    If(DYN_PTR, target); {
        jmpHost(target);
    } EndIf();
}

void JitArmV8CodeGen::compile(DecodedOp* op) {
    if (!compileScalarSseRegion(op) && !compileCachedMathRegion(op) && !compilePureFpuHelper(op) && !compileConditionalCompare(op) && !compileMultiplyFlags(op) && !compileIntegerParity(op) && !compileNarrowParity(op) && !compileShiftFlags(op) && !compileNarrowArithmetic(op) && !compileCarryChain(op) && !compileNarrowIncDec(op) && !compileCachedFpuOp(op)) JitCodeGen::compile(op);
}

static double armFpuExp2Minus1(double value) { return std::pow(2.0, value) - 1; }
static double armFpuAtanYX(double x, double y) { return std::atan2(y, x); }

bool JitArmV8CodeGen::compileCachedMathRegion(DecodedOp* op) {
    static const bool enabled = !std::getenv("BOXEDWINE_ARM_CACHED_MATH_REGION_SLOW");
    if (!enabled || !armCachedFpuMathEnabled() || !fpuStackCache.entryTop) return false;
    auto eligible = [](DecodedOp* cur) {
        return cur && !cur->lock && (cur->inst == FSIN || cur->inst == FCOS || cur->inst == F2XM1);
    };
    if (!eligible(op)) return false;
    auto& slot = cachedFpuSlot(0);
    if (!slot.loaded || slot.needsValidation) return false;
    std::array<DecodedOp*, 256> region{};
    U32 count = 0, eip = currentEip;
    bool clearC2 = false;
    for (auto* cur = op; eligible(cur) && count < region.size(); eip += cur->len, cur = cur->next) {
        if (eip > lastOpEip || (count && (cur->flags2 & OP_FLAG2_JUMP_TARGET))) break;
        region[count++] = cur;
        clearC2 |= cur->inst != F2XM1;
    }
    if (count < 2) return false;
#ifdef __TEST
    ++cachedMathRegions;
#endif
    const U8 reg = slot.value->hardwareReg();
    if (reg < 8 || reg > 15) kpanic("ARM cached math region requires a saved D register");
    // Publish metadata before the first ABI call: postCompile must never spill
    // a tag from a caller-clobbered register while the guest cache is saved.
    for (U8 i = 0; i < 8; ++i) spillFpuTag(i);
    spillFpuTop();
    auto saved = saveFpuTemporaries();
    compiler.blr(xWriteCacheToCPU);
    compiler.fmov(asmjit::a64::d0, Vec::make_d(reg));
    for (U32 i = 0; i < count; ++i) {
        if (i) preCompile(region[i], true);
        void* function = region[i]->inst == FSIN ? (void*)static_cast<double(*)(double)>(std::sin) :
            region[i]->inst == FCOS ? (void*)static_cast<double(*)(double)>(std::cos) : (void*)armFpuExp2Minus1;
        // Keep the exact sequence of library operations, with its intermediate
        // result already in the next call's argument register. No guest memory,
        // stack operation, branch, or independent entry crosses this region.
        callRuntimeHelper(function);
        if (i + 1 < count) postCompile(region[i]);
    }
    compiler.fmov(Vec::make_d(reg), asmjit::a64::d0);
    callLoadCacheFromCPU();
    restoreFpuTemporaries(saved);
    slot.dirty = true;
    if (clearC2) {
        RegPtr sw = readCPU(JitWidth::b32, offsetof(CPU, fpu.sw));
        andValue(JitWidth::b32, sw, ~0x0400u);
        writeCPU(JitWidth::b32, offsetof(CPU, fpu.sw), sw);
    }
    skipToOp = region[count - 1];
    return true;
}

// A two-double homogeneous aggregate is returned in D0/D1 by AAPCS64,
// including the Apple and Windows ARM64 variants.
struct ArmFpuSinCosResult { double sine; double cosine; };
static ArmFpuSinCosResult armFpuSinCos(double value) {
    return {std::sin(value), std::cos(value)};
}


bool JitArmV8CodeGen::compileCachedFpuExamine(DecodedOp* op) {
    if (op->inst != FXAM || !armCachedFpuExamineEnabled() ||
            !fpuStackCache.entryTop) return false;
    auto& slot = cachedFpuSlot(0);
    if (!slot.loaded || slot.needsValidation) return false;
#ifdef __TEST
    ++cachedFpuExamines;
#endif
    RegPtr value = getTmpReg(), sign = getTmpReg(), result = getTmpReg(), scratch = getTmpReg();
    compiler.fmov(R64(value), Vec::make_d(slot.value->hardwareReg()));
    compiler.lsr(R64(sign), R64(value), 63);
    compiler.lsl(R32(sign), R32(sign), 9);
    Label empty = compiler.new_label(), nonzero = compiler.new_label();
    Label hasExponent = compiler.new_label(), classified = compiler.new_label();
    compiler.cmp(R32(cachedFpuTag(0)), TAG_Empty);
    compiler.b_eq(empty);
    compiler.lsl(R64(value), R64(value), 1); // discard sign; retain every fraction bit
    movValue(JitWidth::b32, result, 0x0400);
    compiler.cbnz(R64(value), nonzero);
    movValue(JitWidth::b32, result, 0x4000);
    compiler.b(classified);
    compiler.bind(nonzero);
    compiler.ubfx(R64(scratch), R64(value), 53, 11);
    compiler.cbnz(R32(scratch), hasExponent);
    // The common method compares subnormals with floating zero. Keep that
    // comparison so the host's flush-input mode cannot change classification.
    compiler.fcmp(Vec::make_d(slot.value->hardwareReg()), asmjit::Imm(0));
    compiler.b_ne(classified);
    movValue(JitWidth::b32, result, 0x4000);
    compiler.b(classified);
    compiler.bind(hasExponent);
    compiler.cmp(R32(scratch), 0x7ff);
    compiler.b_ne(classified);
    movValue(JitWidth::b32, result, 0x0500);
    compiler.lsl(R64(value), R64(value), 11);
    compiler.cbz(R64(value), classified);
    movValue(JitWidth::b32, result, 0x0100);
    // Match the common NaN comparison's host exception status without changing
    // the retained payload. Empty slots bypass all floating comparisons.
    compiler.fcmp(Vec::make_d(slot.value->hardwareReg()), Vec::make_d(slot.value->hardwareReg()));
    compiler.b(classified);
    compiler.bind(empty);
    movValue(JitWidth::b32, result, 0x4100);
    compiler.bind(classified);
    orReg(JitWidth::b32, result, sign);
    RegPtr sw = readCPU(JitWidth::b32, offsetof(CPU, fpu.sw));
    andValue(JitWidth::b32, sw, ~0x4700u);
    orReg(JitWidth::b32, sw, result);
    writeCPU(JitWidth::b32, offsetof(CPU, fpu.sw), sw);
    return true;
}

bool JitArmV8CodeGen::compilePureFpuHelper(DecodedOp* op) {
    if (op->lock || !armFpuHelpersEnabled()) return false;
    if (compileCachedFpuExamine(op)) return true;
    void (*helper)(CPU*) = nullptr;
    switch (op->inst) {
    case F2XM1: helper = common_F2XM1; break;
    case FSIN: helper = common_FSIN; break;
    case FCOS: helper = common_FCOS; break;
    case FPTAN: helper = common_FPTAN; break;
    case FSINCOS: helper = common_FSINCOS; break;
    case FYL2X: helper = common_FYL2X; break;
    case FYL2XP1: helper = common_FYL2XP1; break;
    case FPATAN: helper = common_FPATAN; break;
    case FPREM: helper = common_FPREM; break;
    case FPREM_nearest: helper = common_FPREM_nearest; break;
    case FSCALE: helper = common_FSCALE; break;
    case FXTRACT: helper = common_FXTRACT; break;
    case FXAM: helper = common_FXAM; break;
    default: return false;
    }
    const bool cachedBinary = armCachedFpuMathEnabled() &&
        (op->inst == FYL2X || op->inst == FYL2XP1 || op->inst == FPATAN);
    const bool cachedMath = armCachedFpuMathEnabled() &&
        (op->inst == FSIN || op->inst == FCOS || op->inst == F2XM1 ||
         op->inst == FPTAN || op->inst == FSINCOS);
    if (cachedBinary && fpuStackCache.entryTop) {
        auto& x = cachedFpuSlot(0);
        auto& y = cachedFpuSlot(1);
        if (x.loaded && !x.needsValidation && y.loaded && !y.needsValidation) {
            // Both inputs are already binary64. Unknown/raw combinations retain
            // the common helper's exact conversion and representation behavior.
            // A retained value does not prove its slot is nonempty. The common
            // FPOP sets stack-fault and exception-summary bits for an empty ST0.
            {
                RegPtr empty = getTmpReg8();
                mov(JitWidth::b32, empty, cachedFpuTag(0));
                compareValue(JitWidth::b8, empty, TAG_Empty, JitEvaluate::EQUALS, empty);
                guardCachedFpuFallback(empty);
            }
            const U8 xReg = x.value->hardwareReg(), yReg = y.value->hardwareReg();
            if (xReg < 8 || xReg > 15 || yReg < 8 || yReg > 15 || xReg == yReg)
                kpanic("ARM cached binary math requires distinct saved D registers");
            auto saved = saveFpuTemporaries();
            auto function = op->inst == FYL2X ? common_fpuYLog2X :
                op->inst == FYL2XP1 ? common_fpuYLog2XPlus1 : armFpuAtanYX;
            compiler.blr(xWriteCacheToCPU);
            compiler.fmov(asmjit::a64::d0, Vec::make_d(xReg));
            compiler.fmov(asmjit::a64::d1, Vec::make_d(yReg));
            callRuntimeHelper((void*)function);
            compiler.fmov(Vec::make_d(yReg), asmjit::a64::d0);
            callLoadCacheFromCPU();
            restoreFpuTemporaries(saved);
            y.dirty = true;
            cachedFpuPop();
            return true;
        }
    }
    if (cachedMath && fpuStackCache.entryTop) {
        auto& slot = cachedFpuSlot(0);
        if (slot.loaded && !slot.needsValidation) {
            // Only a value already proved to be cached binary64 can use this
            // path. Raw or unknown inputs keep the common helper's exact
            // representation/conversion behavior. D8-D15 survive the host ABI.
            const U8 reg = slot.value->hardwareReg();
            if (reg < 8 || reg > 15) kpanic("ARM cached math requires a callee-saved D register");
            U8 cosineReg = INVALID_REG;
            if (op->inst == FSINCOS) {
                // Reserve the pushed result in the native cache before the
                // call. Neither this helper nor libm can observe guest TOP.
                cachedFpuPush(slot.value);
                cosineReg = cachedFpuSlot(0).value->hardwareReg();
                if (cosineReg < 8 || cosineReg > 15 || cosineReg == reg)
                    kpanic("ARM cached sincos requires distinct saved D registers");
            }
            auto saved = saveFpuTemporaries();
            void* function = op->inst == FSINCOS ? (void*)armFpuSinCos :
                op->inst == FSIN ? (void*)static_cast<double(*)(double)>(std::sin) :
                op->inst == FCOS ? (void*)static_cast<double(*)(double)>(std::cos) :
                op->inst == FPTAN ? (void*)static_cast<double(*)(double)>(std::tan) : (void*)armFpuExp2Minus1;
            compiler.blr(xWriteCacheToCPU);
            compiler.fmov(asmjit::a64::d0, Vec::make_d(reg));
            callRuntimeHelper(function);
            compiler.fmov(Vec::make_d(reg), asmjit::a64::d0);
            if (cosineReg != INVALID_REG) compiler.fmov(Vec::make_d(cosineReg), asmjit::a64::d1);
            callLoadCacheFromCPU();
            restoreFpuTemporaries(saved);
            slot.dirty = true;
            if (op->inst == FPTAN) cachedFpuLoadConst(offsetof(CPU, fOne));
            if (op->inst != F2XM1) {
                RegPtr sw = readCPU(JitWidth::b32, offsetof(CPU, fpu.sw));
                andValue(JitWidth::b32, sw, ~0x0400u);
                writeCPU(JitWidth::b32, offsetof(CPU, fpu.sw), sw);
            }
            return true;
        }
    }
    // These register-only helpers neither access guest memory nor redirect
    // execution. Preserve their cached/raw x87 semantics, but avoid decoding
    // and dispatching the following instruction through jitRunSingleOp.
    flushFpuCache();
    writeCurrentEip(0);
    callHostFunction((void*)helper, {DynParam(JitCallParamType::CPU)});
    return true;
}

bool JitArmV8CodeGen::compileScalarSseRegion(DecodedOp* op) {
    static const bool enabled = !std::getenv("BOXEDWINE_ARM_SCALAR_REGION_SLOW");
    if (!enabled) return false;
    static const bool elideCopies = !std::getenv("BOXEDWINE_ARM_SCALAR_ELIDE_COPIES_SLOW");
    static const bool divss = !std::getenv("BOXEDWINE_ARM_SCALAR_DIVSS_REGION_SLOW");
    const bool minMax = !std::getenv("BOXEDWINE_ARM_MINMAX_SELECT_SLOW");
    auto fullCopy = [](DecodedOp* cur) {
        if (!cur || cur->lock) return false;
        switch (cur->inst) {
        case MovapsXmmXmm: case MovupsXmmXmm: case MovapdXmmXmm:
        case MovupdXmmXmm: case MovdqaXmmXmm: case MovdquXmmXmm: return true;
        default: return false;
        }
    };
    auto width = [minMax](DecodedOp* cur) -> U32 {
        if (!cur || cur->lock) return 0;
        switch (cur->inst) {
        case AddssXmm: case SubssXmm: case MulssXmm: case SqrtssXmm: return 32;
        case AddsdXmmXmm: case SubsdXmmXmm: case MulsdXmmXmm: case SqrtsdXmmXmm: return 64;
        case MaxssXmm: case MinssXmm: return minMax ? 32 : 0;
        case MaxsdXmmXmm: case MinsdXmmXmm: return minMax ? 64 : 0;
        case DivssXmm: return divss ? 32 : 0;
        case DivsdXmmXmm: return 64;
        case MovssXmmXmm: return 32;
        case MovsdXmmXmm: return 64;
        case Cvtsi2ssXmmR32: return 32;
        case Cvtsi2sdXmmR32: return 64;
        default: return 0;
        }
    };
    U32 bits = width(op);
    if (!bits && !fullCopy(op)) return false;
    std::array<DecodedOp*, 256> region{};
    U32 count = 0, eip = currentEip, destinations = 0, arithmetic = 0;
    for (DecodedOp* cur = op; cur && count < region.size(); eip += cur->len, cur = cur->next) {
        if (eip > lastOpEip || (count && (cur->flags2 & OP_FLAG2_JUMP_TARGET))) break;
        const bool copy = fullCopy(cur);
        const U32 currentBits = width(cur);
        if (!copy && (!currentBits || (bits && currentBits != bits))) break;
        if (!bits && !copy) bits = currentBits;
        region[count++] = cur;
        if (copy) {
            if (cur->reg != cur->rm) {
                if (destinations & (1u << cur->rm)) destinations |= 1u << cur->reg;
                else destinations &= ~(1u << cur->reg);
            }
        } else {
            destinations |= 1u << cur->reg;
            arithmetic += cur->inst != MovssXmmXmm && cur->inst != MovsdXmmXmm;
        }
    }
    U32 destinationCount = 0;
    for (U32 r = 0; r < 8; ++r) if (destinations & (1u << r)) ++destinationCount;
    // A scalar move already costs one instruction. Only arithmetic contributes
    // a removed merge; charge final merges for every written destination.
    if (!bits || count < 2 || arithmetic <= destinationCount) return false;

    // Keep only scalar results in temporaries. The cached guest XMM registers
    // retain their upper lanes until one final merge per written destination.
    // No memory, helper, branch, mixed width, or interior entry crosses the span.
    std::array<SSERegPtr, 8> values{};
    std::array<U8, 8> upperOrigins{0, 1, 2, 3, 4, 5, 6, 7};
    auto lane = [bits](U8 reg) {
        Vec result = bits == 64 ? Vec::make_d(reg).d2() : Vec::make_s(reg).s4();
        result.set_element_index(0);
        return result;
    };
    auto merge = [this, &values, &lane](U8 index) {
        if (values[index]) {
            compiler.ins(lane(xmmCache[index]), lane(values[index]->hardwareReg()));
            values[index].reset();
        }
    };
    auto scalar = [bits](SSERegPtr reg) {
        return bits == 64 ? Vec::make_d(reg->hardwareReg()) : Vec::make_s(reg->hardwareReg());
    };
    auto source = [this, &values](U8 index) {
        return values[index] ? values[index] : loadCpuXMMReg(index);
    };
    MakeSSE* make = bits == 64 ? +[](SSERegPtr r) { return Vec::make_d(r->hardwareReg()).d(); } :
        +[](SSERegPtr r) { return Vec::make_s(r->hardwareReg()).s(); };
    bool divisionGuarded = false;
    for (U32 i = 0; i < count; ++i) {
        auto* cur = region[i];
        if (i) {
            // Pending low lanes require the region's live temporaries. Hide
            // these entries, but keep them promotable like scalar-zero proof
            // interiors: an independent dispatch must recompile the region
            // with a boundary here, rather than inherit its temporary state.
            preCompile(cur);
            scalarInternalEntries.insert(currentEip);
        }
        // preOp resets the per-instruction allocator; these values span ops.
        for (const auto& value : values) if (value) vUsed[value->hardwareReg()] = true;
        if (fullCopy(cur)) {
            if (cur->reg != cur->rm) {
                const bool redundant = elideCopies && values[cur->rm] &&
                    upperOrigins[cur->reg] == upperOrigins[cur->rm];
                // The cached Q registers still carry the correct upper
                // lanes. Share the pending low value without serializing
                // arithmetic through an INS and a full-width copy.
                // Arithmetic below separates aliases before writing them.
                values[cur->reg] = values[cur->rm];
                // Scalar operations only write temporaries until the final
                // merge. Equal origins therefore prove identical upper lanes.
                // A pending low value supplies the rest of this full copy;
                // without one, the physical low lane must still be copied.
                if (!redundant) compiler.mov(Vec::make_q(xmmCache[cur->reg]), Vec::make_q(xmmCache[cur->rm]));
#ifdef __TEST
                else ++elidedVectorCopies;
#endif
                upperOrigins[cur->reg] = upperOrigins[cur->rm];
            }
            if (i + 1 < count) postCompile(cur);
            continue;
        }
        if (cur->inst == DivssXmm && !divisionGuarded) {
            // No instruction in this region changes MXCSR. Publish all
            // preceding scalar results before interpreting this exact DIVSS
            // when its exceptions are unmasked. The slow arm never rejoins.
            auto savedCache = fpuStackCache;
            RegPtr unmasked = readCPU(JitWidth::b32, offsetof(CPU, sseDivExceptionsUnmasked));
            If(JitWidth::b32, unmasked); {
                for (U8 r = 0; r < 8; ++r) if (values[r])
                    compiler.ins(lane(xmmCache[r]), lane(values[r]->hardwareReg()));
                emulateSingleOp();
                blockExit();
            } EndIf();
            fpuStackCache = savedCache;
            divisionGuarded = true;
        }
        auto left = source(cur->reg), right = source(cur->rm);
        bool shared = false;
        if (values[cur->reg]) for (U32 r = 0; r < values.size(); ++r)
            if (r != cur->reg && values[r] == values[cur->reg]) shared = true;
        if (!values[cur->reg] || shared) values[cur->reg] = getTmpSSE();
        Vec dst = scalar(values[cur->reg]), lhs = scalar(left), rhs = scalar(right);
        switch (cur->inst) {
        case AddssXmm: case AddsdXmmXmm: compiler.fadd(dst, lhs, rhs); break;
        case SubssXmm: case SubsdXmmXmm: compiler.fsub(dst, lhs, rhs); break;
        case MulssXmm: case MulsdXmmXmm: compiler.fmul(dst, lhs, rhs); break;
        case DivssXmm: case DivsdXmmXmm: compiler.fdiv(dst, lhs, rhs); break;
        case SqrtssXmm: case SqrtsdXmmXmm: compiler.fsqrt(dst, rhs); break;
        case MaxssXmm: case MaxsdXmmXmm: fmax(values[cur->reg], left, right, make); break;
        case MinssXmm: case MinsdXmmXmm: fmin(values[cur->reg], left, right, make); break;
        case MovssXmmXmm: case MovsdXmmXmm: compiler.fmov(dst, rhs); break;
        case Cvtsi2ssXmmR32: case Cvtsi2sdXmmR32:
            compiler.scvtf(dst, R32(getReadOnlyReg(cur->rm))); break;
        default: kpanic("Unexpected scalar SSE region instruction");
        }
        if (i + 1 < count) postCompile(cur);
    }
    for (U8 r = 0; r < 8; ++r) merge(r);
    skipToOp = region[count - 1];
    return true;
}

void JitArmV8CodeGen::postCompile(DecodedOp* op) {
    updateScalarZero(op);
    // Metadata uses ordinary GP temporaries; values have eight dedicated D regs.
    for (U8 i = 0; i < 8; ++i) spillFpuTag(i);
    spillFpuTop();
    JitCodeGen::postCompile(op);
}

void JitArmV8CodeGen::nativeFpuIndex(U8 index) {
    // Reserved scratch registers avoid allocator recursion and preserve NZCV.
    auto& top = fpuStackCache.entryTop;
    if (top && top->isLoaded()) compiler.mov(wScratch, R32(top));
    else compiler.ldr(wScratch, Mem(xCPU, offsetof(CPU, armFpuCacheTop)));
    if (index) {
        compiler.add(wScratch, wScratch, index);
        compiler.and_(wScratch, wScratch, 7);
    }
}

void JitArmV8CodeGen::materializeFpuCache(U8 slotMask) {
    if (!fpuStackCache.entryTop) return;
    bool wroteTop = false;
    for (U8 i = 0; i < 8; ++i) {
        auto& slot = fpuStackCache.slots[i];
        if (!(slotMask & (1u << i))) continue;
        bool tagDirty = slot.tagDirty && slot.tag->isLoaded();
        if (!slot.dirty && !tagDirty) continue;
        nativeFpuIndex(i);
        if (slotMask == 0xff && i == fpuStackCache.top) {
            compiler.str(wScratch, Mem(xCPU, offsetof(CPU, fpu.top)));
            wroteTop = true;
        }
        if (slot.dirty) {
            compiler.add(xBranch, xCPU, xScratch, Shift(ShiftOp::kLSL, 3));
            compiler.str(Vec::make_d(slot.value->hardwareReg()), Mem(xBranch, offsetof(CPU, fpu.regCache)));
        }
        compiler.add(xScratch, xCPU, xScratch);
        if (tagDirty) compiler.strb(R32(slot.tag), Mem(xScratch, offsetof(CPU, fpu.tags)));
        if (slot.dirty) {
            compiler.mov(wBranch, 1);
            compiler.strb(wBranch, Mem(xScratch, offsetof(CPU, fpu.isRegCached)));
        }
    }
    // Conditional conversion helpers may expose a different intermediate TOP.
    if (slotMask == 0xff && !wroteTop) {
        nativeFpuIndex(fpuStackCache.top);
        compiler.str(wScratch, Mem(xCPU, offsetof(CPU, fpu.top)));
    }
}

void JitArmV8CodeGen::spillFpuTag(U8 index) {
    auto& slot = fpuStackCache.slots[index];
    if (!slot.tag || !slot.tag->isLoaded()) return;
    if (slot.tagDirty) {
        nativeFpuIndex(index);
        compiler.add(xScratch, xCPU, xScratch);
        compiler.strb(R32(slot.tag), Mem(xScratch, offsetof(CPU, fpu.tags)));
    }
    regUsed[slot.tag->hardwareReg()] = false;
    slot.tag->invalidateHardwareReg();
    slot.tagDirty = false;
}

void JitArmV8CodeGen::preserveFpuCacheTop(RegPtr top) {
    compiler.str(R32(top), Mem(xCPU, offsetof(CPU, armFpuCacheTop)));
}

void JitArmV8CodeGen::spillFpuTop() {
    auto& top = fpuStackCache.entryTop;
    if (!top || !top->isLoaded()) return;
    // Stack pushes/pops change the relative mapping, not the saved entry TOP.
    regUsed[top->hardwareReg()] = false;
    top->invalidateHardwareReg();
}

bool JitArmV8CodeGen::spillFpuTmp() {
    for (U8 i = 0; i < 8; ++i) {
        auto& tag = fpuStackCache.slots[i].tag;
        if (tag && tag->isLoaded() && tag.use_count() == 1) {
            spillFpuTag(i);
            return true;
        }
    }
    auto& top = fpuStackCache.entryTop;
    if (top && top->isLoaded() && top.use_count() == 1) {
        spillFpuTop();
        return true;
    }
    return false;
}

RegPtr JitArmV8CodeGen::getFpuCacheTopReg() {
    return RegPtr(new JitReg(INVALID_REG, INVALID_REG, [this] {
        U8 reg = findTmpReg();
        compiler.ldr(R32(reg), Mem(xCPU, offsetof(CPU, armFpuCacheTop)));
        return reg;
    }), [this](JitReg* reg) {
        if (reg->isLoaded()) regUsed[reg->hardwareReg()] = false;
        delete reg;
    });
}

RegPtr JitArmV8CodeGen::getFpuCacheTagReg(U8) {
    auto holder = new JitReg*(nullptr);
    RegPtr result(new JitReg(INVALID_REG, INVALID_REG, [this, holder] { return allocateFpuTag(*holder); }),
        [this, holder](JitReg* reg) {
            if (reg->isLoaded()) regUsed[reg->hardwareReg()] = false;
            delete holder;
            delete reg;
        });
    *holder = result.get();
    return result;
}

U8 JitArmV8CodeGen::allocateFpuTag(JitReg* tag, bool load) {
    U8 reg = findTmpReg();
    for (U8 i = 0; i < 8; ++i) if (fpuStackCache.slots[i].tag.get() == tag) {
        if (load) {
            nativeFpuIndex(i);
            compiler.add(xScratch, xCPU, xScratch);
            compiler.ldrb(R32(reg), Mem(xScratch, offsetof(CPU, fpu.tags)));
        }
        return reg;
    }
    kpanic("ARM x87 tag without slot");
    return reg;
}

U8 JitArmV8CodeGen::getFpuWriteReg(RegPtr tag) {
    if (!tag->isLoaded()) {
        for (auto& slot : fpuStackCache.slots) if (slot.tag == tag) {
            tag->setHardwareReg(allocateFpuTag(tag.get(), false));
            break;
        }
    }
    return tag->hardwareReg();
}

FPURegPtr JitArmV8CodeGen::getFpuCacheReg(U8 index) {
    // D8-D15 are disjoint from guest SSE (V0-V7) and temporaries (V16-V24).
    // Their low 64 bits survive host calls; JIT entry/exit preserves host values.
    return std::make_shared<FPURegInternal>(8 + index);
}

void JitArmV8CodeGen::moveFpuReg(FPURegPtr dst, FPURegPtr src) {
    compiler.fmov(Vec::make_d(dst->hardwareReg()), Vec::make_d(src->hardwareReg()));
}

static void armCacheFpuValue(CPU* cpu, U32 index) { cpu->fpu.getF64(index); }

bool JitArmV8CodeGen::loadFpuValue(FPURegPtr dst, RegPtr index, RegPtr valid, bool nearest) {
    if (valid || !nearest) kpanic("unsupported ARM x87 cache validation");
    IfNotRegCached(index);
    callHostFunction((void*)armCacheFpuValue, {DynParam(JitCallParamType::CPU), DynParam(JitCallParamType::REG_32, index)});
    EndIf();
    loadCpuFpuReg(dst, index);
    return false;
}

std::pair<U32, U32> JitArmV8CodeGen::saveFpuTemporaries(U8 result) {
    std::pair<U32, U32> saved{};
    if (!fpuStackCache.entryTop) return saved;
    for (U8 reg : tmps) if (reg < 19 && regUsed[reg] && reg != result) {
        saved.first |= 1u << reg;
        compiler.str(R64(reg), Mem(asmjit::a64::sp, -16).pre());
    }
    for (U8 reg : vtmps) if (vUsed[reg]) {
        saved.second |= 1u << reg;
        compiler.str(Vec::make_v128(reg), Mem(asmjit::a64::sp, -16).pre());
    }
    return saved;
}

void JitArmV8CodeGen::restoreFpuTemporaries(std::pair<U32, U32> saved) {
    for (int i = NUMBER_OF_VREG_TMPS - 1; i >= 0; --i) if (saved.second & (1u << vtmps[i]))
        compiler.ldr(Vec::make_v128(vtmps[i]), Mem(asmjit::a64::sp, 16).post());
    for (int i = NUMBER_OF_TMPS - 1; i >= 0; --i) if (saved.first & (1u << tmps[i]))
        compiler.ldr(R64(tmps[i]), Mem(asmjit::a64::sp, 16).post());
}

void JitArmV8CodeGen::preOp(DecodedOp* op) {
    rUsed.fill(false);
    vUsed.fill(false);
    currentOp = op;
    Label label;
    if (!opLabels.get(currentEip, label)) {
        label = compiler.new_label();
        opLabels.set(currentEip, label);
    }
    if (fpuRegisterLoop && currentEip == fpuRegisterLoop->header) {
        // The public entry includes imports; the private backedge skips them.
        // GP scratch metadata is reloaded per instruction, unlike D8-D15.
        prepareFpuRegisterLoop(fpuRegisterLoop->slots);
        spillFpuTop();
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
    const bool memory = currentOp && (instructionInfo[currentOp->inst].readMemWidth ||
        instructionInfo[currentOp->inst].writeMemWidth);
    const bool cold = memory && !forceInlinePoll && !ifLabels.empty() &&
        std::getenv("BOXEDWINE_ARM_MEMORY_EXIT_COLD");
    asmjit::Section* originalSection = compiler.current_section();
    if (cold) {
        if (!pollExitSection) {
            auto error = code.new_section(asmjit::Out(pollExitSection), ".poll-exits", SIZE_MAX,
                asmjit::SectionFlags::kExecutable, 4, 1);
            if (error != asmjit::Error::kOk) kpanic("ARM JIT could not allocate cold memory exits");
        }
        Label exit = compiler.new_label();
        compiler.b(exit);
        compiler.section(pollExitSection);
        compiler.bind(exit);
    }
    materializeFpuCache();
    writeCurrentEip(0);

    if (cacheLinearMemoryBase()) {
        if (std::getenv("BOXEDWINE_ARM_HELPER_LITERAL")) {
            // This is a tail transfer: execution cannot fall through into the
            // immutable pointer literal. The next guest entry follows it.
            Label pointer = compiler.new_label();
            compiler.ldr(xBranch, Mem(pointer));
            compiler.br(xBranch);
            compiler.bind(pointer);
            compiler.embed_uint64((DYN_PTR_SIZE)cpu->thread->process->emulateSingleOp);
        } else {
            compiler.mov(xBranch, (DYN_PTR_SIZE)cpu->thread->process->emulateSingleOp);
            compiler.br(xBranch);
        }
    } else {
        compiler.br(xEmulateSingleOp);
    } // we won't return to here
    if (cold) compiler.section(originalSection);
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
    if (spillFpuTmp()) return findTmpReg(allowInvalidReturn);
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

U32 JitArmV8CodeGen::getAvailableTmpRegCount() {
    if (disableTmps) {
        return 0;
    }
    U32 result = 0;
    for (int i = 0; i < NUMBER_OF_TMPS; i++) {
        if (!regUsed[tmps[i]]) {
            result++;
        }
    }
    return result;
}

bool JitArmV8CodeGen::isTmpRegAvailable() {
    return getAvailableTmpRegCount() != 0;
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
    // Guest address arithmetic wraps at 32 bits. A negative displacement can
    // therefore use SUB immediate without materializing its unsigned value.
    // Do not apply this to a 64-bit add, where that value is positive.
    if (regWidth == JitWidth::b32 && asmjit::a64::Utils::is_add_sub_imm(0u - imm)) {
        compiler.sub(R32(reg), R32(reg), 0u - imm);
        return;
    }
    if (!asmjit::a64::Utils::is_add_sub_imm(imm)) {
        addReg(regWidth, reg, loadConst(imm));
        return;
    }
    regValue(regWidth, reg, imm, [this](asmjit::a64::Gp dst, asmjit::a64::Gp reg, U32 value) {
        compiler.add(dst, reg, value);
    });
}

void JitArmV8CodeGen::addValueWithDest(JitWidth regWidth, RegPtr dst, RegPtr reg, U32 value) {
    if (regWidth == JitWidth::b32) {
        if (asmjit::a64::Utils::is_add_sub_imm(value)) {
            compiler.add(R32(dst), R32(reg), value);
        } else if (asmjit::a64::Utils::is_add_sub_imm(0u - value)) {
            compiler.sub(R32(dst), R32(reg), 0u - value);
        } else {
            compiler.add(R32(dst), R32(reg), R32(loadConst(value)));
        }
        return;
    }
    if (asmjit::a64::Utils::is_add_sub_imm(value)) {
        if (regWidth == JitWidth::b64) {
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
        compiler.lsr(R32(tmp1), R32(src), R32(cl));

        // (var1 << (9 - var2))       
        compiler.sub(R32(tmp2), R32(tmp2), R32(cl));
        compiler.lsl(R32(tmp2), R32(src), R32(tmp2));

        compiler.orr(R32(tmp1), R32(tmp2), R32(tmp1));

        // (cf << (8 - var2))
        compiler.sub(R32(tmp2), R32(cl), 8);
        compiler.neg(R32(tmp2), R32(tmp2));
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
        IfGreaterThanOrEqual(JitWidth::b32, ComparisonType::Unsigned, cl, 0x10); {
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
        IfGreaterThanOrEqual(JitWidth::b32, ComparisonType::Unsigned, cl, 0x10); {
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
        if (overflow) {
            compiler.smull(R64(dst), R32(dst), R32(src));
            compiler.lsr(R64(overflow), R64(dst), 32);
            compiler.mov(R32(dst), R32(dst)); // clear out top 32-bits
        } else {
            // A truncated product has identical low bits for signed and
            // unsigned inputs. The W form also clears the upper host bits.
            compiler.mul(R32(dst), R32(dst), R32(src));
        }
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
    if (regWidth == JitWidth::b32) {
        RegPtr value = loadConst(src2);
        if (overflow) {
            compiler.smull(R64(dst), R32(src), R32(value));
            compiler.ubfx(R64(overflow), R64(dst), 32, 32);
            compiler.mov(R32(dst), R32(dst)); // clear out top 32-bits
        } else {
            compiler.mul(R32(dst), R32(src), R32(value));
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

void JitArmV8CodeGen::readMMU(RegPtr dest, RegPtr index, U32 offset) {
    compiler.ldr(R64(dest), createMem(regMMU, index->hardwareReg(), 3, offset));
}

void JitArmV8CodeGen::readMMU(RegPtr dest, U32 index) {
    compiler.ldr(R64(dest), createMem(regMMU, index * 8));
}

RegPtr JitArmV8CodeGen::getLinearMemoryBase(RegPtr tmp) {
    if (cacheLinearMemoryBase()) {
        // This callee-saved register is loaded at JIT entry and is never a
        // destination or temporary, including when a guest access faults.
        return std::make_shared<JitReg>(26, 0xff);
    }
    // A grouped linear-memory read can fault when another guest page in the
    // host-page group prevents a direct mapping. Keep the address base out of
    // the load destination so the exception context retains a valid address
    // calculation while the instruction is handled through the MMU path.
    if (!tmp || ramPageUseLinearMemoryAdjacent()) {
        tmp = getTmpReg();
    }
    constexpr S32 offset = (S32)offsetof(KMemoryData, linearMemoryBase) - (S32)offsetof(KMemoryData, mmu);
    compiler.ldr(R64(tmp), Mem(xMMU, offset));
    return tmp;
}

Mem JitArmV8CodeGen::createMem(MemPtr mem) {
    if (mem->sib) {
        return createMem(mem->rm, mem->sib, mem->lsl, mem->offset);
    }
    return createMem(mem->rm, mem->offset);
}

Mem JitArmV8CodeGen::createMem1(MemPtr mem) {
    if (mem->sib) {
        if (mem->rm) {
            if (mem->offset) {
                RegPtr tmp = getTmpReg();
                compiler.add(R64(tmp), R64(mem->rm), R64(mem->sib));
                if (asmjit::a64::Utils::is_add_sub_imm(mem->offset)) {
                    compiler.add(R64(tmp), R64(tmp), mem->offset);
                } else {
                    compiler.add(R64(tmp), R64(tmp), R64(loadConst(mem->offset)));
                }
                return createMem(tmp, 0);
            } else {
                RegPtr tmp = getTmpReg();
                compiler.add(R64(tmp), R64(mem->rm), R64(mem->sib));
                return createMem(tmp, 0);
            }
        } else if (mem->offset) {
            RegPtr tmp = getTmpReg();
            if (asmjit::a64::Utils::is_add_sub_imm(mem->offset)) {
                compiler.add(R64(tmp), R64(mem->sib), mem->offset);
            } else {
                compiler.add(R64(tmp), R64(mem->sib), R64(loadConst(mem->offset)));
            }
            return createMem(tmp, 0);
        }
        return createMem(mem->sib, 0);
    }
    if (mem->offset) {
        RegPtr tmp = getTmpReg();
        if (asmjit::a64::Utils::is_add_sub_imm(mem->offset)) {
            compiler.add(R64(tmp), R64(mem->rm), mem->offset);
        } else {
            compiler.add(R64(tmp), R64(mem->rm), R64(loadConst(mem->offset)));
        }
        return createMem(tmp, 0);
    }
    return createMem(mem->rm, 0);
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

void JitArmV8CodeGen::movFromMemory(DecodedOp* op, JitWidth dstWidth, JitWidth srcWidth, bool signExtend) {
    // Resolve the address and any MMU fallback before touching the guest
    // destination, which may also be an input to the address calculation.
    read(srcWidth, calculateEaa(op), [this, op, dstWidth, srcWidth, signExtend](MemPtr address) {
        RegPtr dst = getReg(op->reg, -1, dstWidth != JitWidth::b32);
        RegPtr value = dstWidth == JitWidth::b32 ? dst : getTmpReg();
        readHostValue(srcWidth, address, value, true, signExtend);
        if (dstWidth != JitWidth::b32) {
            // ARM narrow loads replace the whole W register. Merge AX-sized
            // results only after the load succeeds to preserve the upper bits.
            mov(dstWidth, dst, value);
        }
    });
}

void JitArmV8CodeGen::readHost(JitWidth width, MemPtr mem, RegPtr dest, bool emlulatedMemory) {
    // arm zero extends reads
    if (!isTmp[dest->hardwareReg()] && (width == JitWidth::b8 || width == JitWidth::b16)) {
        RegPtr tmp = getTmpReg();
        readHost(width, mem, tmp, emlulatedMemory);
        mov(width, dest, tmp);
        return;
    }
    if (width == JitWidth::b8 && isRegHigh(dest)) {
        kpanic("JitArmV8CodeGen::readHost unexpected dest");
    }
    readHostValue(width, mem, dest, emlulatedMemory, false);
}

void JitArmV8CodeGen::readHostValue(JitWidth width, MemPtr mem, RegPtr dest, bool emlulatedMemory, bool signExtend) {
    // Always replace the whole W/X register, even for a byte or word load.
    // In particular, guest ESP-EDI destinations here do not mean AH-BH.
#ifdef BOXEDWINE_HOST_EXCEPTIONS
    static const bool signedLoads = !std::getenv("BOXEDWINE_ARM_SIGNED_ACQUIRE_LOADS_SLOW");
    const bool signedAcquire = signedLoads && signExtend &&
        (width == JitWidth::b8 || width == JitWidth::b16);
    if (emlulatedMemory && tsoMode == TSOMode::FEAT_LRCPC2 &&
            currentOp->exceptionCount < MAX_OP_EXCEPTION_COUNT && signedAcquire) {
        // Signed LDAPUR has the same acquire ordering and fault tier as
        // the existing LDAPR path, and replaces the following SXTB/SXTH.
        MemPtr address = mem->copy();
        RegPtr base = calculateAddress(address);
        U32 instruction = width == JitWidth::b8 ? 0x19c00000u : 0x59c00000u;
        compiler.embed_uint32(instruction | (base->hardwareReg() << 5) | dest->hardwareReg());
        return;
    }
#endif
    if (width == JitWidth::b32) {
#ifdef BOXEDWINE_HOST_EXCEPTIONS
        if (emlulatedMemory && tsoMode == TSOMode::FEAT_LRCPC2 && currentOp->exceptionCount < MAX_OP_EXCEPTION_COUNT) {
            // compiler.ldapr(R32(dest), createMemR(reg, sib, lsl, disp));
            RegPtr addressReg = calculateAddress(mem);
            U32 op = 0xB8BFC000 | dest->hardwareReg() | (addressReg->hardwareReg() << 5);
            compiler.embed_int32(op);
        } else
#endif
        {
            compiler.ldr(R32(dest), createMem(mem));
        }
    } else if (width == JitWidth::b16) {
#ifdef BOXEDWINE_HOST_EXCEPTIONS
        if (emlulatedMemory && tsoMode == TSOMode::FEAT_LRCPC2 && currentOp->exceptionCount < MAX_OP_EXCEPTION_COUNT) {
            // compiler.ldaprh(R32(dest), createMemR(reg, sib, lsl, disp));
            RegPtr addressReg = calculateAddress(mem);
            U32 op = 0x78BFC000 | dest->hardwareReg() | (addressReg->hardwareReg() << 5);
            compiler.embed_int32(op);
            if (signExtend) {
                compiler.sxth(R32(dest), R32(dest));
            }
        } else
#endif
        {
            if (signExtend) {
                compiler.ldrsh(R32(dest), createMem(mem));
            } else {
                compiler.ldrh(R32(dest), createMem(mem));
            }
        }
    } else if (width == JitWidth::b8) {
#ifdef BOXEDWINE_HOST_EXCEPTIONS
        if (emlulatedMemory && tsoMode == TSOMode::FEAT_LRCPC2 && currentOp->exceptionCount < MAX_OP_EXCEPTION_COUNT) {
            // compiler.ldaprb(R32(dest), createMemR(reg, sib, lsl, disp));
            RegPtr addressReg = calculateAddress(mem);
            U32 op = 0x38BFC000 | dest->hardwareReg() | (addressReg->hardwareReg() << 5);
            compiler.embed_int32(op);
            if (signExtend) {
                compiler.sxtb(R32(dest), R32(dest));
            }
        } else
#endif
        {
            if (signExtend) {
                compiler.ldrsb(R32(dest), createMem(mem));
            } else {
                compiler.ldrb(R32(dest), createMem(mem));
            }
        }
#ifdef BOXEDWINE_64
    } else if (width == JitWidth::b64) {
#ifdef BOXEDWINE_HOST_EXCEPTIONS
        if (emlulatedMemory && tsoMode == TSOMode::FEAT_LRCPC2 && currentOp->exceptionCount < MAX_OP_EXCEPTION_COUNT) {
            // compiler.ldapr(R64(dest), createMemR(reg, sib, lsl, disp));
            RegPtr addressReg = calculateAddress(mem);
            U32 op = 0xF8BFC000 | dest->hardwareReg() | (addressReg->hardwareReg() << 5);
            compiler.embed_int32(op);
        } else
#endif
        {
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
#ifdef BOXEDWINE_HOST_EXCEPTIONS
        if (emlulatedMemory && tsoMode == TSOMode::FEAT_LRCPC2 && currentOp->exceptionCount < MAX_OP_EXCEPTION_COUNT) {
            // compiler.stlur(R32(dest), createMem9(reg, disp));
			MemPtr memCopy = mem->copy();
            U32 imm = 0;

            if (memCopy->offset && (S32)memCopy->offset >= -256 && (S32)memCopy->offset <= 255) {
                // Quake 2 hits this path
                imm = memCopy->offset & 0x1ff;
                memCopy->offset = 0;
            }
            RegPtr addressReg = calculateAddress(memCopy);
            U32 op = 0x99000000 | src->hardwareReg() | (addressReg->hardwareReg() << 5) | (imm << 12);
            compiler.embed_int32(op);
        } else
#endif
        {
            compiler.str(R32(src), createMem(mem));
        }
    } else if (width == JitWidth::b16) {
#ifdef BOXEDWINE_HOST_EXCEPTIONS
        if (emlulatedMemory && tsoMode == TSOMode::FEAT_LRCPC2 && currentOp->exceptionCount < MAX_OP_EXCEPTION_COUNT) {
            // compiler.stlurh(R32(dest), createMem9(reg, disp));
            MemPtr memCopy = mem->copy();
            U32 imm = 0;

            if (memCopy->offset && (S32)memCopy->offset >= -256 && (S32)memCopy->offset <= 255) {
                imm = memCopy->offset & 0x1ff;
                memCopy->offset = 0;
            }
            RegPtr addressReg = calculateAddress(memCopy);
            U32 op = 0x59000000 | src->hardwareReg() | (addressReg->hardwareReg() << 5) | (imm << 12);
            compiler.embed_int32(op);
        } else
#endif
        {
            compiler.strh(R32(src), createMem(mem));
        }
    } else if (width == JitWidth::b8) {
#ifdef BOXEDWINE_HOST_EXCEPTIONS
        if (emlulatedMemory && tsoMode == TSOMode::FEAT_LRCPC2 && currentOp->exceptionCount < MAX_OP_EXCEPTION_COUNT) {
            // compiler.stlurb(R32(dest), createMem9(reg, disp));
            MemPtr memCopy = mem->copy();
            U32 imm = 0;

            if (memCopy->offset && (S32)memCopy->offset >= -256 && (S32)memCopy->offset <= 255) {
                imm = memCopy->offset & 0x1ff;
                memCopy->offset = 0;
            }
            RegPtr addressReg = calculateAddress(memCopy);
            RegPtr src8 = getReg8InLowByte(src);
            U32 op = 0x19000000 | src8->hardwareReg() | (addressReg->hardwareReg() << 5) | (imm << 12);
            compiler.embed_int32(op);
        } else
#endif
        {
            compiler.strb(R32(getReg8InLowByte(src)), createMem(mem));
        }
#ifdef BOXEDWINE_64
    }
    else if (width == JitWidth::b64) {
#ifdef BOXEDWINE_HOST_EXCEPTIONS
        if (emlulatedMemory && tsoMode == TSOMode::FEAT_LRCPC2 && currentOp->exceptionCount < MAX_OP_EXCEPTION_COUNT) {
            // compiler.stlur(R32(dest), createMem9(reg, disp));
            MemPtr memCopy = mem->copy();
            U32 imm = 0;

            if (memCopy->offset && (S32)memCopy->offset >= -256 && (S32)memCopy->offset <= 255) {
                imm = memCopy->offset & 0x1ff;
                memCopy->offset = 0;
            }
            RegPtr addressReg = calculateAddress(memCopy);
            U32 op = 0xD9000000 | src->hardwareReg() | (addressReg->hardwareReg() << 5) | (imm << 12);
            compiler.embed_int32(op);
        } else
#endif
        {
            compiler.str(R64(src), createMem(mem));
        }
#endif
    } else {
        kpanic_fmt("JitArmV8CodeGen::write unexpected width: %d", (U32)width);
    }
}

void JitArmV8CodeGen::writeHost(JitWidth width, MemPtr address, U32 value, bool emlulatedMemory) {
    writeHost(width, address, loadConst(value), emlulatedMemory);
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
    // Shared CPU-field guards use a null index for a direct field access.
    if (!sib) return readCPU(width, offset, std::move(reg));
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
        auto source = R32(src);
        compiler.mov(R32(getFpuWriteReg(dest)), source);
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
        compiler.mov(R32(getFpuWriteReg(dst)), (U32)imm);
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
        for (std::size_t i = 0; i < params.size(); i++) {
            pushParam(params[i], static_cast<U32>(i));
        }
    }
}

void JitArmV8CodeGen::callLoadCacheFromCPU() {
    if (cacheFlatJitBase(cpu)) {
        // The target-table base is hotter than the helper address. This helper
        // follows the native ABI and preserves the callee-saved x28 base.
        callRuntimeHelper((void*)cpu->thread->process->syncToHost);
    } else {
        compiler.blr(xLoadCacheFromCPU);
    }
}

void JitArmV8CodeGen::callRuntimeHelper(void* address) {
    compiler.mov(xBranch, (U64)address);
    compiler.blr(xBranch);
}

void JitArmV8CodeGen::callHostFunction(void* address, const std::vector<DynParam>& params, bool restoreCache, bool saveCache) {
    materializeFpuCache();
    auto saved = saveFpuTemporaries();
    if (saveCache) {
        compiler.blr(xWriteCacheToCPU);
    }
    setParams(params);    

    callRuntimeHelper(address);

    if (restoreCache) {
        callLoadCacheFromCPU();
    }
    restoreFpuTemporaries(saved);
}

void JitArmV8CodeGen::nakedCall(RegPtr reg) {
    compiler.blr(R64(reg));
}

void JitArmV8CodeGen::nakedReturn() {
    compiler.ret(asmjit::a64::x30);
}

void JitArmV8CodeGen::callHostFunctionWithResult(RegPtr result, void* address, const std::vector<DynParam>& params) {
    materializeFpuCache();
    auto saved = saveFpuTemporaries(result->hardwareReg());
    compiler.blr(xWriteCacheToCPU);

    setParams(params);

    callRuntimeHelper(address);
    compiler.mov(R64(result), xEAX);
    callLoadCacheFromCPU();
    restoreFpuTemporaries(saved);
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

void JitArmV8CodeGen::IfLessThan(JitWidth regWidth, ComparisonType type, RegPtr reg, U32 value) {
    cmp(regWidth, reg, value);

    Label label = compiler.new_label();
    ifLabels.push_back(label);
    if (type == ComparisonType::Signed) {
        compiler.b_ge(label);
    } else {
        compiler.b_hs(label);
    }
}

void JitArmV8CodeGen::IfLessThan(JitWidth regWidth, ComparisonType type, RegPtr reg1, RegPtr reg2) {
    cmp(regWidth, reg1, reg2);

    Label label = compiler.new_label();
    ifLabels.push_back(label);
    if (type == ComparisonType::Signed) {
        compiler.b_ge(label);
    } else {
        compiler.b_hs(label);
    }
}

void JitArmV8CodeGen::IfGreaterThanOrEqual(JitWidth regWidth, ComparisonType type, RegPtr reg1, RegPtr reg2) {
    cmp(regWidth, reg1, reg2);

    Label label = compiler.new_label();
    ifLabels.push_back(label);
    compiler.b_lt(label);
    if (type == ComparisonType::Signed) {
        compiler.b_lt(label);
    } else {
        compiler.b_lo(label);
    }
}

void JitArmV8CodeGen::IfGreaterThanOrEqual(JitWidth regWidth, ComparisonType type, RegPtr reg1, U32 value) {
    cmp(regWidth, reg1, value);

    Label label = compiler.new_label();
    ifLabels.push_back(label);
    if (type == ComparisonType::Signed) {
        compiler.b_lt(label);
    } else {
        compiler.b_lo(label);
    }
}

void JitArmV8CodeGen::IfGreaterThan(JitWidth regWidth, ComparisonType type, RegPtr reg1, RegPtr reg2) {
    cmp(regWidth, reg1, reg2);

    Label label = compiler.new_label();
    ifLabels.push_back(label);
    if (type == ComparisonType::Signed) {
        compiler.b_le(label);
    } else {
        compiler.b_ls(label);
    }
}

void JitArmV8CodeGen::IfGreaterThan(JitWidth regWidth, ComparisonType type, RegPtr reg1, U32 value) {
    cmp(regWidth, reg1, value);

    Label label = compiler.new_label();
    ifLabels.push_back(label);
    if (type == ComparisonType::Signed) {
        compiler.b_le(label);
    } else {
        compiler.b_ls(label);
    }
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
                    addValueWithDest(JitWidth::b32, result, getReadOnlyReg(op->sibIndex), disp);
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
                addValueWithDest(JitWidth::b32, result, getReadOnlyReg(op->rm), disp);
            }
        } else if (disp) {
            result = getTmpReg();
            if (base) {
                addValueWithDest(JitWidth::b32, result, base, disp);
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

void JitArmV8CodeGen::clearIfSpansPage(JitWidth width, RegPtr offset, RegPtr reg) {
    clearMMUPermissionIfSpansPage(width, offset, reg);
}

void JitArmV8CodeGen::JumpIfCondition(JitConditional condition, U32 address) {
    if (!isBlockOpBoundary(address)) {
        IfCondition(condition);
        branchToSideExit(address);
        EndIf();
        return;
    }
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
    materializeFpuCache();
    compiler.br(R64(reg));
}

void JitArmV8CodeGen::jmpHost(DYN_PTR_SIZE address) {
    materializeFpuCache();
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
    if (!std::getenv("BOXEDWINE_ARM_MINMAX_SELECT_SLOW")) {
        // x86 selects the right operand for unordered/equal inputs, including
        // signed zero and the exact NaN payload. FCMGT is false in those cases.
        bool leftAlias = dst->hardwareReg() == src1->hardwareReg();
        bool rightAlias = dst->hardwareReg() == src2->hardwareReg();
        SSERegPtr mask = leftAlias || rightAlias ? getTmpSSE() : dst;
        compiler.fcmgt(vMake(mask), vMake(src1), vMake(src2));
        if (leftAlias) compiler.bif(toSseB16(dst), toSseB16(src2), toSseB16(mask));
        else if (rightAlias) compiler.bit(toSseB16(dst), toSseB16(src1), toSseB16(mask));
        else compiler.bsl(toSseB16(dst), toSseB16(src1), toSseB16(src2));
        return;
    }
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
    if (!std::getenv("BOXEDWINE_ARM_MINMAX_SELECT_SLOW")) {
        // x86 selects the right operand for unordered/equal inputs, including
        // signed zero and the exact NaN payload. FCMGT is false in those cases.
        bool leftAlias = dst->hardwareReg() == src1->hardwareReg();
        bool rightAlias = dst->hardwareReg() == src2->hardwareReg();
        SSERegPtr mask = leftAlias || rightAlias ? getTmpSSE() : dst;
        compiler.fcmgt(vMake(mask), vMake(src2), vMake(src1));
        if (leftAlias) compiler.bif(toSseB16(dst), toSseB16(src2), toSseB16(mask));
        else if (rightAlias) compiler.bit(toSseB16(dst), toSseB16(src1), toSseB16(mask));
        else compiler.bsl(toSseB16(dst), toSseB16(src1), toSseB16(src2));
        return;
    }
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
        U32 roundingMode = (cpu->mxcsr >> 13) & 3;
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

void JitArmV8CodeGen::truncateScalarToInt32(RegPtr dst, SSERegPtr src, bool doublePrecision) {
    const Vec input = doublePrecision ? toSseD(src) : toSseS(src);
    static const bool useFrintts = asmjit::CpuInfo::host().features().arm().has_frintts() &&
        !std::getenv("BOXEDWINE_ARM_FRINTTS_SLOW");
    if (useFrintts) {
        // FRINT32Z produces the x86 indefinite value for NaN, infinity and
        // overflow before FCVTZS converts the integral floating-point value.
        SSERegPtr tmp = getTmpSSE();
        const Vec rounded = doublePrecision ? toSseD(tmp) : toSseS(tmp);
        compiler.frint32z(rounded, input);
        compiler.fcvtzs(R32(dst), rounded);
    } else {
        // Negative overflow already saturates to INT_MIN. Keep the converted
        // result only for an ordered value below +2^31; this also rejects NaN.
        SSERegPtr limit = loadSSEConst(doublePrecision ? SSE_MAX_INT32_PLUS_ONE_AS_DOUBLE : SSE_MAX_INT32_PLUS_ONE_AS_FLOAT);
        compiler.fcvtzs(R32(dst), input);
        compiler.fcmp(input, doublePrecision ? toSseD(limit) : toSseS(limit));
        RegPtr indefinite = loadConst(0x80000000);
        compiler.csel(R32(dst), R32(dst), R32(indefinite), asmjit::a64::CondCode::kMI);
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
    auto materialize = [&] {
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
    };
    bool canFuseMemoryCompare = !std::getenv("BOXEDWINE_ARM_SSE_MEMORY_SLOW");
    if (instructionInfo[currentOp->inst].readMemWidth) {
        // read() has already emitted the guest load. A fused backward branch
        // would insert a scheduler poll here and restart at the producer,
        // potentially observing a different memory value. Keep that case on
        // the ordinary path until the poll can be emitted before the load.
        U32 eip = currentEip + currentOp->len;
        DecodedOp* next = currentOp->next;
        for (U32 i = 0; next && i < getDirectFlagWindow(); ++i, eip += next->len, next = next->next) {
            if (next->isJumpCC() && eip + next->len + next->imm <= eip) {
                canFuseMemoryCompare = false;
                break;
            }
            if (instructionInfo[next->inst].flagsSets) break;
        }
    }
    const bool canFuseCompare = !instructionInfo[currentOp->inst].readMemWidth || canFuseMemoryCompare;
    if (canFuseCompare &&
            !std::getenv("BOXEDWINE_ARM_SSE_NATIVE_FLAGS_SLOW") &&
            !std::getenv("BOXEDWINE_ARM_SSE_COMPARE_SLOW")) {
        // FCMP directly describes x86 below/below-or-equal and unordered.
        // Keeping its native flags also makes parity available without
        // constructing EFLAGS or saving/restoring the unordered bit.
        bool fused = false;
        directFloatFlags = true;
        tryDirect(currentOp, [&] {
            compiler.fcmp(vMake(dst), vMake(src));
            fused = true;
        }, [] {}, CF | ZF | PF);
        directFloatFlags = false;
        if (fused) return;
    }
    if (canFuseCompare &&
            armJitHasFlagM2() &&
            !std::getenv("BOXEDWINE_ARM_SSE_COMPARE_SLOW")) {
        tryDirect(currentOp, [&] {
            compiler.fcmp(vMake(dst), vMake(src));
            // AXFLAG includes unordered in Z, clears N/V, and leaves C as
            // the inverse of x86 CF. Parity still needs the materialized path.
            compiler.axflag();
            cfInverted = true;
        }, materialize, CF | ZF | SF | OF);
    } else {
        materialize();
    }
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

void JitArmV8CodeGen::storeXMMToMem128(SSERegPtr reg, MemPtr address) {
    compiler.str(toSse128(reg), createMem(address));
}

void JitArmV8CodeGen::storeXMMToMem64(SSERegPtr reg, MemPtr address) {
    compiler.str(toSse64(reg), createMem(address));
}

void JitArmV8CodeGen::storeXMMToMem32(SSERegPtr reg, MemPtr address) {
    compiler.str(toSse32(reg), createMem(address));
}

void JitArmV8CodeGen::storeHighXMMToMem64(SSERegPtr reg, MemPtr address) {
    compiler.st1(toSseD2(reg, 1), createMem1(address));
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

SSERegPtr JitArmV8CodeGen::loadXMMFromMem128(U8 index, MemPtr address, SSERegPtr reg) {
    if (!reg) {
        reg = getXMM(index, false);
    }
    compiler.ldr(toSse128(reg), createMem(address));
    return reg;
}

SSERegPtr JitArmV8CodeGen::loadXMMFromMem64(U8 index, MemPtr address) {
    SSERegPtr reg = getXMM(index, false);
    compiler.ldr(toSse64(reg), createMem(address));
    return reg;
}

SSERegPtr JitArmV8CodeGen::loadLowXMMFromMem64(U8 index, MemPtr address) {
    SSERegPtr reg = getXMM(index, true);
    compiler.ld1(toSseD2(reg, 0), createMem1(address));
    return reg;
}

SSERegPtr JitArmV8CodeGen::loadHighXMMFromMem64(U8 index, MemPtr address) {
    SSERegPtr reg = getXMM(index, true);
    compiler.ld1(toSseD2(reg, 1), createMem1(address));
    return reg;
}

SSERegPtr JitArmV8CodeGen::loadXMMFromMem32(U8 index, MemPtr address) {
    SSERegPtr reg = getXMM(index, true);
    compiler.ldr(toSseS(reg), createMem(address));
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

void JitArmV8CodeGen::maskmovq(MMXRegPtr src, MMXRegPtr mask, MemPtr destAddress) {
    MMXRegPtr tmp = getTmpMMX();
    MMXRegPtr tmpMask = getTmpMMX();

    compiler.ldr(toMmxD1(tmp), createMem(destAddress));
    // spec: The most significant bit in each byte of the mask operand determines whether the corresponding byte in the source operand is written to the corresponding byte location in memory: 0 indicates no write and 1 indicates write.
    compiler.sshr(toMmxB8(tmpMask), toMmxB8(mask), 7);
    compiler.bsl(toMmxB8(tmpMask), toMmxB8(src), toMmxB8(tmp)); // if tmpMask[i] == 1 then tmpMask[i] = src[i] else tmpMaskd[i] = tmp[i]
    compiler.str(toMmxD1(tmpMask), createMem(destAddress));
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
    if (armGpScalarConversionEnabled()) compiler.scvtf(toSseS(tmp), R32(src));
    else {
        compiler.ins(toSseS4(tmp, 0), R32(src));
        compiler.scvtf(toSseS(tmp), toSseS(tmp));
    }
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
    truncateScalarToInt32(dst, src, false);
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

bool JitArmV8CodeGen::shuffleDwords(SSERegPtr dst, SSERegPtr src, U32 mask) {
    switch (mask) {
    case 0x00:
    case 0x55:
    case 0xaa:
    case 0xff:
        compiler.dup(toSseS4(dst), toSseS4(src, mask & 3));
        break;
    case 0xe4:
        if (dst->hardwareReg() != src->hardwareReg()) {
            compiler.mov(toSse128(dst), toSse128(src));
        }
        break;
    case 0xb1:
        compiler.rev64(toSseS4(dst), toSseS4(src));
        break;
    case 0x1b:
        compiler.rev64(toSseS4(dst), toSseS4(src));
        compiler.ext(toSseB16(dst), toSseB16(dst), toSseB16(dst), 8);
        break;
    case 0x39:
    case 0x4e:
    case 0x93:
        compiler.ext(toSseB16(dst), toSseB16(src), toSseB16(src), (mask & 3) * 4);
        break;
    case 0x44:
        compiler.zip1(toSseD2(dst), toSseD2(src), toSseD2(src));
        break;
    case 0xee:
        compiler.zip2(toSseD2(dst), toSseD2(src), toSseD2(src));
        break;
    case 0x88:
        compiler.uzp1(toSseS4(dst), toSseS4(src), toSseS4(src));
        break;
    case 0xdd:
        compiler.uzp2(toSseS4(dst), toSseS4(src), toSseS4(src));
        break;
    case 0x50:
        compiler.zip1(toSseS4(dst), toSseS4(src), toSseS4(src));
        break;
    case 0xfa:
        compiler.zip2(toSseS4(dst), toSseS4(src), toSseS4(src));
        break;
    case 0xa0:
        compiler.trn1(toSseS4(dst), toSseS4(src), toSseS4(src));
        break;
    case 0xf5:
        compiler.trn2(toSseS4(dst), toSseS4(src), toSseS4(src));
        break;
    default:
        return false;
    }
    return true;
}

void JitArmV8CodeGen::shufpsXmmXmm(SSERegPtr dst, SSERegPtr src, U32 mask) {
    if (dst->hardwareReg() == src->hardwareReg() && shuffleDwords(dst, src, mask)) {
        return;
    }
    // These selections preserve whole pairs or select even/odd lanes. ARM's
    // vector shuffles read both inputs before writing the destination, so the
    // same paths are also valid when the guest operands alias.
    switch (mask) {
    case 0x44:
        compiler.zip1(toSseD2(dst), toSseD2(dst), toSseD2(src));
        return;
    case 0xee:
        compiler.zip2(toSseD2(dst), toSseD2(dst), toSseD2(src));
        return;
    case 0xe4:
        compiler.ins(toSseD2(dst, 1), toSseD2(src, 1));
        return;
    case 0x4e:
        compiler.ext(toSseB16(dst), toSseB16(dst), toSseB16(src), 8);
        return;
    case 0x88:
        compiler.uzp1(toSseS4(dst), toSseS4(dst), toSseS4(src));
        return;
    case 0xdd:
        compiler.uzp2(toSseS4(dst), toSseS4(dst), toSseS4(src));
        return;
    }
    SSERegPtr tmp = getTmpSSE();
    
    // Start with a complete write: inserting lane zero would retain a false
    // dependency on the previous instruction that used this temporary.
    compiler.dup(toSseS4(tmp), toSseS4(dst, mask & 3));
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

U32 JitArmV8CodeGen::ramAccessAlignment(JitWidth width) {
#ifdef BOXEDWINE_HOST_EXCEPTIONS
    // LDAPR/STLUR can fault on unaligned accesses even to accessible RAM.
    if (width <= JitWidth::b64 && tsoMode == TSOMode::FEAT_LRCPC2 &&
            currentOp->exceptionCount < MAX_OP_EXCEPTION_COUNT) {
        return 1u << (U32)width;
    }
#endif
    return 1;
}

void JitArmV8CodeGen::comissXmmXmm(SSERegPtr dst, SSERegPtr src) {
    comis(dst, src, toSseS);
}

void JitArmV8CodeGen::ucomissXmmXmm(SSERegPtr dst, SSERegPtr src) {
    comis(dst, src, toSseS);
}

void JitArmV8CodeGen::stmxcsr(MemPtr address) {
    RegPtr tmp = getTmpReg();
    compiler.ldr(R32(tmp), createMem(regCPU, offsetof(CPU, mxcsr)));
    compiler.str(R32(tmp), createMem(address));
}

void JitArmV8CodeGen::ldmxcsr(MemPtr address) {
    RegPtr tmp = getTmpReg();
    compiler.ldr(R32(tmp), createMem(address));
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

MMXRegPtr JitArmV8CodeGen::loadMMXFromMem32(U8 index, MemPtr address) {
    MMXRegPtr tmp = getTmpMMX();
    compiler.ldr(toMmxS(tmp), createMem(address));
    return tmp;
}

MMXRegPtr JitArmV8CodeGen::loadMMXFromMem64(U8 index, MemPtr address) {
    MMXRegPtr tmp = getTmpMMX();
    compiler.ldr(toMmxD(tmp), createMem(address));
    return tmp;
}

void JitArmV8CodeGen::storeMMXToMem32(MMXRegPtr reg, MemPtr address) {
    compiler.str(toMmxS(reg), createMem(address));
}

void JitArmV8CodeGen::storeMMXToMem64(MMXRegPtr reg, MemPtr address) {
    compiler.str(toMmxD1(reg), createMem(address));
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
    if (hasZeroUpper64(dst)) {
        compiler.fadd(toSseD(dst), toSseD(dst), toSseD(src));
        return;
    }
    SSERegPtr tmp = getTmpSSE();
    compiler.fadd(toSseD(tmp), toSseD(dst), toSseD(src));
    compiler.ins(toSseD2(dst, 0), toSseD2(tmp, 0)); // need to maintain the high 64-bits
}

void JitArmV8CodeGen::subpdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.fsub(toSseD2(dst), toSseD2(dst), toSseD2(src));
}

void JitArmV8CodeGen::subsdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    if (hasZeroUpper64(dst)) {
        compiler.fsub(toSseD(dst), toSseD(dst), toSseD(src));
        return;
    }
    SSERegPtr tmp = getTmpSSE();
    compiler.fsub(toSseD(tmp), toSseD(dst), toSseD(src));
    compiler.ins(toSseD2(dst, 0), toSseD2(tmp, 0)); // need to maintain the high 64-bits
}

void JitArmV8CodeGen::mulpdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.fmul(toSseD2(dst), toSseD2(dst), toSseD2(src));
}

void JitArmV8CodeGen::mulsdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    if (hasZeroUpper64(dst)) {
        compiler.fmul(toSseD(dst), toSseD(dst), toSseD(src));
        return;
    }
    SSERegPtr tmp = getTmpSSE();
    compiler.fmul(toSseD(tmp), toSseD(dst), toSseD(src));
    compiler.ins(toSseD2(dst, 0), toSseD2(tmp, 0)); // need to maintain the high 64-bits
}

void JitArmV8CodeGen::divpdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.fdiv(toSseD2(dst), toSseD2(dst), toSseD2(src));
}

void JitArmV8CodeGen::divsdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    if (hasZeroUpper64(dst)) {
        compiler.fdiv(toSseD(dst), toSseD(dst), toSseD(src));
        return;
    }
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
    if (hasZeroUpper64(dst)) {
        compiler.fsqrt(toSseD(dst), toSseD(src));
        return;
    }
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
    if (armGpScalarConversionEnabled()) compiler.scvtf(toSseD(tmpReg), R32(src));
    else {
        compiler.ins(toSseS4(tmpReg, 0), R32(src));
        compiler.sxtl(toSseD2(tmpReg), toSseS2(tmpReg));
        compiler.scvtf(toSseD(tmpReg), toSseD(tmpReg));
    }
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
    truncateScalarToInt32(dst, src, true);
}

void JitArmV8CodeGen::movsdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    if (scalarZeroDirectEnabled && hasZeroUpper64(dst)) {
        compiler.fmov(toSseD(dst), toSseD(src));
        return;
    }
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
    // Bitwise transfer, with the upper 96 bits cleared as required by MOVD.
    compiler.fmov(toSseS(dst), R32(src));
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

void JitArmV8CodeGen::maskmovdqu(SSERegPtr src, SSERegPtr mask, MemPtr address) {
    // maskmovdqu xmm1, xmm2
    // this will mov xmm1[i] to DS:EDI[i] if (xmm2[i] & 80)
    SSERegPtr vTmpReg = getTmpSSE();
    SSERegPtr vTmpRegMask = getTmpSSE();

    Mem mem = createMem(address);
    compiler.ldr(toSse128(vTmpReg), mem);
    compiler.sshr(toSseB16(vTmpRegMask), toSseB16(mask), 7);
    compiler.bsl(toSseB16(vTmpRegMask), toSseB16(src), toSseB16(vTmpReg));
    compiler.str(toSse128(vTmpRegMask), mem);
}

void JitArmV8CodeGen::pshufdXmmXmm(SSERegPtr dst, SSERegPtr src, U32 mask) {
    if (shuffleDwords(dst, src, mask)) {
        return;
    }
    SSERegPtr tmp = dst;

    if (dst->hardwareReg() == src->hardwareReg()) {
        tmp = getTmpSSE();
    }
    compiler.dup(toSseS4(tmp), toSseS4(src, mask & 3));
    compiler.ins(toSseS4(tmp, 1), toSseS4(src, (mask >> 2) & 3));
    compiler.ins(toSseS4(tmp, 2), toSseS4(src, (mask >> 4) & 3));
    compiler.ins(toSseS4(tmp, 3), toSseS4(src, (mask >> 6) & 3));
    if (dst->hardwareReg() == src->hardwareReg()) {
        compiler.mov(toSse128(dst), toSse128(tmp));
    }
}

void JitArmV8CodeGen::shuffleWords(SSERegPtr dst, SSERegPtr src, U32 mask, bool high) {
    const bool alias = dst->hardwareReg() == src->hardwareReg();
    if (mask == 0xe4) {
        if (!alias) compiler.mov(toSse128(dst), toSse128(src));
        return;
    }
    SSERegPtr tmp = alias ? getTmpSSE() : dst;
    const U32 first = high ? 4 : 0;
    if (mask == 0xb1) {
        compiler.rev32(toSseH8(tmp), toSseH8(src));
    } else if (mask == 0x1b) {
        compiler.rev64(toSseH8(tmp), toSseH8(src));
    } else {
        // A full write avoids a false dependency on the old temporary value.
        compiler.dup(toSseH8(tmp), toSseH8(src, first + (mask & 3)));
        if (mask != 0x00 && mask != 0x55 && mask != 0xaa && mask != 0xff) {
            compiler.ins(toSseH8(tmp, first + 1), toSseH8(src, first + ((mask >> 2) & 3)));
            compiler.ins(toSseH8(tmp, first + 2), toSseH8(src, first + ((mask >> 4) & 3)));
            compiler.ins(toSseH8(tmp, first + 3), toSseH8(src, first + ((mask >> 6) & 3)));
        }
    }
    // The other half is copied from the source, including when dst aliases it.
    if (alias) {
        compiler.ins(toSseD2(dst, high ? 1 : 0), toSseD2(tmp, high ? 1 : 0));
    } else {
        compiler.ins(toSseD2(dst, high ? 0 : 1), toSseD2(src, high ? 0 : 1));
    }
}

void JitArmV8CodeGen::pshufhwXmmXmm(SSERegPtr dst, SSERegPtr src, U32 mask) {
    shuffleWords(dst, src, mask, true);
}

void JitArmV8CodeGen::pshuflwXmmXmm(SSERegPtr dst, SSERegPtr src, U32 mask) {
    shuffleWords(dst, src, mask, false);
}

void JitArmV8CodeGen::shufpdXmmXmm(SSERegPtr dst, SSERegPtr src, U32 mask) {
    // Only the low two immediate bits select the two 64-bit lanes.
    if (dst->hardwareReg() == src->hardwareReg()) {
        switch (mask & 3) {
        case 0: compiler.dup(toSseD2(dst), toSseD2(src, 0)); break;
        case 1: compiler.ext(toSseB16(dst), toSseB16(src), toSseB16(src), 8); break;
        case 2: break;
        case 3: compiler.dup(toSseD2(dst), toSseD2(src, 1)); break;
        }
    } else {
        switch (mask & 3) {
        case 0: compiler.zip1(toSseD2(dst), toSseD2(dst), toSseD2(src)); break;
        case 1: compiler.ext(toSseB16(dst), toSseB16(dst), toSseB16(src), 8); break;
        case 2: compiler.ins(toSseD2(dst, 1), toSseD2(src, 1)); break;
        case 3: compiler.zip2(toSseD2(dst), toSseD2(dst), toSseD2(src)); break;
        }
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

void JitArmV8CodeGen::clflush(MemPtr address) {
    // might need some work on this
    compiler.dc(asmjit::a64::Predicate::DC::kCVAC, R64(calculateAddress(address)));
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
    // ARM saturates FCVT, while x86 returns integer indefinite for invalid
    // conversion. Convert to 64 bits first so rounded 32-bit overflow is visible.
    RegPtr result = fpuRegToInt(fpuRegSrc, truncate, true);
    Label invalid = compiler.new_label();
    Label done = compiler.new_label();
    compiler.fcmp(toVec(fpuRegSrc), toVec(fpuRegSrc));
    compiler.b_vs(invalid); // NaN converts to zero on ARM.
    compiler.sxtw(xScratch, R32(result));
    compiler.cmp(R64(result), xScratch);
    compiler.b_eq(done);
    compiler.bind(invalid);
    compiler.mov(R32(result), 0x80000000u);
    compiler.bind(done);
    return result;
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

void JitArmV8CodeGen::storeFPUToInt64(FPURegPtr src, MemPtr address, bool truncate) {
    RegPtr result = fpuRegToInt(src, truncate, true);
    writeHost(JitWidth::b64, address, result);
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

void JitArmV8CodeGen::storeFpuReg(FPURegPtr reg, MemPtr address, DynFpuWidth width) {
    if (width == DYN_FPU_64_BIT) {
        compiler.str(toVec(reg), createMem(address));
    } else {
        compiler.str(toVec32(reg), createMem(address));
    }
}

void JitArmV8CodeGen::loadFpuReg(FPURegPtr reg, MemPtr address, DynFpuWidth width) {
    if (width == DYN_FPU_64_BIT) {
        compiler.ldr(toVec(reg), createMem(address));
    } else {
        compiler.ldr(toVec32(reg), createMem(address));
    }
}

void JitArmV8CodeGen::fpuRegExtend32To64(FPURegPtr dst, FPURegPtr src) {
    compiler.fcvt(toVec(dst), toVec32(src));
}

void JitArmV8CodeGen::fpuReg64To32(FPURegPtr dst, FPURegPtr src) {
    compiler.fcvt(toVec32(dst), toVec(src));
    //compiler.fcvtn(Vec::make_v64_with_element_type(VecElementType::kS, dst->hardwareReg()), Vec::make_v128_with_element_type(VecElementType::kD, src->hardwareReg()));
}

void JitArmV8CodeGen::loadFpuRegFromInt(FPURegPtr reg, MemPtr address) {
    RegPtr tmp = getTmpReg();
    readHost(JitWidth::b32, address, tmp);
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

void JitArmV8CodeGen::fpuAbs(FPURegPtr dst) {
    compiler.fabs(toVec(dst), toVec(dst));
}

void JitArmV8CodeGen::fpuNeg(FPURegPtr dst) {
    compiler.fneg(toVec(dst), toVec(dst));
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
    auto error = code.resolve_cross_section_fixups();
    if (error != asmjit::Error::kOk) kpanic("ARM JIT cross-section fixups failed");
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
    auto customMemoryOp = [op, this](MemPtr address) {
        U32 neededFlags = currentOp->needsToSetFlags(cpu) & ZF;
        if (neededFlags) {
            if (currentOp->getNeededFlagsAfter(PF | SF | AF | CF | OF)) {
                // CMPXCHG8B changes only ZF; preserve other live flags.
                fillFlags();
            } else {
                // ZF is written directly below. A later branch/SETZ must not
                // derive it from the lazy arithmetic result that preceded CAS.
                storeLazyFlagType(FLAGS_NONE);
                currentLazyFlags = FLAGS_NONE;
            }
        }
        RegPtr addressReg = calculateAddress(address);
        RegPtr offsetReg = getTmpReg();
        compiler.and_(R64(offsetReg), R64(addressReg), 7);
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
                compiler.ccmp(wEDX, wTmp8, 0, asmjit::a64::CondCode::kEQ); // if EAX matched, compare EDX; otherwise force Z clear
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
            compiler.ccmp(wEDX, R32(tmp2), 0, asmjit::a64::CondCode::kEQ); // if EAX matched, compare EDX; otherwise force Z clear

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
    };
    RegPtr address = calculateEaa(op);
    // CASP is a single restartable memory operation. The load/store-exclusive
    // fallback can skip its store on a mismatch, so it still needs a precheck.
    if (rt.cpu_features().has(asmjit::CpuFeatures::ARM::kLRCPC)) {
        JitCodeGen::write(JitWidth::b64, std::move(address), nullptr, std::move(customMemoryOp));
    } else {
        writeWithMmuCheck(JitWidth::b64, std::move(address), nullptr, std::move(customMemoryOp));
    }
}

void JitArmV8CodeGen::dynamic_cmpxchg_lock(JitWidth width, DecodedOp* op) {
    auto customMemoryOp = [width, op, this](MemPtr mem) {
        LazyFlagType flagType;
        
        if (width == JitWidth::b32) {
            flagType = FLAGS_CMP32;
        } else if (width == JitWidth::b16) {
            flagType = FLAGS_CMP16;
        } else if (width == JitWidth::b8) {
            flagType = FLAGS_CMP8;
        } else {
            kpanic("JitArmV8CodeGen::dynamic_cmpxchg_lock");
            flagType = FLAGS_NONE;
        }
                
        RegPtr tmp = getTmpReg();
        RegPtr tmp2 = getTmpReg();
        RegPtr reg = getReadOnlyRegInLower(width, op->reg);
        RegPtr eax = getReg(0);


        RegPtr address = calculateAddress(mem);

        RegPtr tmpEax = eax;
        bool needToCopyEaxBack = false;        
        JitFlags jitFlags(this, cpu, op, flagType, eax, nullptr, false);
        jitFlags.copyDest(eax);

        if (width != JitWidth::b32 || (jitFlags.flags && (jitFlags.flags->usesSrc(jitFlags.needsToSetFlags) || jitFlags.flags->usesResult(jitFlags.needsToSetFlags)))) {
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
            if (jitFlags.flags && jitFlags.flags->usesResult(jitFlags.needsToSetFlags)) {
                compiler.sub(R32(tmp2), wEAX, R32(tmpEax));
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
            if (jitFlags.flags && jitFlags.flags->usesResult(jitFlags.needsToSetFlags)) {
                compiler.sub(R32(tmp2), R32(tmpEax), wEAX);
                storeLazyFlagsResult(tmp2);
            }            
        }
        jitFlags.commit(tmp2);
    };
    RegPtr address = calculateEaa(op);
    // LSE CAS is restartable. The load/store-exclusive fallback can skip its
    // store on a mismatch, so it still needs write permission checked first.
    if (rt.cpu_features().has(asmjit::CpuFeatures::ARM::kLSE)) {
        JitCodeGen::write(width, std::move(address), nullptr, std::move(customMemoryOp));
    } else {
        writeWithMmuCheck(width, std::move(address), nullptr, std::move(customMemoryOp));
    }
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
    JitCodeGen::write(JitWidth::b32, calculateEaa(op), nullptr, [op, this](MemPtr mem) {
        RegPtr address = calculateAddress(mem);
        if (rt.cpu_features().has(asmjit::CpuFeatures::ARM::kLSE)) {            
            RegPtr reg = getReg(op->reg);
            compiler.swpal(R32(reg), R32(reg), Mem(R64(address)));
        } else {
            RegPtr reg = getReg(op->reg);
            RegPtr oldValue = getTmpReg();
            RegPtr cond = getTmpReg();
            Label label = compiler.new_label();

            compiler.bind(label);            

            // Do not update the guest register until the store succeeds.
            compiler.ldaxr(R32(oldValue), Mem(R64(address)));
            compiler.stlxr(R32(cond), R32(reg), Mem(R64(address)));
            compiler.cbnz(R32(cond), label);
            compiler.mov(R32(reg), R32(oldValue));
        }
    });
}

void JitArmV8CodeGen::dynamic_xchge16r16_lock(DecodedOp* op) {
    JitCodeGen::write(JitWidth::b16, calculateEaa(op), nullptr, [op, this](MemPtr mem) {
        RegPtr address = calculateAddress(mem);
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
    JitCodeGen::write(JitWidth::b8, calculateEaa(op), nullptr, [op, this](MemPtr mem) {
        RegPtr address = calculateAddress(mem);
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
    JitCodeGen::write(width, calculateEaa(op), nullptr, [hasSrc, immSrc, addCF, atomicCallback, writebackReg, width, flagsType, op, callback, this](MemPtr mem) {
        RegPtr address = calculateAddress(std::move(mem));        
        U32 needsToSetFlags = op->needsToSetFlags(cpu);
        const LazyFlags* flags = lazyFlags[flagsType];
        bool direct = width == JitWidth::b32 && !(flags && (flags->usesResult(needsToSetFlags) || flags->usesDst(needsToSetFlags)));
        RegPtr reg;
        
        if (hasSrc) {
            reg = immSrc ? loadConst(op->imm) : direct ? getReg(width, op->reg) : getReadOnlyRegInLower(width, op->reg);
        }
        JitFlags flagData(this, cpu, op, flagsType, reg, nullptr, writebackReg || (addCF && reg->emulatedReg == 0xff));
        RegPtr cf = flagData.oldCF;

        RegPtr dst = getTmpReg();
        RegPtr result = getTmpReg();

        if (addCF) {
            if (!cf) {
                cf = getCF();
            }
            reg = getTmpIfNotTmp(reg);
            addReg(width, reg, cf);
        }
#ifdef BOXEDWINE_HOST_EXCEPTIONS
        if (atomicCallback && rt.cpu_features().has(asmjit::CpuFeatures::ARM::kLSE)) {
            atomicCallback(reg, dst, address);
            if (flags && flags->usesResult(needsToSetFlags)) {
                callback(result, dst, reg);
            }
            if (writebackReg) {
                reg = getReg(width, op->reg);
                mov(width, reg, dst);
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
        flagData.commit(result, dst);
        // this is a bit backwards for neg flags, perhaps should switch to using dst instead of src for neg flags
        if (!hasSrc && flags && flags->usesSrc(needsToSetFlags)) {
            storeLazyFlagsSrc(dst);
        }
    });
}

void JitArmV8CodeGen::dynamic_arith_value_lock(JitWidth width, DecodedOp* op, U32 value, LazyFlagType flagsType, std::function<void(RegPtr src, RegPtr dst, RegPtr address)> atomicCallback, std::function<void(RegPtr result, RegPtr dst, U32 src)> callback) {
    JitCodeGen::write(width, calculateEaa(op), nullptr, [value, atomicCallback, width, flagsType, op, callback, this](MemPtr mem) {
        RegPtr address = calculateAddress(mem);
        const LazyFlags* flags = lazyFlags[flagsType];        
        JitFlags flagData(this, cpu, op, flagsType, value, nullptr);

        RegPtr dst = getTmpReg();
        RegPtr result = getTmpReg();
        
#ifdef BOXEDWINE_HOST_EXCEPTIONS
        if (rt.cpu_features().has(asmjit::CpuFeatures::ARM::kLSE)) {
            RegPtr reg = loadConst(value);
            atomicCallback(reg, dst, address);
            if (flags && flags->usesResult(flagData.needsToSetFlags)) {
                callback(result, dst, value);
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
        flagData.commit(result, dst);
    });
}
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

    // The OF and CF flags are set to 0. The SF, ZF, and PF flags are set according to the result(see the "Operation" section above).The state of the AF flag is undefined.
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

void JitArmV8CodeGen::direct_flags_op(JitWidth width, JitFlagOp op, RegPtr dst, RegPtr src) {
    if (width != JitWidth::b32) {
        kpanic("JitArmV8CodeGen::direct_flags_op width");
    }
    switch (op) {
    case JitFlagOp::Add:
        cfInverted = false;
        compiler.adds(R32(dst), R32(dst), R32(src));
        break;
    case JitFlagOp::Sub:
        cfInverted = true;
        compiler.subs(R32(dst), R32(dst), R32(src));
        break;
    case JitFlagOp::And:
        cfInverted = false;
        compiler.ands(R32(dst), R32(dst), R32(src));
        break;
    case JitFlagOp::Or:
        cfInverted = false;
        compiler.orr(R32(dst), R32(dst), R32(src));
        compiler.ands(asmjit::a64::wzr, R32(dst), R32(dst));
        break;
    case JitFlagOp::Xor:
        cfInverted = false;
        compiler.eor(R32(dst), R32(dst), R32(src));
        compiler.ands(asmjit::a64::wzr, R32(dst), R32(dst));
        break;
    default:
        kpanic("JitArmV8CodeGen::direct_flags_op");
    }
}

void JitArmV8CodeGen::direct_flags_op(JitWidth width, JitFlagOp op, RegPtr dst, U32 src) {
    if (width != JitWidth::b32) {
        kpanic("JitArmV8CodeGen::direct_flags_op width");
    }
    if ((op == JitFlagOp::Add || op == JitFlagOp::Sub) && !asmjit::a64::Utils::is_add_sub_imm(src)) {
        direct_flags_op(width, op, dst, loadConst(src));
        return;
    }
    if ((op == JitFlagOp::And || op == JitFlagOp::Or || op == JitFlagOp::Xor) &&
        !asmjit::a64::Utils::is_logical_imm(src, 32)) {
        direct_flags_op(width, op, dst, loadConst(src));
        return;
    }
    switch (op) {
    case JitFlagOp::Add:
        cfInverted = false;
        compiler.adds(R32(dst), R32(dst), src);
        break;
    case JitFlagOp::Sub:
        cfInverted = true;
        compiler.subs(R32(dst), R32(dst), src);
        break;
    case JitFlagOp::And:
        cfInverted = false;
        compiler.ands(R32(dst), R32(dst), src);
        break;
    case JitFlagOp::Or:
        cfInverted = false;
        compiler.orr(R32(dst), R32(dst), src);
        compiler.ands(asmjit::a64::wzr, R32(dst), R32(dst));
        break;
    case JitFlagOp::Xor:
        cfInverted = false;
        compiler.eor(R32(dst), R32(dst), src);
        compiler.ands(asmjit::a64::wzr, R32(dst), R32(dst));
        break;
    default:
        kpanic("JitArmV8CodeGen::direct_flags_op");
    }
}

void JitArmV8CodeGen::direct_neg(JitWidth width, RegPtr dst) {
    if (width != JitWidth::b32) {
        kpanic("JitArmV8CodeGen::direct_neg width");
    }
    cfInverted = true;
    compiler.subs(R32(dst), asmjit::a64::wzr, R32(dst));
}

void JitArmV8CodeGen::direct_flags_op_with_cf(JitWidth width, JitCarryOp op, RegPtr dst, RegPtr src, RegPtr cf) {
    if (width != JitWidth::b32) {
        kpanic("JitArmV8CodeGen::direct_flags_op_with_cf width");
    }
    if (op == JitCarryOp::Adc) {
        compiler.cmp(R32(cf), 1);
        compiler.adcs(R32(dst), R32(dst), R32(src));
        cfInverted = false;
    } else {
        compiler.cmp(asmjit::a64::wzr, R32(cf));
        compiler.sbcs(R32(dst), R32(dst), R32(src));
        cfInverted = true;
    }
}

void JitArmV8CodeGen::direct_flags_op_with_cf(JitWidth width, JitCarryOp op, RegPtr dst, U32 src, RegPtr cf) {
    direct_flags_op_with_cf(width, op, dst, loadConst(src), cf);
}

void JitArmV8CodeGen::onBlockPreCommit(DecodedOp* op) {
    if (pendingLabels.size()) {
        kpanic("JitX86CodeGen::onBlockPreCommit");
    }
}

bool JitArmV8CodeGen::supportsDirectCondition(JitConditional condition) {
    if (directMultiplyFlags) {
        return condition == JitConditional::O || condition == JitConditional::NO ||
            condition == JitConditional::B || condition == JitConditional::NB;
    }
    if (directIntegerParity) {
        return condition != JitConditional::O && condition != JitConditional::NO &&
            condition != JitConditional::L && condition != JitConditional::NL &&
            condition != JitConditional::LE && condition != JitConditional::NLE;
    }
    if (directFloatFlags) {
        return condition == JitConditional::B || condition == JitConditional::NB ||
            condition == JitConditional::BE || condition == JitConditional::NBE ||
            condition == JitConditional::P || condition == JitConditional::NP;
    }
    if (condition == JitConditional::P || condition == JitConditional::NP) {
        return false;
    }
    return true;
}

asmjit::a64::CondCode JitArmV8CodeGen::getCondCode(JitConditional condition) {
    if (directMultiplyFlags) {
        if (condition == JitConditional::O || condition == JitConditional::B) return asmjit::a64::CondCode::kNE;
        if (condition == JitConditional::NO || condition == JitConditional::NB) return asmjit::a64::CondCode::kEQ;
        kpanic("Unsupported direct multiply condition");
    }
    if (directIntegerParity) {
        if (directParityValue && (condition == JitConditional::P || condition == JitConditional::NP)) {
            // Only parity conditions need V. The table value is even parity;
            // N/Z/C remain available for the other consumers in this region.
            compiler.embed_uint32(0xba000401u | (R64(directParityValue).id() << 5));
            return condition == JitConditional::P ? asmjit::a64::CondCode::kVS : asmjit::a64::CondCode::kVC;
        }
        if (condition == JitConditional::P) return asmjit::a64::CondCode::kVC;
        if (condition == JitConditional::NP) return asmjit::a64::CondCode::kVS;
    }
    if (directFloatFlags) {
        switch (condition) {
        case JitConditional::B: return asmjit::a64::CondCode::kLT;
        case JitConditional::NB: return asmjit::a64::CondCode::kGE;
        case JitConditional::BE: return asmjit::a64::CondCode::kLE;
        case JitConditional::NBE: return asmjit::a64::CondCode::kGT;
        case JitConditional::P: return asmjit::a64::CondCode::kVS;
        case JitConditional::NP: return asmjit::a64::CondCode::kVC;
        default: kpanic("Unsupported direct floating-point condition");
        }
    }
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
            cfInverted = true;
        }
        return asmjit::a64::CondCode::kLS;
    case JitConditional::NBE: 
        if (!cfInverted) {
            compiler.cfinv();
            cfInverted = true;
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
    const bool parityValueBranch = directIntegerParity && directParityValue &&
        (condition == JitConditional::P || condition == JitConditional::NP) &&
        std::getenv("BOXEDWINE_ARM_PARITY_CBZ");
    auto emitBranch = [&](Label label, bool inverse) {
        if (parityValueBranch) {
            // The retained value is 0/1, so CBZ/CBNZ can consume it directly.
            // Both have the same signed 19-bit displacement as B.cond; this
            // does not introduce TBZ's shorter branch-range restriction.
            if ((condition == JitConditional::P) != inverse) compiler.cbnz(R32(directParityValue), label);
            else compiler.cbz(R32(directParityValue), label);
        } else {
            auto nativeCondition = getCondCode(condition);
            compiler.b(inverse ? asmjit::arm::negate_cond(nativeCondition) : nativeCondition, label);
        }
    };
    if (!isBlockOpBoundary(address)) {
        Label fallthrough = compiler.new_label();
        emitBranch(fallthrough, true);
        branchToSideExit(address);
        compiler.bind(fallthrough);
        return;
    }
    Label label;

    if (!opLabels.get(address, label)) {
        label = compiler.new_label();
        opLabels.set(address, label);
        pendingLabels.set(address, label);
    }

    emitBranch(label, false);
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
    if (directIntegerParity && directParityValue &&
            (condition == JitConditional::P || condition == JitConditional::NP)) {
        RegPtr value = directParityValue;
        if (condition == JitConditional::NP) {
            value = getTmpReg();
            compiler.eor(R32(value), R32(directParityValue), 1);
        }
        compiler.bfi(R32(dst), R32(value), dst->isHigh ? 8 : 0, 8);
        return;
    }
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

U8* JitArmV8CodeGen::createBlockExit() {
    compiler.blr(xWriteCacheToCPU);

    if (supportsFpuStackCache()) {
        compiler.add(xScratch, xCPU, offsetof(CPU, armSavedFpuRegs));
        for (U8 i = 0; i < 8; i += 2)
            compiler.ldp(Vec::make_d(8 + i), Vec::make_d(9 + i), Mem(xScratch, i * 8));
    }
    compiler.mov(xTmp7, xCPU);
    for (int i = 19; i < 31; i += 2) {
        compiler.ldp(R64(i), R64(i + 1), Mem(xTmp7, offsetof(CPU, storedRegs) + (i - 19) * 8));
    }
    compiler.ret(asmjit::a64::x30);
    return createDynamicExecutableMemory();
}

void JitArmV8CodeGen::blockExit() {
    materializeFpuCache();
    compiler.mov(xBranch, cpu->thread->process->blockExit);
    compiler.br(xBranch);
}

U8* JitArmV8CodeGen::createStartJITCode() {
    for (int i = 19; i < 31; i+=2) {
        compiler.stp(R64(i), R64(i+1), Mem(xEAX, offsetof(CPU, storedRegs) + (i - 19) * 8));
    }
    if (supportsFpuStackCache()) {
        compiler.add(xScratch, xEAX, offsetof(CPU, armSavedFpuRegs));
        for (U8 i = 0; i < 8; i += 2)
            compiler.stp(Vec::make_d(8 + i), Vec::make_d(9 + i), Mem(xScratch, i * 8));
    }
    compiler.mov(R64(29), asmjit::a64::sp); // mov fp, sp
    compiler.mov(xCPU, R64(0));
    compiler.ldr(xBranch, Mem(R64(1), offsetof(DecodedOp, pfnJitCode)));
    compiler.mov(xMMU, (U64)&getMemData(KThread::currentThread()->memory)->mmu);
#ifdef BOXEDWINE_MEM_CACHE
    static_assert(offsetof(KMemoryData, mmu) + sizeof(void*) * K_NUMBER_OF_PAGES == offsetof(KMemoryData, readCache));
    static_assert(offsetof(KMemoryData, mmu) + sizeof(void*) * K_NUMBER_OF_PAGES * 2 == offsetof(KMemoryData, writeCache));
#endif
    if (cacheFlatJitBase(cpu)) compiler.ldr(xFlatJitBase, Mem(xCPU, offsetof(CPU, jitFlatEntries)));
    else compiler.mov(xLoadCacheFromCPU, (U64)KThread::currentThread()->process->syncToHost);
    compiler.mov(xWriteCacheToCPU, (U64)KThread::currentThread()->process->syncFromHost);
    if (cacheLinearMemoryBase()) {
        constexpr S32 offset = (S32)offsetof(KMemoryData, linearMemoryBase) - (S32)offsetof(KMemoryData, mmu);
        compiler.ldr(xLinearMemoryBase, Mem(xMMU, offset));
    } else {
        compiler.mov(xEmulateSingleOp, (DYN_PTR_SIZE)cpu->thread->process->emulateSingleOp);
    }
    loadCacheFromCPU();
    If(JitWidth::b64, std::make_shared<JitReg>(regBranch, 0xff)); {
        // A reset CPU can enter another CPU's compiled code directly or via
        // an interpreter tail. The host call uses xBranch, so preserve the
        // selected entry explicitly and keep the native stack aligned.
        IfNotCPU(DYN_PTR, nullptr, 0, offsetof(CPU, calculateCF)); {
            compiler.sub(asmjit::a64::sp, asmjit::a64::sp, 16);
            compiler.str(xBranch, Mem(asmjit::a64::sp));
            callHostFunction((void*)bindNativeCarryHelpers, {DynParam(JitCallParamType::CPU)});
            compiler.ldr(xBranch, Mem(asmjit::a64::sp));
            compiler.add(asmjit::a64::sp, asmjit::a64::sp, 16);
        } EndIf();
        compiler.blr(xBranch);
    } StartElse(); {
        // Invalidation can clear the entry after an interpreter tail call has
        // selected this wrapper. nextOp may still be that chain's old head;
        // refetch at the current EIP when returning to the run loop.
        writeCPUValue(DYN_PTR, offsetof(CPU, nextOp), 0);
    } EndIf();
    blockExit();
    return createDynamicExecutableMemory();
}

void JitArmV8CodeGen::loadCacheFromCPU() {
    static const bool pairLoads = !std::getenv("BOXEDWINE_ARM_SYNC_PAIRS_SLOW");
    for (int i = 0; i < 8; i++) {
        if (pairLoads && i + 1 < 8 &&
                regCache[i] != INVALID_REG && regCache[i + 1] != INVALID_REG) {
            compiler.ldp(R32(regCache[i]), R32(regCache[i + 1]),
                Mem(xCPU, (U32)(offsetof(CPU, reg[0].u32) + sizeof(U32) * i)));
            ++i;
        } else if (regCache[i] != INVALID_REG) {
            compiler.ldr(R32(regCache[i]), Mem(xCPU, (U32)(offsetof(CPU, reg[0].u32) + sizeof(U32) * i)));
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
    static const bool pairStores = !std::getenv("BOXEDWINE_ARM_SYNC_PAIRS_SLOW");
    for (int i = 0; i < 8; i++) {
        if (pairStores && i + 1 < 8 &&
                regCache[i] != INVALID_REG && regCache[i + 1] != INVALID_REG) {
            compiler.stp(R32(regCache[i]), R32(regCache[i + 1]),
                Mem(xCPU, (U32)(offsetof(CPU, reg[0].u32) + sizeof(U32) * i)));
            ++i;
        } else if (regCache[i] != INVALID_REG) {
            compiler.str(R32(regCache[i]), Mem(xCPU, (U32)(offsetof(CPU, reg[0].u32) + sizeof(U32) * i)));
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

#ifdef _WIN32
static void clearArmJitBlockEntry(void* entry) noexcept {
    ::memset(entry, 0, 4);
}
#else
static void clearArmJitBlockEntries(void* opaque) noexcept {
    const std::vector<void*>& jitOps = *static_cast<const std::vector<void*>*>(opaque);
    for (void* p : jitOps) {
        ::memset(p, 0, 4);
    }
}
#endif

void clearJitBlock(const std::vector<void*>& jitOps) {
    if (jitOps.empty()) return;
#ifdef _WIN32
    // Windows changes protection only for the supplied address range. Each
    // disjoint entry therefore needs its own protected write and cache flush.
    for (void* entry : jitOps) {
        Platform::writeCodeToMemory(entry, 4, clearArmJitBlockEntry, entry);
    }
#else
    // Guest-address order does not imply native-address order, and entries
    // can belong to disjoint executable allocations. A first-to-last range
    // can underflow or cross unmapped memory during cache maintenance.
    // Write all traps in one JIT write window, then flush only written words.
    Platform::writeCodeToMemory(jitOps.front(), 4, clearArmJitBlockEntries,
        const_cast<std::vector<void*>*>(&jitOps));
    for (size_t i = 1; i < jitOps.size(); ++i) {
        Platform::clearInstructionCache(jitOps[i], 4);
    }
#endif
}

#endif
