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
#if defined(BOXEDWINE_JIT_X86) || defined(BOXEDWINE_JIT_X64)

#include "jitX86CodeGen.h"
#include "../jit/jitSSE.h"
#include <array>

#undef u8
#undef h8
#undef h16

#include <asmjit/core.h>
#include <asmjit/x86.h>

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
#define NUMBER_OF_TMPS 5
#ifdef BOXEDWINE_64

#if 0
// this is a tiny bit faster, but there is a bug, ff installer crashes
// 38.2 vs 37.8 fps for Quake 2
static U8 regCache[] = { 8, 9, 11, 12, 10, 5, 6, 7 };
static bool isTmp[] = { true, true, true, true, false, false, false, false, false, false, false, false, false, false, true, false };
static U8 tmps[] = { 14, 3, 2, 1, 0 };
// should be a volitile tmp reg
#define PARAM_CALL_TMP compiler.rdx
#else
U8 regCache[] = { 0, 1, 2, 3, 10, 5, 6, 7 };
U8 xmmCache[] = { 0, 1, 2, 3, 4, 5, 6, 7 };
static bool isTmp[] = { false, false, false, false, false, false, false, false, true, true, false, true, true, false, true, false };
static U8 tmps[] = { 14, 12, 11, 9, 8 };
static U8 XMMtmps[] = { 12, 13, 14, 15 };
// should be a volitile tmp reg
#define PARAM_CALL_TMP asmjit::x86::r11
#endif
#define HOST_MMU asmjit::x86::r15
#define HOST_CPU asmjit::x86::r13
#define NUMBER_OF_REGS 16
#define NUMBER_OF_XMM_REG 16
#define NUMBER_OF_XMM_TMPS 4
#define RN(x) R64(x)

#ifdef _WIN32
// RBX, RBP, RSP, RDI, RSI, R12, R13, R14, and R15 are non volitile
static bool isVolitile[] = { true, true, true, false, false, false, false, false, true, true, true, true, false, false, false, false };
#else
// RBX, RBP, RSP, and R12–R15 are non volitile
static bool isVolitile[] = { true, true, true, false, false, false, true, true, true, true, true, true, false, false, false, false };
#endif

#else
U8 regCache[] = { 5, INVALID_REG, INVALID_REG, INVALID_REG, INVALID_REG, INVALID_REG, INVALID_REG, INVALID_REG };
U8 xmmCache[] = { INVALID_REG, INVALID_REG, INVALID_REG, INVALID_REG, INVALID_REG, INVALID_REG, INVALID_REG, INVALID_REG };
static bool isVolitile[] = { true, true, true, false, false, false, false, false };
static bool isTmp[] = { true, true, true, true, false, false, true, false };
static U8 tmps[] = { 6, 3, 2, 1, 0 }; // if changed, sync up with getConditionCalculationReg
static U8 XMMtmps[] = { 4, 5, 6, 7 };
#define HOST_CPU asmjit::x86::edi
#define NUMBER_OF_REGS 8
#define NUMBER_OF_XMM_REG 8
#define NUMBER_OF_XMM_TMPS 4
#define RN(x) R32(x)
#endif

#define regAl asmjit::x86::al
#define regAx asmjit::x86::ax
#define regEax asmjit::x86::eax
#define regCl asmjit::x86::cl
#define regCx asmjit::x86::cx
#define regEcx asmjit::x86::ecx
#define regDx asmjit::x86::dx
#define regEdx asmjit::x86::edx
#define regEbx asmjit::x86::ebx
#define regEsp asmjit::x86::esp
#define regEbp asmjit::x86::ebp
#define regEsi asmjit::x86::esi
#define regEdi asmjit::x86::edi

typedef asmjit::x86::Mem Mem;
#define Mem8 asmjit::x86::byte_ptr
#define Mem16 asmjit::x86::word_ptr
#define Mem32 asmjit::x86::dword_ptr
#define Mem64 asmjit::x86::qword_ptr
#define Mem128 asmjit::x86::dqword_ptr

typedef asmjit::Label Label;
typedef asmjit::x86::Vec Vec;

asmjit::x86::Gp R64(RegPtr reg) {
    return asmjit::x86::gp64(reg->hardwareReg());
}

asmjit::x86::Gp R64(U8 reg) {
    return asmjit::x86::gp64(reg);
}

asmjit::x86::Gp R32(RegPtr reg) {
    return asmjit::x86::gp32(reg->hardwareReg());
}

asmjit::x86::Gp R32(U8 reg) {
    return asmjit::x86::gp32(reg);
}

asmjit::x86::Gp R16(RegPtr reg) {
    return asmjit::x86::gp16(reg->hardwareReg());
}

asmjit::x86::Gp R16(U8 reg) {
    return asmjit::x86::gp16(reg);
}

asmjit::x86::Gp R8(RegPtr reg) {
#ifndef BOXEDWINE_64
    if (reg->hardwareReg() >= 4) {
        return asmjit::x86::gp8_hi(reg->hardwareReg() - 4);
    }
#endif
    return asmjit::x86::gp8_lo(reg->hardwareReg());
}

asmjit::x86::Gp R8(U8 reg) {
#ifndef BOXEDWINE_64
    if (reg >= 4) {
        return asmjit::x86::gp8_hi(reg - 4);        
    }    
#endif
    return asmjit::x86::gp8_lo(reg);
}

/*
asmjit::x86::Gp R(JitWidth width, U8 reg) {
    if (width == JitWidth::b32) {
        return R32(reg);
    } else if (width == JitWidth::b16) {
        return R16(reg);
    } else if (width == JitWidth::b8) {
        return R8(reg);
#ifdef BOXEDWINE_64
    } else if (width == JitWidth::b64) {
        return R64(reg);
#endif
    } else {
        kpanic("jitX86CodeGen::R");
        return R8(reg);
    }
}
*/

asmjit::x86::Gp R(JitWidth width, RegPtr reg) {
    if (width == JitWidth::b32) {
        return R32(reg);
    } else if (width == JitWidth::b16) {
        return R16(reg);
    } else if (width == JitWidth::b8) {
        return R8(reg);
#ifdef BOXEDWINE_64
    } else if (width == JitWidth::b64) {
        return R64(reg);
#endif
    } else {
        kpanic("jitX86CodeGen::R");
        return R8(reg);
    }
}

asmjit::x86::Vec XMM(U8 reg) {
    return asmjit::x86::xmm(reg);
}

class JitX86CodeGen : public JitSSE, asmjit::ErrorHandler {
public:    
    void handle_error(asmjit::Error err, const char* message, asmjit::BaseEmitter* origin) override {
        kpanic(message);
    }

    JitX86CodeGen(CPU* cpu) : JitSSE(cpu) {
#ifdef BOXEDWINE_64
#ifdef BOXEDWINE_MSVC
        params[0] = asmjit::x86::rcx;
        params[1] = asmjit::x86::rdx;
        params[2] = asmjit::x86::r8;
        params[3] = asmjit::x86::r9;
#else
        params[0] = asmjit::x86::rdi;
        params[1] = asmjit::x86::rsi;
        params[2] = asmjit::x86::rdx;
        params[3] = asmjit::x86::rcx;
#endif
#endif
        code.init(rt.environment());
        code.attach(&compiler);
        code.set_error_handler(this);
    }

    void preOp(DecodedOp* op) override;
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
    RegPtr getConditionCalculationReg(U32 index) override;
    RegPtr getStringRegEcx() override;
    RegPtr getStringRegEsi() override;
    RegPtr getStringRegEdi() override;

    void writeEip(RegPtr eip) override;
    void writeEip(U32 eip) override;
    bool isTmpRegAvailable() override;    
    void forceSyncBackIfNotCached(RegPtr reg) override;
    
    U8 findTmpReg(bool needs8bitReg, S8 hint = -1, bool allowInvalidReturn = false);
    void emulateSingleOp() override;

    void addReg(JitWidth regWidth, RegPtr reg, RegPtr rm) override;
    void addValue(JitWidth regWidth, RegPtr reg, U32 imm) override;
    void orReg(JitWidth regWidth, RegPtr reg, RegPtr rm) override;
    void orValue(JitWidth regWidth, RegPtr reg, U32 imm) override;
    void subReg(JitWidth regWidth, RegPtr reg, RegPtr rm) override;
    void subValue(JitWidth regWidth, RegPtr reg, U32 imm) override;
    void andReg(JitWidth regWidth, RegPtr reg, RegPtr rm) override;
    void andValue(JitWidth regWidth, RegPtr reg, U32 immm) override;
#ifdef BOXEDWINE_64
    void andValue64(RegPtr reg, U64 immm) override;
#endif
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
    void rclReg(JitWidth regWidth, RegPtr reg, RegPtr rm, RegPtr cf) override;
    void rclValue(JitWidth regWidth, RegPtr reg, U32 imm, RegPtr cf) override;
    void rcrReg(JitWidth regWidth, RegPtr reg, RegPtr rm, RegPtr cf) override;
    void rcrValue(JitWidth regWidth, RegPtr reg, U32 imm, RegPtr cf) override;
    void shldReg(JitWidth regWidth, RegPtr reg, RegPtr rm, RegPtr cl) override;
    void shldValue(JitWidth regWidth, RegPtr reg, RegPtr rm, U32 imm) override;
    void shrdReg(JitWidth regWidth, RegPtr reg, RegPtr rm, RegPtr cl) override;
    void shrdValue(JitWidth regWidth, RegPtr reg, RegPtr rm, U32 imm) override;    
    void xchgReg(JitWidth regWidth, RegPtr dest, RegPtr src) override;
    void xaddReg(JitWidth regWidth, RegPtr reg, RegPtr rm) override;
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
    RegPtr testZeroReg(JitWidth regWidth, RegPtr reg, RegPtr result = nullptr) override;

    void readMMU(RegPtr dest, RegPtr index, U32 offset = 0) override;
    void readMMU(RegPtr dest, U32 index) override;
    void readHost(JitWidth width, MemPtr address, RegPtr result, bool emlulatedMemory = true) override;
    void writeHost(JitWidth width, MemPtr address, RegPtr src, bool emlulatedMemory = true) override;
    void writeHost(JitWidth width, MemPtr address, U32 value, bool emlulatedMemory = true) override;
    Mem createMem(JitWidth width, MemPtr address);

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
    void IfTestBit(JitWidth regWidth, RegPtr reg, U32 bitPos) override;
    void IfNotTestBit(JitWidth regWidth, RegPtr reg, U32 bitPos) override;
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
    void IfDF() override;
    void IfSmallStack() override;
    void JumpIfCondition(JitConditional condition, U32 address) override;
    void clearMMUPermissionIfSpansPage(JitWidth width, RegPtr offset, RegPtr reg) override;
    void clearIfSpansPage(JitWidth width, RegPtr offset, RegPtr reg) override;

    U32 MarkJumpLocation() override;
    void Goto(U32 location) override;
    void jmpHost(RegPtr reg) override;
    void jmpHost(DYN_PTR_SIZE address) override;
    void updateFlagsIfNecessary();    
    RegPtr getReadOnlyFlags(RegPtr tmp = nullptr) override;
    void setFlags(RegPtr flags, U32 mask) override;

    void callHostFunction(void* address, const std::vector<DynParam>& params, bool restoreCache = true, bool saveCache = true) override;
    void callHostFunctionWithResult(RegPtr result, void* address, const std::vector<DynParam>& params) override;
    void nakedCall(RegPtr reg) override;
    void nakedReturn() override;
#ifdef BOXEDWINE_64
    void setParam(asmjit::x86::Gp reg, const DynParam& param);
#else
    void pushParam(const DynParam& param);
#endif
    void callLoadCache();
    void callWriteCache();

    std::array<bool, NUMBER_OF_REGS> regUsed{ 0 };
    std::array<bool, NUMBER_OF_XMM_REG> xmmUsed{ 0 };

    void JumpInBlock(U32 address) override;
    void StartElse() override;
    void EndIf() override;
    void blockExit() override;

    void direct_cmp(JitWidth width, RegPtr left, RegPtr right) override;
    void direct_cmp(JitWidth width, RegPtr left, U32 right) override;
    void direct_test(JitWidth width, RegPtr left, RegPtr right) override;
    void direct_test(JitWidth width, RegPtr left, U32 right) override;
    void direct_jump(JitConditional condition, U32 address) override;
    void direct_cmov(JitWidth width, JitConditional condition, RegPtr dst, RegPtr src) override;
    void direct_setcc(JitConditional condition, RegPtr dst) override;

    asmjit::x86::CondCode getCondCode(JitConditional condition);

    bool directDoesAffectFlags(DecodedOp* op) override;
    RegPtr calculateEaa(DecodedOp* op, U32 popEspAmount = 0) override;

    // FPU
    FPURegPtr getFPUTmp() override;
    void storeCpuFpuReg(FPURegPtr reg, RegPtr index) override;
    void loadCpuFpuReg(FPURegPtr reg, RegPtr index) override;
    void loadCpuFpuRegConst(FPURegPtr reg, U32 offset) override;
    void loadFpuRegFromInt(FPURegPtr reg, MemPtr address) override;
    void storeFpuReg(FPURegPtr reg, MemPtr address, DynFpuWidth width = DYN_FPU_64_BIT) override;
    RegPtr fpuRegToInt32(FPURegPtr fpuRegSrc, bool truncate) override;
    void fpuRegToInt64(FPURegPtr regDst, FPURegPtr fpuRegSrc, bool truncate);
    void fpuRegInt64To64(FPURegPtr regDst, FPURegPtr fpuRegSrc);
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
    void fpuSqrt(FPURegPtr dst, FPURegPtr src) override;
    void fcompare(FPURegPtr fpuReg1, FPURegPtr fpuReg2, RegPtr ordTags, const std::function<void()>& pfnEqual, const std::function<void()>& pfnLessThan, const std::function<void()>& pfnGreaterThan, const std::function<void()>& pfnInvalid) override;

    // MMX
    MMXRegPtr getTmpMMX() override;
    MMXRegPtr loadMMXFromReg(RegPtr reg) override;
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
    void IfSseLessThan(SSERegPtr src1, SSERegPtr src2) override;
    bool isSseRegCached(U8 reg) override;
    void storeCpuXMMReg(SSERegPtr reg, U32 index) override;
    SSERegPtr loadCpuXMMReg(U8 index) override;
    SSERegPtr loadXMMFromMem128(U8 reg, MemPtr address) override;
    SSERegPtr loadXMMFromMem64(U8 reg, MemPtr address) override;
    SSERegPtr loadLowXMMFromMem64(U8 reg, MemPtr address) override;
    SSERegPtr loadHighXMMFromMem64(U8 reg, MemPtr address) override;
    SSERegPtr loadXMMFromMem32(U8 reg, MemPtr address) override;
    void storeXMMToMem128(SSERegPtr reg, MemPtr address) override;
    void storeXMMToMem64(SSERegPtr reg, MemPtr address) override;
    void storeXMMToMem32(SSERegPtr reg, MemPtr address) override;
    void storeHighXMMToMem64(SSERegPtr reg, MemPtr address) override;

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

    // optional override, hopefully faster than the common_ methods
    void updateHardwareFlags(U32 flags);
    void dynamic_rdtsc(DecodedOp* op) override;
    void dynamic_arithE32R32_lock(DecodedOp* op, std::function<void(RegPtr dest, MemPtr address)> callback, bool writeReg = false);
    void dynamic_arithE16R16_lock(DecodedOp* op, std::function<void(RegPtr dest, MemPtr address)> callback, bool writeReg = false);
    void dynamic_arithE8R8_lock(DecodedOp* op, std::function<void(RegPtr dest, MemPtr address)> callback, bool writeReg = false);
    void dynamic_arithE32_lock(DecodedOp* op, std::function<void(MemPtr address)> callback);
    void dynamic_arithE16_lock(DecodedOp* op, std::function<void(MemPtr address)> callback);
    void dynamic_arithE8_lock(DecodedOp* op, std::function<void(MemPtr address)> callback);

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

protected:
    friend void startNewJIT(CPU* cpu, U32 address, DecodedOp* op);

    U32 getBufferSize() override;
    U32 getIfJumpSize() override;
    U32 getBufferLocation(U32 id) override;
    U32 markBufferLocation() override;
    void copyBuffer(U8* dst, U32 size) override;
    void onBlockPreCommit(DecodedOp* op) override;

    void setCC(asmjit::x86::Gp reg, JitEvaluate condition);

    U8* createStartJITCode() override;

    void loadCache();
    void writeCache();
    void setParams(const std::vector<DynParam>& params);
    U8* createSyncToHost() override;
    U8* createSyncFromHost() override;
    U8* createBlockExit() override;
    bool isHintAvailable(S8 hint);
    U8 findTmpXMM();
    SSERegPtr getXMM(U8 index, bool load);

    Vec getMMXReg(MMXRegPtr reg);
    Vec getFPUReg(FPURegPtr reg);

    void IfPF();
    void IfCF();
    void IfZF();
#ifdef BOXEDWINE_64
    asmjit::x86::Gp params[4];
#endif

    asmjit::JitRuntime rt;
    asmjit::CodeHolder code;
    asmjit::x86::Assembler compiler;

    std::vector<Label> labels;
    std::vector<Label> ifLabels;
    BHashTable<U32, Label> opLabels;
    BHashTable<U32, Label> pendingLabels;
};

void JitX86CodeGen::preOp(DecodedOp* op) {
    regUsed.fill(false);
    xmmUsed.fill(false);
    currentOp = op;
    Label label;
    if (!opLabels.get(currentEip, label)) {
        label = compiler.new_label();
        opLabels.set(currentEip, label);
    }
    compiler.bind(label);
    pendingLabels.remove(currentEip);
}

U8 JitX86CodeGen::findTmpXMM() {
    for (int i = 0; i < NUMBER_OF_XMM_TMPS; i++) {
        U8 tmp = XMMtmps[i];
        if (!xmmUsed[tmp]) {
            xmmUsed[tmp] = true;
            return tmp;
        }
    }
    kpanic("JitX86CodeGen::findTmpXMM");
    return 0xff;
}

bool JitX86CodeGen::isSseRegCached(U8 reg) {
    if (reg >= 8) {
        kpanic("isSseRegCached");
        return false;
    }
    return xmmCache[reg] != INVALID_REG;
}

Vec JitX86CodeGen::getFPUReg(FPURegPtr reg) {
    return Vec::make_xmm(reg->hardwareReg());
}

SSERegPtr JitX86CodeGen::getTmpSSE() {
    return std::shared_ptr<SSERegInternal>(new SSERegInternal(findTmpXMM(), 0xff), [this](SSERegInternal* p) {
        xmmUsed[p->hardwareReg()] = false;
        delete p;
    });
}

Vec JitX86CodeGen::getMMXReg(MMXRegPtr reg) {
    return Vec::make_xmm(reg->hardwareReg());
}

MMXRegPtr JitX86CodeGen::getTmpMMX() {
    return std::shared_ptr<MMXRegInternal>(new MMXRegInternal(findTmpXMM(), 0xff), [this](MMXRegInternal* p) {
        xmmUsed[p->hardwareReg()] = false;
        delete p;
    });
}

FPURegPtr JitX86CodeGen::getFPUTmp() {
    return std::shared_ptr<FPURegInternal>(new FPURegInternal(findTmpXMM()), [this](FPURegInternal* p) {
        xmmUsed[p->hardwareReg()] = false;
        delete p;
    });
}

void JitX86CodeGen::emulateSingleOp() {
    writeCurrentEip(0);
    compiler.jmp((DYN_PTR_SIZE)cpu->thread->process->emulateSingleOp);
}

bool JitX86CodeGen::isHintAvailable(S8 hint) {
    return (hint >= 0 && isTmp[hint] && !regUsed[hint]);
}

U8 JitX86CodeGen::findTmpReg(bool needs8bitReg, S8 hint, bool allowInvalidReturn) {
#ifdef BOXEDWINE_64    
    if (isHintAvailable(hint)) {
        regUsed[hint] = true;
        return (U8)hint;
    }
    for (int i = 0; i < NUMBER_OF_TMPS; i++) {
        U8 tmp = tmps[i];
        if (needs8bitReg && tmp > 3) {
            continue;
        }
        if (!regUsed[tmp]) {
            regUsed[tmp] = true;
            return tmp;
        }
    }
    if (needs8bitReg) {
        return findTmpReg(false, hint);
    }
    if (!allowInvalidReturn) {
        kpanic("JitX86CodeGen::getTmpReg ran out of tmp regs");
    }
    return 0xff;
#else
    if (hint >= 0 && !regUsed[hint]) {
        regUsed[hint] = true;
        return (U8)hint;
    }
    U8 tmpReg = 0xff;
    if (!needs8bitReg && !regUsed[6]) {
        regUsed[6] = true;
        return 6;
    }
    for (int i = 3; i >= 0; i--) {
        if (!regUsed[i]) {
            regUsed[i] = true;
            tmpReg = i;
            break;
        }
    }
    if (tmpReg == 0xff && !allowInvalidReturn) {
        kpanic("JitX86CodeGen::getTmpReg ran out of tmp regs");
    }
    return tmpReg;
#endif
}

RegPtr JitX86CodeGen::getConditionCalculationReg(U32 index) {
    U8 tmp = 0xff;
#ifdef BOXEDWINE_64
    if (index == 0) {
        tmp = tmps[NUMBER_OF_TMPS - 1];
        if (regUsed[tmp]) {
            kpanic("JitX86CodeGen::getConditionCalculationReg 0");
        }
    } else if (index == 1) {
        tmp = tmps[NUMBER_OF_TMPS - 2];
        if (regUsed[tmp]) {
            kpanic("JitX86CodeGen::getConditionCalculationReg 1");
        }
    } else if (index == 2) {
        tmp = tmps[NUMBER_OF_TMPS - 3];
        if (regUsed[tmp]) {
            kpanic("JitX86CodeGen::getConditionCalculationReg 2");
        }
    } else {
        kpanic("JitX86CodeGen::getConditionCalculationReg");
        return nullptr;
    }
#else
    if (index == 0) {
        tmp = 0;
        if (regUsed[0]) {
            kpanic("JitX86CodeGen::getConditionCalculationReg 0");
        }
    } else if (index == 1) {
        tmp = 2;
        if (regUsed[2]) {
            kpanic("JitX86CodeGen::getConditionCalculationReg 1");
        }
    } else if (index == 2) {
        tmp = 1;
        if (regUsed[1]) {
            kpanic("JitX86CodeGen::getConditionCalculationReg 2");
        }
    } else {
        kpanic("JitX86CodeGen::getConditionCalculationReg");
        return nullptr;
    }
#endif
    regUsed[tmp] = true;
    return std::shared_ptr<JitReg>(new JitReg(tmp, 0xff), [this](JitReg* p) {
        if (p->isLoaded()) {
            regUsed[p->hardwareReg()] = false;
        }
        delete p;
    });
}

RegPtr JitX86CodeGen::getTmpReg() {
    return getTmpRegWithHint(-1);
}

RegPtr JitX86CodeGen::getTmpRegWithHint(S8 hint) {
    return std::shared_ptr<JitReg>(new JitReg(findTmpReg(false, hint), 0xff), [this](JitReg* p) {
        if (p->isLoaded()) {
            regUsed[p->hardwareReg()] = false;
        }
        delete p;
    });
}

RegPtr JitX86CodeGen::getTmpReg8() {
    return std::shared_ptr<JitReg>(new JitReg(findTmpReg(true), 0xff), [this](JitReg* p) {
        if (p->isLoaded()) {
            regUsed[p->hardwareReg()] = false;
        }
        delete p;
    });
}

RegPtr JitX86CodeGen::getTmpReg(U8 reg, bool delayed, S8 hint) {
    if (delayed) {
        auto getTmp = [reg, hint, this]() {
            U8 hardwareReg = findTmpReg(false, hint);
            if (regCache[reg] != INVALID_REG) {
                compiler.mov(R32(hardwareReg), R32(regCache[reg]));
            } else {
                compiler.mov(R32(hardwareReg), Mem(HOST_CPU, CPU::offsetofReg32(reg)));
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
        RegPtr result = std::shared_ptr<JitReg>(new JitReg(findTmpReg(false, hint), 0xff), [this](JitReg* p) {
            if (p->isLoaded()) {
                regUsed[p->hardwareReg()] = false;
            }
            delete p;
        });
        if (regCache[reg] != INVALID_REG) {
            compiler.mov(R32(result), R32(regCache[reg]));
        } else {
            compiler.mov(R32(result), Mem(HOST_CPU, CPU::offsetofReg32(reg)));
        }
        return result;
    }
}

RegPtr JitX86CodeGen::getTmpRegForCallResult() {
    U8 reg;

    if (regUsed[0] || !isTmp[0]) {
        reg = findTmpReg(true);
    } else {
        reg = 0;
        regUsed[0] = true;
    }
    return std::shared_ptr<JitReg>(new JitReg(reg, 0xff), [this](JitReg* p) {
        if (p->isLoaded()) {
            regUsed[p->hardwareReg()] = false;
        }
        delete p;
    });
}

RegPtr JitX86CodeGen::getTmpSegAddress(U8 reg) {
    return getReadOnlySegAddress(reg);
}

RegPtr JitX86CodeGen::getReadOnlySegAddress(U8 reg) {
    RegPtr result = getTmpReg();
    compiler.mov(R32(result), Mem(HOST_CPU, CPU::offsetofSegAddress(reg)));
    return result;
}

RegPtr JitX86CodeGen::getReadOnlySegValue(U8 reg) {
    RegPtr result = getTmpReg();
    compiler.mov(R32(result), Mem(HOST_CPU, CPU::offsetofSegValue(reg)));
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
                        compiler.mov(R8(hardwareReg), R8(regCache[reg] + 4));
                    } else {
                        compiler.mov(R32(hardwareReg), R32(regCache[reg]));
                        compiler.shr(R16(hardwareReg), 8);
                    }
                } else {
                    compiler.mov(R32(hardwareReg), R32(regCache[reg]));
                }
            } else {
                compiler.mov(R8(hardwareReg), Mem(HOST_CPU, CPU::offsetofReg8(originalReg)));
            }
            return hardwareReg;
        };

        result = std::shared_ptr<JitReg>(new JitReg(0xff, 0xff, false, getTmp), [this](JitReg* p) {
            if (p->isLoaded()) {
                regUsed[p->hardwareReg()] = false;
            }
            delete p;
        });
    } else {
        result = std::shared_ptr<JitReg>(new JitReg(findTmpReg(true, hint), 0xff, false), [this](JitReg* p) {
            if (p->isLoaded()) {
                regUsed[p->hardwareReg()] = false;
            }
            delete p;
        });
        if (regCache[reg] != INVALID_REG) {
            if (isHigh) {
                if (regCache[reg] < 4 && result->hardwareReg() < 4) {
                    compiler.mov(R8(result), R8(regCache[reg] + 4));
                } else {
                    compiler.mov(R32(result), R32(regCache[reg]));
                    compiler.shr(R16(result), 8);
                }
            } else {
                compiler.mov(R32(result), R32(regCache[reg]));
            }
        } else {
            compiler.mov(R8(result), Mem(HOST_CPU, CPU::offsetofReg8(originalReg)));
        }
    }
    return result;
}

RegPtr JitX86CodeGen::readEip() {
    RegPtr result = getTmpReg();
    compiler.mov(R32(result), Mem(HOST_CPU, offsetof(CPU, eip.u32)));    
    return result;
}

void JitX86CodeGen::writeEip(RegPtr reg) {
    compiler.mov(Mem(HOST_CPU, offsetof(CPU, eip.u32)), R32(reg));
}

void JitX86CodeGen::writeEip(U32 eip) {
    compiler.mov(Mem32(HOST_CPU, offsetof(CPU, eip.u32)), eip);
}

bool JitX86CodeGen::isTmpRegAvailable() {
    U8 found = findTmpReg(false, -1, true);
    if (found == 0xff) {
        return false;
    }
    regUsed[found] = false;
    return true;
}

void JitX86CodeGen::forceSyncBackIfNotCached(RegPtr reg) {
    if (reg->emulatedReg != 0xff && regCache[reg->emulatedReg] == INVALID_REG) {
        compiler.mov(Mem(HOST_CPU, CPU::offsetofReg32(reg->emulatedReg)), R32(reg));
    }
}

RegPtr JitX86CodeGen::getStringRegEcx() {
#ifndef BOXEDWINE_64
    if (regUsed[1]) {
        kpanic("");
    }
#endif
    return getReg(1, 1);
}

RegPtr JitX86CodeGen::getStringRegEsi() {
#ifndef BOXEDWINE_64
    if (regUsed[6]) {
        kpanic("");
    }
#endif
    return getReg(6, 6);
}

RegPtr JitX86CodeGen::getStringRegEdi() {
#ifndef BOXEDWINE_64
    if (regUsed[2]) {
        kpanic("");
    }
#endif
    return getReg(7, 2);
}

RegPtr JitX86CodeGen::getReg(U8 reg, S8 hint, bool load) {
    if (regCache[reg] == INVALID_REG) {
        RegPtr result = std::shared_ptr<JitReg>(new JitReg(findTmpReg(false, hint), reg), [this](JitReg* p) {
            compiler.mov(Mem(HOST_CPU, CPU::offsetofReg32(p->emulatedReg)), R32(p->hardwareReg()));
            regUsed[p->hardwareReg()] = false;
            delete p;
            });
        if (load) {
            compiler.mov(R32(result), Mem(HOST_CPU, CPU::offsetofReg32(reg)));
        }
        return result;
    } else {
        if (isHintAvailable(hint)) {
            regUsed[hint] = true;
            RegPtr result = std::shared_ptr<JitReg>(new JitReg(hint, reg), [reg, this](JitReg* p) {
                compiler.mov(R32(regCache[reg]), R32(p->hardwareReg()));
                regUsed[p->hardwareReg()] = false;
                delete p;
            });
            if (load) {
                compiler.mov(R32(result), R32(regCache[reg]));
            }
            return result;
        }
        return std::shared_ptr<JitReg>(new JitReg(regCache[reg], reg), [this](JitReg* p) {
            if (p->isLoaded()) {
                regUsed[p->hardwareReg()] = false;
            }
            delete p;
        });
    }
}

RegPtr JitX86CodeGen::getReg8(U8 reg, bool load) {
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
                        compiler.and_(R32(regCache[p->emulatedReg]), 0xffff00ff);
                        compiler.and_(R32(p->hardwareReg()), 0xff);
                        compiler.shl(R32(p->hardwareReg()), 8);
                        compiler.or_(R32(regCache[p->emulatedReg]), R32(p->hardwareReg()));
                    } else {
                        compiler.mov(R8(regCache[p->emulatedReg] + 4), R8(p->hardwareReg()));
                    }
                } else {
                    compiler.mov(Mem(HOST_CPU, CPU::offsetofReg32(p->emulatedReg) + 1), R8(p->hardwareReg()));
                }
                regUsed[p->hardwareReg()] = false;
            }
            delete p;
        });
        if (load) {
            if (regCache[reg] != INVALID_REG) {
                if (result->hardwareReg() > 3 || regCache[reg] > 3) {
                    compiler.mov(R32(result), R32(regCache[reg]));
                    compiler.shr(R16(result), 8);
                } else {
                    compiler.mov(R8(result), R8(regCache[reg] + 4));
                }
            } else {
                compiler.mov(R8(result), Mem(HOST_CPU, CPU::offsetofReg32(reg) + 1));
            }
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
                compiler.mov(R32(regCache[p->emulatedReg]), R32(p->hardwareReg()));
            } else {
                compiler.mov(Mem(HOST_CPU, CPU::offsetofReg32(p->emulatedReg)), R32(p->hardwareReg()));
            }
            regUsed[p->hardwareReg()] = false;
        }
        delete p;
    });
    if (regCache[reg] != INVALID_REG) {
        compiler.mov(R32(result), R32(regCache[reg]));
    } else {
        compiler.mov(R32(result), Mem(HOST_CPU, CPU::offsetofReg32(reg)));
    }
    return result;
}

RegPtr JitX86CodeGen::getReadOnlyReg(U8 reg, bool delayed, S8 hint) {
    if (regCache[reg] != INVALID_REG) {
        return std::shared_ptr<JitReg>(new JitReg(regCache[reg], reg), [this](JitReg* p) {
            if (p->isLoaded()) {
                regUsed[p->hardwareReg()] = false;
            }
            delete p;
        });
    } else {
        return getTmpReg(reg, delayed, hint);
    }
}

RegPtr JitX86CodeGen::getReadOnlyReg8(U8 reg, bool delayed, S8 hint) {
    if (regCache[reg] != INVALID_REG && regCache[reg] < 4) {
        return std::shared_ptr<JitReg>(new JitReg(regCache[reg], reg), [this](JitReg* p) {
            if (p->isLoaded()) {
                regUsed[p->hardwareReg()] = false;
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
        compiler.add(R32(reg), R32(rm));
    } else if (regWidth == JitWidth::b16) {
        compiler.add(R16(reg), R16(rm));
    } else if (regWidth == JitWidth::b8) {
        compiler.add(R8(get8bitReg(reg)), R8(get8bitReg(rm)));
#ifdef BOXEDWINE_64
    } else if (regWidth == JitWidth::b64) {
        compiler.add(R64(reg), R64(rm));
#endif
    } else {
        kpanic("JitX86CodeGen::addReg");
    }
}

void JitX86CodeGen::addValue(JitWidth regWidth, RegPtr reg, U32 imm) {
    if (regWidth == JitWidth::b32) {
        compiler.add(R32(reg), imm);
    } else if (regWidth == JitWidth::b16) {
        compiler.add(R16(reg), (U16)imm);
    } else if (regWidth == JitWidth::b8) {
        compiler.add(R8(get8bitReg(reg)), (U8)imm);
    } else {
        kpanic("JitX86CodeGen::addValue");
    }
}

void JitX86CodeGen::orReg(JitWidth regWidth, RegPtr reg, RegPtr rm) {
    if (regWidth == JitWidth::b32) {
        compiler.or_(R32(reg), R32(rm));
    } else if (regWidth == JitWidth::b16) {
        compiler.or_(R16(reg), R16(rm));
    } else if (regWidth == JitWidth::b8) {
        compiler.or_(R8(get8bitReg(reg)), R8(get8bitReg(rm)));
    } else {
        kpanic("JitX86CodeGen::orReg");
    }
}

void JitX86CodeGen::orValue(JitWidth regWidth, RegPtr reg, U32 imm) {
    if (regWidth == JitWidth::b32) {
        compiler.or_(R32(reg), imm);
    } else if (regWidth == JitWidth::b16) {
        compiler.or_(R16(reg), (U16)imm);
    } else if (regWidth == JitWidth::b8) {
        compiler.or_(R8(get8bitReg(reg)), (U8)imm);
    } else {
        kpanic("JitX86CodeGen::orValue");
    }
}

void JitX86CodeGen::subReg(JitWidth regWidth, RegPtr reg, RegPtr rm) {
    if (regWidth == JitWidth::b32) {
        compiler.sub(R32(reg), R32(rm));
    } else if (regWidth == JitWidth::b16) {
        compiler.sub(R16(reg), R16(rm));
    } else if (regWidth == JitWidth::b8) {
        compiler.sub(R8(get8bitReg(reg)), R8(get8bitReg(rm)));
    } else {
        kpanic("JitX86CodeGen::subReg");
    }
}

void JitX86CodeGen::subValue(JitWidth regWidth, RegPtr reg, U32 imm) {
    if (regWidth == JitWidth::b32) {
        compiler.sub(R32(reg), imm);
    } else if (regWidth == JitWidth::b16) {
        compiler.sub(R16(reg), (U16)imm);
    } else if (regWidth == JitWidth::b8) {
        compiler.sub(R8(get8bitReg(reg)), (U8)imm);
    } else {
        kpanic("JitX86CodeGen::subValue");
    }
}

void JitX86CodeGen::andReg(JitWidth regWidth, RegPtr reg, RegPtr rm) {
    if (regWidth == JitWidth::b32) {
        compiler.and_(R32(reg), R32(rm));
    } else if (regWidth == JitWidth::b16) {
        compiler.and_(R16(reg), R16(rm));
    } else if (regWidth == JitWidth::b8) {
        compiler.and_(R8(get8bitReg(reg)), R8(get8bitReg(rm)));
    } else {
        kpanic("JitX86CodeGen::andReg");
    }
}

#ifdef BOXEDWINE_64
void JitX86CodeGen::andValue64(RegPtr reg, U64 imm) {
    compiler.and_(R64(reg), imm);
}
#endif

void JitX86CodeGen::andValue(JitWidth regWidth, RegPtr reg, U32 imm) {
    if (regWidth == JitWidth::b32) {
        compiler.and_(R32(reg), imm);
    } else if (regWidth == JitWidth::b16) {
        compiler.and_(R16(reg), (U16)imm);
    } else if (regWidth == JitWidth::b8) {
        compiler.and_(R8(get8bitReg(reg)), (U8)imm);
#ifdef BOXEDWINE_64
    }  else if (regWidth == JitWidth::b64) {
        compiler.and_(R64(reg), imm);
#endif
    } else {
        kpanic("JitX86CodeGen::andValue");
    }
}

void JitX86CodeGen::xorReg(JitWidth regWidth, RegPtr reg, RegPtr rm) {
    if (regWidth == JitWidth::b32) {
        compiler.xor_(R32(reg), R32(rm));
    } else if (regWidth == JitWidth::b16) {
        compiler.xor_(R16(reg), R16(rm));
    } else if (regWidth == JitWidth::b8) {
        compiler.xor_(R8(get8bitReg(reg)), R8(get8bitReg(rm)));
    } else {
        kpanic("JitX86CodeGen::xorReg");
    }
}

void JitX86CodeGen::xorValue(JitWidth regWidth, RegPtr reg, U32 imm) {
    if (regWidth == JitWidth::b32) {
        compiler.xor_(R32(reg), imm);
    } else if (regWidth == JitWidth::b16) {
        compiler.xor_(R16(reg), (U16)imm);
    } else if (regWidth == JitWidth::b8) {
        compiler.xor_(R8(get8bitReg(reg)), (U8)imm);
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
            compiler.shrx(R32(reg), R32(reg), R32(rm));
            return;
        }
    }
#endif
    if (rm->hardwareReg() != 1) {
        compiler.push(RN(1));
        if (reg->hardwareReg() == 1) {
            kpanic("JitX86CodeGen::shlReg 1");
        }
    }
    if (regWidth == JitWidth::b32) {
        if (rm->hardwareReg() != 1) {
            compiler.mov(regEcx, R32(rm));
        }
        compiler.shr(R32(reg), regEcx);
    } else if (regWidth == JitWidth::b16) {
        if (rm->hardwareReg() != 1) {
            compiler.mov(regCx, R16(rm));
        }
        compiler.shr(R16(reg), regCx);
    } else if (regWidth == JitWidth::b8) {
        if (rm->hardwareReg() != 1) {
            compiler.mov(regCl, R8(get8bitReg(rm)));
        }
        compiler.shr(R8(get8bitReg(reg)), regCl);
    } else {
        kpanic("JitX86CodeGen::shrReg");
    }
    if (rm->hardwareReg() != 1) {
        compiler.pop(RN(1));
    }
}

void JitX86CodeGen::shrValue(JitWidth regWidth, RegPtr reg, U32 imm) {
    if (regWidth == JitWidth::b32) {
        compiler.shr(R32(reg), imm);
    } else if (regWidth == JitWidth::b16) {
        compiler.shr(R16(reg), (U16)imm);
    } else if (regWidth == JitWidth::b8) {
        compiler.shr(R8(get8bitReg(reg)), (U8)imm);
#ifdef BOXEDWINE_64
    } else if (regWidth == JitWidth::b64) {
        compiler.shr(R64(get8bitReg(reg)), (U8)imm);
#endif
    } else {
        kpanic("JitX86CodeGen::shrValue");
    }
}

void JitX86CodeGen::shlReg(JitWidth regWidth, RegPtr reg, RegPtr rm) {
#ifdef BOXEDWINE_64
    if (hasBMI2) {
        if (regWidth == JitWidth::b32 && rm->hardwareReg() != 1) {
            compiler.shlx(R32(reg), R32(reg), R32(rm));
            return;
        }
    }
#endif
    if (rm->hardwareReg() != 1) {
        compiler.push(RN(1));
        if (reg->hardwareReg() == 1) {
            kpanic("JitX86CodeGen::shlReg 1");
        }
    }
    if (regWidth == JitWidth::b32) {
        if (rm->hardwareReg() != 1) {
            compiler.mov(regEcx, R32(rm));
        }
        compiler.shl(R32(reg), regEcx);
    } else if (regWidth == JitWidth::b16) {
        if (rm->hardwareReg() != 1) {
            compiler.mov(regCx, R16(rm));
        }
        compiler.shl(R16(reg), regCx);
    } else if (regWidth == JitWidth::b8) {
        if (rm->hardwareReg() != 1) {
            compiler.mov(regCl, R8(get8bitReg(rm)));
        }
        compiler.shl(R8(get8bitReg(reg)), regCl);
#ifdef BOXEDWINE_64
    } else if (regWidth == JitWidth::b64) {
        if (rm->hardwareReg() != 1) {
            compiler.mov(regCl, R8(get8bitReg(rm)));
        }
        compiler.shl(R64(reg), regCl);
#endif
    } else {
        kpanic("JitX86CodeGen::shlReg");
    }
    if (rm->hardwareReg() != 1) {
        compiler.pop(RN(1));
    }
}

void JitX86CodeGen::shlValue(JitWidth regWidth, RegPtr reg, U32 imm) {
    if (regWidth == JitWidth::b32) {
        compiler.shl(R32(reg), imm);
    } else if (regWidth == JitWidth::b16) {
        compiler.shl(R16(reg), (U16)imm);
    } else if (regWidth == JitWidth::b8) {
        compiler.shl(R8(get8bitReg(reg)), (U8)imm);
    } else {
        kpanic("JitX86CodeGen::shlValue");
    }
}

void JitX86CodeGen::sarReg(JitWidth regWidth, RegPtr reg, RegPtr rm) {
#ifdef BOXEDWINE_64
    if (hasBMI2) {
        if (regWidth == JitWidth::b32 && rm->hardwareReg() != 1) {
            compiler.sarx(R32(reg), R32(reg), R32(rm));
            return;
        }
    }
#endif
    if (rm->hardwareReg() != 1) {
        compiler.push(RN(1));
        if (reg->hardwareReg() == 1) {
            kpanic("JitX86CodeGen::shlReg 1");
        }
    }
    if (regWidth == JitWidth::b32) {
        if (rm->hardwareReg() != 1) {
            compiler.mov(regEcx, R32(rm));
        }
        compiler.sar(R32(reg), regEcx);
    } else if (regWidth == JitWidth::b16) {
        if (rm->hardwareReg() != 1) {
            compiler.mov(regCx, R16(rm));
        }
        compiler.sar(R16(reg), regCx);
    } else if (regWidth == JitWidth::b8) {
        if (rm->hardwareReg() != 1) {
            compiler.mov(regCl, R8(get8bitReg(rm)));
        }
        compiler.sar(R8(get8bitReg(reg)), regCl);
    } else {
        kpanic("JitX86CodeGen::sarReg");
    }
    if (rm->hardwareReg() != 1) {
        compiler.pop(RN(1));
    }
}

void JitX86CodeGen::sarValue(JitWidth regWidth, RegPtr reg, U32 imm) {
    if (regWidth == JitWidth::b32) {
        compiler.sar(R32(reg), imm);
    } else if (regWidth == JitWidth::b16) {
        compiler.sar(R16(reg), (U16)imm);
    } else if (regWidth == JitWidth::b8) {
        compiler.sar(R8(get8bitReg(reg)), (U8)imm);
    } else {
        kpanic("JitX86CodeGen::sarValue");
    }
}

void JitX86CodeGen::notReg2(JitWidth regWidth, RegPtr reg) {
    if (regWidth == JitWidth::b32) {
        compiler.not_(R32(reg));
    } else if (regWidth == JitWidth::b16) {
        compiler.not_(R16(reg));
    } else if (regWidth == JitWidth::b8) {
        compiler.not_(R8(get8bitReg(reg)));
    } else {
        kpanic("JitX86CodeGen::notReg");
    }
    // not doesn't set flags
}

void JitX86CodeGen::negReg2(JitWidth regWidth, RegPtr reg) {
    if (regWidth == JitWidth::b32) {
        compiler.neg(R32(reg));
    } else if (regWidth == JitWidth::b16) {
        compiler.neg(R16(reg));
    } else if (regWidth == JitWidth::b8) {
        compiler.neg(R8(get8bitReg(reg)));
    } else {
        kpanic("JitX86CodeGen::negReg");
    }
}

void JitX86CodeGen::bsfReg(JitWidth regWidth, RegPtr reg, RegPtr rm) {
    if (regWidth == JitWidth::b32) {
        compiler.bsf(R32(reg), R32(rm));
    } else if (regWidth == JitWidth::b16) {
        compiler.bsf(R16(reg), R16(rm));
    } else {
        kpanic("JitX86CodeGen::bsfReg");
    }
}

void JitX86CodeGen::bsrReg(JitWidth regWidth, RegPtr reg, RegPtr rm) {
    if (regWidth == JitWidth::b32) {
        compiler.bsr(R32(reg), R32(rm));
    } else if (regWidth == JitWidth::b16) {
        compiler.bsr(R16(reg), R16(rm));
    } else {
        kpanic("JitX86CodeGen::bsrReg");
    }
}

void JitX86CodeGen::rolReg(JitWidth regWidth, RegPtr reg, RegPtr rm) {
    if (regWidth == JitWidth::b32) {
        compiler.rol(R32(reg), R32(rm));
    } else if (regWidth == JitWidth::b16) {
        compiler.rol(R16(reg), R16(rm));
    } else if (regWidth == JitWidth::b8) {
        compiler.rol(R8(get8bitReg(reg)), R8(get8bitReg(rm)));
    } else {
        kpanic("JitX86CodeGen::rolReg");
    }
}

void JitX86CodeGen::rolValue(JitWidth regWidth, RegPtr reg, U32 imm) {
    if (regWidth == JitWidth::b32) {
        compiler.rol(R32(reg), imm);
    } else if (regWidth == JitWidth::b16) {
        compiler.rol(R16(reg), (U16)imm);
    } else if (regWidth == JitWidth::b8) {
        compiler.rol(R8(get8bitReg(reg)), (U8)imm);
    } else {
        kpanic("JitX86CodeGen::rolValue");
    }
}

void JitX86CodeGen::rorReg(JitWidth regWidth, RegPtr reg, RegPtr rm) {
    if (regWidth == JitWidth::b32) {
        compiler.ror(R32(reg), R32(rm));
    } else if (regWidth == JitWidth::b16) {
        compiler.ror(R16(reg), R16(rm));
    } else if (regWidth == JitWidth::b8) {
        compiler.ror(R8(get8bitReg(reg)), R8(get8bitReg(rm)));
    } else {
        kpanic("JitX86CodeGen::rorReg");
    }
}

void JitX86CodeGen::rorValue(JitWidth regWidth, RegPtr reg, U32 imm) {
    if (regWidth == JitWidth::b32) {
        compiler.ror(R32(reg), imm);
    } else if (regWidth == JitWidth::b16) {
        compiler.ror(R16(reg), (U16)imm);
    } else if (regWidth == JitWidth::b8) {
        compiler.ror(R8(get8bitReg(reg)), (U8)imm);
    } else {
        kpanic("JitX86CodeGen::rorValue");
    }
}

void JitX86CodeGen::rclReg(JitWidth regWidth, RegPtr reg, RegPtr rm, RegPtr cf) {
    // shr will move the bottom bit into the carry flag
    compiler.shr(R32(cf), 1);

    if (regWidth == JitWidth::b32) {
        compiler.rcl(R32(reg), R32(rm));
    } else if (regWidth == JitWidth::b16) {
        compiler.rcl(R16(reg), R16(rm));
    } else if (regWidth == JitWidth::b8) {
        compiler.rcl(R8(get8bitReg(reg)), R8(get8bitReg(rm)));
    } else {
        kpanic("JitX86CodeGen::rclReg");
    }
}

void JitX86CodeGen::rclValue(JitWidth regWidth, RegPtr reg, U32 imm, RegPtr cf) {
    // shr will move the bottom bit into the carry flag
    compiler.shr(R32(cf), 1);

    if (regWidth == JitWidth::b32) {
        compiler.rcl(R32(reg), imm);
    } else if (regWidth == JitWidth::b16) {
        compiler.rcl(R16(reg), (U16)imm);
    } else if (regWidth == JitWidth::b8) {
        compiler.rcl(R8(get8bitReg(reg)), (U8)imm);
    } else {
        kpanic("JitX86CodeGen::rclValue");
    }
}

void JitX86CodeGen::rcrReg(JitWidth regWidth, RegPtr reg, RegPtr rm, RegPtr cf) {
    // shr will move the bottom bit into the carry flag
    compiler.shr(R32(cf), 1);

    if (regWidth == JitWidth::b32) {
        compiler.rcr(R32(reg), R32(rm));
    } else if (regWidth == JitWidth::b16) {
        compiler.rcr(R16(reg), R16(rm));
    } else if (regWidth == JitWidth::b8) {
        compiler.rcr(R8(get8bitReg(reg)), R8(get8bitReg(rm)));
    } else {
        kpanic("JitX86CodeGen::rcrReg");
    }
}

void JitX86CodeGen::rcrValue(JitWidth regWidth, RegPtr reg, U32 imm, RegPtr cf) {
    // shr will move the bottom bit into the carry flag
    compiler.shr(R32(cf), 1);

    if (regWidth == JitWidth::b32) {
        compiler.rcr(R32(reg), imm);
    } else if (regWidth == JitWidth::b16) {
        compiler.rcr(R16(reg), (U16)imm);
    } else if (regWidth == JitWidth::b8) {
        compiler.rcr(R8(get8bitReg(reg)), (U8)imm);
    } else {
        kpanic("JitX86CodeGen::rcrValue");
    }
}

void JitX86CodeGen::shldReg(JitWidth regWidth, RegPtr reg, RegPtr rm, RegPtr cl) {
    if (rm->hardwareReg() != 1) {
        compiler.push(RN(1));
    }
    if (reg->hardwareReg() == 1 || rm->hardwareReg() == 1) {
        kpanic("JitX86CodeGen::shldReg cl");
    }
    if (regWidth == JitWidth::b32) {
        if (rm->hardwareReg() != 1) {
            compiler.mov(regEcx, R32(cl));
        }
        compiler.shld(R32(reg), R32(rm), regEcx);
    } else if (regWidth == JitWidth::b16) {
        if (rm->hardwareReg() != 1) {
            compiler.mov(regCx, R16(cl));
        }
        compiler.shld(R16(reg), R16(rm), regCx);
    } else {
        kpanic("JitX86CodeGen::shldReg");
    }
    if (rm->hardwareReg() != 1) {
        compiler.pop(RN(1));
    }
}

void JitX86CodeGen::shldValue(JitWidth regWidth, RegPtr reg, RegPtr rm, U32 imm) {
    // don't need to check if imm is 0, that was handled in the decoder, if it was 0, the decoder will replace shld with nop

    if (regWidth == JitWidth::b32) {
        compiler.shld(R32(reg), R32(rm), imm);
    } else if (regWidth == JitWidth::b16) {
        compiler.shld(R16(reg), R16(rm), (U16)imm);
    } else {
        kpanic("JitX86CodeGen::shldReg");
    }
}

void JitX86CodeGen::shrdReg(JitWidth regWidth, RegPtr reg, RegPtr rm, RegPtr cl) {
    if (rm->hardwareReg() != 1) {
        compiler.push(RN(1));
    }
    if (reg->hardwareReg() == 1 || rm->hardwareReg() == 1) {
        kpanic("JitX86CodeGen::shrdReg cl");
    }
    if (regWidth == JitWidth::b32) {
        if (rm->hardwareReg() != 1) {
            compiler.mov(regEcx, R32(cl));
        }
        compiler.shrd(R32(reg), R32(rm), regEcx);
    } else if (regWidth == JitWidth::b16) {
        if (rm->hardwareReg() != 1) {
            compiler.mov(regCx, R16(cl));
        }
        compiler.shrd(R16(reg), R16(rm), regCx);
    } else {
        kpanic("JitX86CodeGen::shrdReg");
    }
    if (rm->hardwareReg() != 1) {
        compiler.pop(RN(1));
    }
}

void JitX86CodeGen::shrdValue(JitWidth regWidth, RegPtr reg, RegPtr rm, U32 imm) {
    // don't need to check if imm is 0, that was handled in the decoder, if it was 0, the decoder will replace shrd with nop

    if (regWidth == JitWidth::b32) {
        compiler.shrd(R32(reg), R32(rm), imm);
    } else if (regWidth == JitWidth::b16) {
        compiler.shrd(R16(reg), R16(rm), (U16)imm);
    } else {
        kpanic("JitX86CodeGen::shrdReg");
    }
}

void JitX86CodeGen::xchgReg(JitWidth regWidth, RegPtr dest, RegPtr src) {
    if (regWidth == JitWidth::b32) {
        compiler.xchg(R32(dest), R32(src));
    } else if (regWidth == JitWidth::b16) {
        compiler.xchg(R16(dest), R16(src));
    } else if (regWidth == JitWidth::b8) {
        compiler.xchg(R8(get8bitReg(dest)), R8(get8bitReg(src)));
    } else {
        kpanic("JitX86CodeGen::xchgReg");
    }
}

void JitX86CodeGen::byteSwapReg32(RegPtr reg) {
    compiler.bswap(R32(reg));
}

RegPtr JitX86CodeGen::compareReg(JitWidth regWidth, RegPtr reg1, RegPtr reg2, JitEvaluate condition, RegPtr result) {
    if (regWidth == JitWidth::b32) {
        compiler.cmp(R32(reg1), R32(reg2));
    } else if (regWidth == JitWidth::b16) {
        compiler.cmp(R16(reg1), R16(reg2));
    } else if (regWidth == JitWidth::b8) {
        compiler.cmp(R8(get8bitReg(reg1)), R8(get8bitReg(reg2)));
    } else {
        kpanic_fmt("JitX86CodeGen::compareReg reg width %d", regWidth);
    }
    if (!result) {
        result = getTmpReg8();
    }
    setCC(R8(result), condition);
    compiler.movzx(R32(result), R8(get8bitReg(result)));
    return result;
}

RegPtr JitX86CodeGen::testZeroReg(JitWidth regWidth, RegPtr reg, RegPtr result) {
    if (regWidth == JitWidth::b32) {
        compiler.test(R32(reg), R32(reg));
    } else if (regWidth == JitWidth::b16) {
        compiler.test(R16(reg), R16(reg));
    } else if (regWidth == JitWidth::b8) {
        compiler.test(R8(get8bitReg(reg)), R8(get8bitReg(reg)));
    } else {
        kpanic_fmt("JitX86CodeGen::compareReg reg width %d", regWidth);
    }
    if (!result) {
        result = getTmpReg8();
    }
    compiler.setz(R8(result));
    compiler.movzx(R32(result), R8(get8bitReg(result)));
    return result;
}

RegPtr JitX86CodeGen::compareValue(JitWidth regWidth, RegPtr reg, U32 value, JitEvaluate condition, RegPtr result) {
    if (regWidth == JitWidth::b32) {
        compiler.cmp(R32(reg), value);
    } else if (regWidth == JitWidth::b16) {
        compiler.cmp(R16(reg), (U16)value);
    } else if (regWidth == JitWidth::b8) {
        compiler.cmp(R8(get8bitReg(reg)), (U8)value);
    } else {
        kpanic_fmt("JitX86CodeGen::compareValue reg width %d", regWidth);
    }
    if (!result) {
        result = getTmpReg8();
    }
    setCC(R8(result), condition);
    compiler.movzx(R32(result), R8(get8bitReg(result)));
    return result;
}

void JitX86CodeGen::xaddReg(JitWidth regWidth, RegPtr reg, RegPtr rm) {
    if (regWidth == JitWidth::b32) {
        compiler.xadd(R32(rm), R32(reg));
    } else if (regWidth == JitWidth::b16) {
        compiler.xadd(R16(rm), R16(reg));
    } else if (regWidth == JitWidth::b8) {
        compiler.xadd(R8(get8bitReg(rm)), R8(get8bitReg(reg)));
    } else {
        kpanic("JitX86CodeGen::xaddReg");
    }
}

void JitX86CodeGen::mulReg(JitWidth regWidth, RegPtr reg) {
    if (regWidth == JitWidth::b32) {
        // EDX:EAX = (U64)EAX * src;
        if (regUsed[0] || regUsed[2]) {
            kpanic("JitX86CodeGen::mulReg 32");
        }
        RegPtr eax = getReg(0);
        RegPtr edx = getReg(2);
        regUsed[2] = true;
        regUsed[0] = true;
        compiler.mov(regEax, R32(eax));
        compiler.mul(R32(reg));
        compiler.mov(R32(eax), regEax);
        compiler.mov(R32(edx), regEdx);
        regUsed[2] = false;
        regUsed[0] = false;
    } else if (regWidth == JitWidth::b16) {
        // DX:AX = AX * src;
        if (regUsed[0] || regUsed[2]) {
            kpanic("JitX86CodeGen::mulReg 16");
        }
        RegPtr eax = getReg(0);
        RegPtr edx = getReg(2);
        regUsed[2] = true;
        regUsed[0] = true;
        compiler.mov(regEax, R32(eax));
        compiler.mul(R16(reg));
        compiler.mov(R16(eax), regAx);
        compiler.mov(R16(edx), regDx);
        regUsed[2] = false;
        regUsed[0] = false;
    } else if (regWidth == JitWidth::b8) {
        // AX = AL * src;
        if (regUsed[0]) {
            kpanic("JitX86CodeGen::mulReg 8");
        }
        RegPtr eax = getReg(0);
        compiler.mov(regAx, R16(eax));
        compiler.mul(R8(get8bitReg(reg)));
        compiler.mov(R16(eax), regAx);
    } else {
        kpanic("JitX86CodeGen::mulReg");
    }
}

void JitX86CodeGen::imulRR(JitWidth regWidth, RegPtr dst, RegPtr src, RegPtr overflow) {
    if (regWidth == JitWidth::b32) {
        if (!overflow) {
            compiler.imul(R32(dst), R32(src));
        } else {
#ifdef BOXEDWINE_64
            compiler.movsxd(R64(src), R32(src));
            compiler.movsxd(R64(dst), R32(dst));
            compiler.imul(R64(dst), R64(src));
            compiler.mov(R64(overflow), R64(dst));
            compiler.shr(R64(overflow), 32);
            compiler.mov(R32(dst), R32(dst)); // clear out top 32-bits
            compiler.mov(R32(src), R32(src)); // clear out top 32-bits (undo prev sign extend)
#else
            if (dst->hardwareReg() != 0 || overflow->hardwareReg() != 2) {
                kpanic("JitX86CodeGen::imulRRI overflow");
            }
            compiler.imul(R32(src));
#endif
        }
    } else if (regWidth == JitWidth::b16) {
        if (overflow) {
            kpanic("JitX86CodeGen::imulRR overflow");
        }
        compiler.imul(R16(dst), R16(src));
    } else {
        kpanic("JitX86CodeGen::imulRR");
    }
}

void JitX86CodeGen::imulRRI(JitWidth regWidth, RegPtr dst, RegPtr src, U32 src2, RegPtr overflow) {    
    if (regWidth == JitWidth::b32) {
        if (!overflow) {
            compiler.imul(R32(dst), R32(src), src2);
        } else {
#ifdef BOXEDWINE_64
            compiler.movsxd(R64(src), R32(src));
            compiler.movsxd(R64(dst), R32(dst));
            compiler.imul(R64(dst), R64(src), src2);
            compiler.mov(R64(overflow), R64(dst));
            compiler.shr(R64(overflow), 32);
            compiler.mov(R32(dst), R32(dst)); // clear out top 32-bits
            compiler.mov(R32(src), R32(src)); // clear out top 32-bits (undo prev sign extend)
#else
            if (dst->hardwareReg() != 0 || overflow->hardwareReg() != 2) {
                kpanic("JitX86CodeGen::imulRRI overflow");
            }
            compiler.mov(regEax, R32(src));
            RegPtr tmp = getTmpReg();
            movValue(JitWidth::b32, tmp, src2);
            compiler.imul(R32(tmp));
#endif
        }
    } else if (regWidth == JitWidth::b16) {
        if (overflow) {
            kpanic("JitX86CodeGen::imulRRI overflow");
        }
        compiler.imul(R16(dst), R16(src), src2);
    } else {
        kpanic("JitX86CodeGen::imulRRI");
    }
}

void JitX86CodeGen::imulReg(JitWidth regWidth, RegPtr reg) {
    if (regWidth == JitWidth::b32) {
        // EDX:EAX = (S64)((S32)EAX) * ((S32)(src));
        if (regUsed[0] || regUsed[2]) {
            kpanic("JitX86CodeGen::imulReg 32");
        }
        RegPtr eax = getReg(0);
        RegPtr edx = getReg(2);
        regUsed[2] = true;
        regUsed[0] = true;
        compiler.mov(regEax, R32(eax));
        compiler.imul(R32(reg));
        compiler.mov(R32(eax), regEax);
        compiler.mov(R32(edx), regEdx);
        regUsed[2] = false;
        regUsed[0] = false;
    } else if (regWidth == JitWidth::b16) {
        // DX:AX = (S32)((S16)AX) * (S16)src;
        if (regUsed[0] || regUsed[2]) {
            kpanic("JitX86CodeGen::imulReg 16");
        }
        RegPtr eax = getReg(0);
        RegPtr edx = getReg(2);
        regUsed[2] = true;
        regUsed[0] = true;
        compiler.mov(regEax, R32(eax));
        compiler.imul(R16(reg));
        compiler.mov(R16(eax), regAx);
        compiler.mov(R16(edx), regDx);
        regUsed[2] = false;
        regUsed[0] = false;
    } else if (regWidth == JitWidth::b8) {
        // AX = (S16)((S8)AL) * (S8)(src);
        if (regUsed[0]) {
            kpanic("JitX86CodeGen::imulReg 8");
        }
        RegPtr eax = getReg(0);
        compiler.mov(regAx, R16(eax));
        compiler.imul(R8(get8bitReg(reg)));
        compiler.mov(R16(eax), regAx);
    } else {
        kpanic("JitX86CodeGen::imulReg");
    }
}

void JitX86CodeGen::absReg(JitWidth regWidth, RegPtr reg) {
    RegPtr tmp = getTmpReg();
#ifdef BOXEDWINE_64
    compiler.mov(R64(tmp), R64(reg));
#else
    compiler.mov(R32(tmp), R32(reg));    
#endif
    if (regWidth == JitWidth::b8) {
        compiler.neg(R8(reg));
    } else if (regWidth == JitWidth::b16) {
        compiler.neg(R16(reg));
    } else if (regWidth == JitWidth::b32) {
        compiler.neg(R32(reg));
#ifdef BOXEDWINE_64
    } else if (regWidth == JitWidth::b64) {
        compiler.neg(R64(reg));
        compiler.cmovl(R64(reg), R64(tmp));
        return;
#endif
    } else {
        kpanic("JitX86CodeGen::divRegRegWithRemainder");
    }
    compiler.cmovl(R32(reg), R32(tmp));
}

void JitX86CodeGen::clzReg(JitWidth regWidth, RegPtr result, RegPtr reg) {
    if (regWidth == JitWidth::b16) {
        compiler.lzcnt(R16(result), R16(reg));
    } else if (regWidth == JitWidth::b32) {
        compiler.lzcnt(R32(result), R32(reg));
#ifdef BOXEDWINE_64
    } else if (regWidth == JitWidth::b64) {
        compiler.lzcnt(R64(result), R64(reg));
#endif
    } else {
        kpanic("JitX86CodeGen::clzReg");
    }
}

void JitX86CodeGen::divRegRegWithRemainder(JitWidth regWidth, RegPtr dest, RegPtr destHighAndRemainder, RegPtr src) {
    if (regWidth == JitWidth::b8) {
        compiler.div(R8(get8bitReg(src)));
    } else if (regWidth == JitWidth::b16) {
        compiler.div(R16(src));
    } else if (regWidth == JitWidth::b32) {
        compiler.div(R32(src));
    } else {
        kpanic("JitX86CodeGen::divRegRegWithRemainder");
    }    
}

void JitX86CodeGen::idivRegRegWithRemainder(JitWidth regWidth, RegPtr dest, RegPtr destHighAndRemainder, RegPtr src) {
    if (regWidth == JitWidth::b8) {
        compiler.idiv(R8(get8bitReg(src)));
    } else if (regWidth == JitWidth::b16) {
        compiler.idiv(R16(src));
    } else if (regWidth == JitWidth::b32) {
        compiler.idiv(R32(src));
    } else {
        kpanic("JitX86CodeGen::idivRegRegWithRemainder");
    }
}

#include "../../softmmu/kmemory_soft.h"
void JitX86CodeGen::readMMU(RegPtr dest, RegPtr index, U32 offset) {
#ifdef BOXEDWINE_64
    compiler.mov(R64(dest), Mem(HOST_MMU, RN(index), 3, offset));
#else
    compiler.mov(R32(dest), Mem((U32)getMemData(KThread::currentThread()->memory)->mmu, RN(index), 2));
#endif
}

void JitX86CodeGen::readMMU(RegPtr dest, U32 index) {
#ifdef BOXEDWINE_64
    compiler.mov(R64(dest), Mem(HOST_MMU, index * 8));
#else
    compiler.mov(R32(dest), Mem((U32)getMemData(KThread::currentThread()->memory)->mmu + index * 4));
#endif
}

Mem JitX86CodeGen::createMem(JitWidth width, MemPtr mem) {
    if (!mem->rm) {
        kpanic("JitX86CodeGen::createMem rm");
    }
    U32 disp = mem->offset;
    if (disp > 0x7fffff) {
        RegPtr tmp = getTmpReg();
        movValue(JitWidth::b32, tmp, disp);
        addReg(JitWidth::b64, tmp, mem->rm);

        if (width == JitWidth::b32) {            
            if (mem->sib) {
                return Mem32(RN(tmp), RN(mem->sib), mem->lsl, 0);
            }
            return Mem32(RN(tmp));
        }
        if (width == JitWidth::b16) {
            if (mem->sib) {
                return Mem16(RN(tmp), RN(mem->sib), mem->lsl, 0);
            }
            return Mem16(RN(tmp));
        }
        if (width == JitWidth::b8) {
            if (mem->sib) {
                return Mem8(RN(tmp), RN(mem->sib), mem->lsl, 0);
            }
            return Mem8(RN(tmp));
        }
        if (width == JitWidth::b64) {
            if (mem->sib) {
                return Mem64(RN(tmp), RN(mem->sib), mem->lsl, 0);
            }
            return Mem64(RN(tmp));
        }
        if (width == JitWidth::b128) {
            if (mem->sib) {                
                return Mem128(RN(tmp), RN(mem->sib), mem->lsl, 0);
            }
            return Mem128(RN(tmp));
        }
        kpanic_fmt("JitX86CodeGen::createMem unexpected width: %d", (U32)width);
    }
    if (width == JitWidth::b32) {
        if (mem->sib) {
            return Mem32(RN(mem->rm), RN(mem->sib), mem->lsl, mem->offset);
        }
        return Mem32(RN(mem->rm), mem->offset);
    }
    if (width == JitWidth::b16) {
        if (mem->sib) {
            return Mem16(RN(mem->rm), RN(mem->sib), mem->lsl, mem->offset);
        }
        return Mem16(RN(mem->rm), mem->offset);
    }
    if (width == JitWidth::b8) {
        if (mem->sib) {
            return Mem8(RN(mem->rm), RN(mem->sib), mem->lsl, mem->offset);
        }
        return Mem8(RN(mem->rm), mem->offset);
    }
    if (width == JitWidth::b64) {
        if (mem->sib) {
            return Mem64(RN(mem->rm), RN(mem->sib), mem->lsl, mem->offset);
        }
        return Mem64(RN(mem->rm), mem->offset);
    }
    if (width == JitWidth::b128) {
        if (mem->sib) {
            return Mem128(RN(mem->rm), RN(mem->sib), mem->lsl, mem->offset);
        }
        return Mem128(RN(mem->rm), mem->offset);
    }
    kpanic_fmt("JitX86CodeGen::createMem unexpected width: %d", (U32)width);
    return Mem();
}

void JitX86CodeGen::readHost(JitWidth width, MemPtr address, RegPtr result, bool emlulatedMemory) {
    compiler.mov(R(width, result), createMem(width, address));
}

void JitX86CodeGen::writeHost(JitWidth width, MemPtr address, RegPtr src, bool emlulatedMemory) {
    compiler.mov(createMem(width, address), R(width, src));
}

void JitX86CodeGen::writeHost(JitWidth width, MemPtr address, U32 value, bool emlulatedMemory) {
    compiler.mov(createMem(width, address), value);
}

RegPtr JitX86CodeGen::readCPU(JitWidth width, U32 offset, RegPtr reg) {
    if (!reg) {
        if (width == JitWidth::b8) {
            reg = getTmpReg8();
        } else {
            reg = getTmpReg();
        }
    }
    // mov reg, [edi+srcOffset]    
    if (width == JitWidth::b32) {
        compiler.mov(R32(reg), Mem(HOST_CPU, offset));
    } else if (width == JitWidth::b16) {
        compiler.mov(R16(reg), Mem(HOST_CPU, offset));
    } else if (width == JitWidth::b8) {
        compiler.mov(R8(get8bitReg(reg)), Mem(HOST_CPU, offset));
#ifdef BOXEDWINE_64
    } else if (width == JitWidth::b64) {
        compiler.mov(R64(reg), Mem(HOST_CPU, offset));
#endif
    } else {
        kpanic_fmt("unknown dstWidth in JitX86CodeGen::readCPU %d", width);
    }
    return reg;
}

RegPtr JitX86CodeGen::readCPU(JitWidth width, RegPtr sib, U8 lsl, U32 offset, RegPtr reg) {
    if (!reg) {
        if (width == JitWidth::b8) {
            reg = getTmpReg8();
        } else {
            reg = getTmpReg();
        }
    }
    if (width == JitWidth::b32) {
        compiler.mov(R32(reg), Mem(HOST_CPU, RN(sib), lsl, offset));
    } else if (width == JitWidth::b16) {
        compiler.mov(R16(reg), Mem(HOST_CPU, RN(sib), lsl, offset));
    } else if (width == JitWidth::b8) {
        compiler.mov(R8(get8bitReg(reg)), Mem(HOST_CPU, RN(sib), lsl, offset));
#ifdef BOXEDWINE_64
    } else if (width == JitWidth::b64) {
        compiler.mov(R64(reg), Mem(HOST_CPU, RN(sib), lsl, offset));
#endif
    } else {
        kpanic_fmt("unknown dstWidth in JitX86CodeGen::readCPU %d", width);
    }
    return reg;
}

void JitX86CodeGen::writeCPUValue(JitWidth width, RegPtr sib, U8 lsl, U32 offset, DYN_PTR_SIZE src) {
    if (width == JitWidth::b32) {
        compiler.mov(Mem32(HOST_CPU, RN(sib), lsl, offset), (U32)src);
    } else if (width == JitWidth::b16) {
        compiler.mov(Mem16(HOST_CPU, RN(sib), lsl, offset), (U16)src);
    } else if (width == JitWidth::b8) {
        compiler.mov(Mem8(HOST_CPU, RN(sib), lsl, offset), (U8)src);
    } else if (width == JitWidth::b64) {
        compiler.mov(Mem64(HOST_CPU, RN(sib), lsl, offset), src);
    } else {
        kpanic_fmt("unknown dstWidth in JitX86CodeGen::writeCPUValue %d", width);
    }
}

void JitX86CodeGen::writeCPUValue(JitWidth width, U32 offset, DYN_PTR_SIZE src) {
    if (width == JitWidth::b32) {
        compiler.mov(Mem32(HOST_CPU, offset), (U32)src);
    } else if (width == JitWidth::b16) {
        compiler.mov(Mem16(HOST_CPU, offset), (U16)src);
    } else if (width == JitWidth::b8) {
        compiler.mov(Mem8(HOST_CPU, offset), (U8)src);
    } else if (width == JitWidth::b64) {
        RegPtr tmp = getTmpReg();
        compiler.mov(R64(tmp), src);
        compiler.mov(Mem64(HOST_CPU, offset), R64(tmp));
    } else {
        kpanic_fmt("unknown dstWidth in JitX86CodeGen::writeCPUValue %d", width);
    }
}

void JitX86CodeGen::mov(JitWidth regWidth, RegPtr dest, RegPtr src) {
    if (regWidth == JitWidth::b32) {
        compiler.mov(R32(dest), R32(src));
    } else if (regWidth == JitWidth::b16) {
        compiler.mov(R16(dest), R16(src));
    } else if (regWidth == JitWidth::b8) {
        compiler.mov(R8(get8bitReg(dest)), R8(get8bitReg(src)));
#ifdef BOXEDWINE_64
    } else if (regWidth == JitWidth::b64) {
        compiler.mov(R64(dest), R64(src));
#endif
    } else {
        kpanic_fmt("JitX86CodeGen::mov unexpected width: %d", (U32)regWidth);
    }
}

void JitX86CodeGen::movValue(JitWidth regWidth, RegPtr dst, DYN_PTR_SIZE imm) {
    if (regWidth == JitWidth::b32) {
        compiler.mov(R32(dst), (U32)imm);
    } else if (regWidth == JitWidth::b16) {
        compiler.mov(R16(dst), (U16)imm);
    } else if (regWidth == JitWidth::b8) {
        compiler.mov(R8(get8bitReg(dst)), (U8)imm);
#ifdef BOXEDWINE_64
    } else if (regWidth == JitWidth::b64) {
        compiler.mov(R64(dst), imm);
#endif
    } else {
        kpanic_fmt("JitX86CodeGen::mov unexpected width: %d", (U32)regWidth);
    }
}

void JitX86CodeGen::movzx(JitWidth dstWidth, RegPtr dest, JitWidth srcWidth, RegPtr src) {
    if (dstWidth == JitWidth::b32) {
        if (srcWidth == JitWidth::b16) {
            compiler.movzx(R32(dest), R16(src));
        } else if (srcWidth == JitWidth::b8) {
            compiler.movzx(R32(dest), R8(get8bitReg(src)));
            dest->isHigh = false; // for when src == dest
        } else {
            kpanic_fmt("unknown width in JitX86CodeGen::movzx %d <= %d", dstWidth, srcWidth);
        }
    } else if (dstWidth == JitWidth::b16) {
        if (srcWidth == JitWidth::b8) {
            compiler.movzx(R16(dest), R8(get8bitReg(src)));
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
            compiler.movsx(R32(dest), R16(src));
        } else if (srcWidth == JitWidth::b8) {
            compiler.movsx(R32(dest), R8(get8bitReg(src)));
            dest->isHigh = false; // for when src == dest
        } else {
            kpanic_fmt("unknown width in JitX86CodeGen::movsx %d <= %d", dstWidth, srcWidth);
        }
    } else if (dstWidth == JitWidth::b16) {
        if (srcWidth == JitWidth::b8) {
            compiler.movsx(R16(dest), R8(get8bitReg(src)));
            dest->isHigh = false; // for when src == dest
        } else {
            kpanic_fmt("unknown width in JitX86CodeGen::movsx %d <= %d", dstWidth, srcWidth);
        }
    } else {
        kpanic_fmt("unknown width in JitX86CodeGen::movsx %d <= %d", dstWidth, srcWidth);
    }
}

#ifdef BOXEDWINE_64
void JitX86CodeGen::setParam(asmjit::x86::Gp reg, const DynParam& param) {
    switch (param.type) {
    case JitCallParamType::REG_8:
        compiler.movzx(R32(reg.id()), R8(get8bitReg(param.reg)));
        break;
    case JitCallParamType::REG_16:
        compiler.movzx(R32(reg.id()), R16(param.reg));
        break;
    case JitCallParamType::REG_32:
        compiler.mov(R32(reg.id()), R32(param.reg));
        break;
    case JitCallParamType::CPU:
        compiler.mov(reg, HOST_CPU);
        break;
    case JitCallParamType::CONST_8:
        compiler.mov(R32(reg.id()), (U32)(param.value & 0xFF));
        break;
    case JitCallParamType::CONST_16:
        compiler.mov(R32(reg.id()), (U32)(param.value & 0xFFFF));
        break;
    case JitCallParamType::CONST_32:
        compiler.mov(R32(reg.id()), (U32)param.value);
        break;
    case JitCallParamType::CONST_PTR:
        compiler.mov(reg, param.value);
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
        compiler.movzx(R32(param.reg), R8(get8bitReg(param.reg)));
        compiler.push(R32(param.reg));
        break;
    case JitCallParamType::REG_16:
        compiler.movzx(R32(param.reg), R16(param.reg));
        compiler.push(R32(param.reg));
        break;
    case JitCallParamType::REG_32:
        compiler.push(R32(param.reg));
        break;
    case JitCallParamType::CPU:
        compiler.push(HOST_CPU);
        break;
    case JitCallParamType::CONST_8:
        compiler.push((U32)(param.value & 0xFF));
        break;
    case JitCallParamType::CONST_16:
        compiler.push((U32)(param.value & 0xFFFF));
        break;
    case JitCallParamType::CONST_32:
        compiler.push(param.value);
        break;
    case JitCallParamType::CONST_PTR:
        compiler.push((U32)param.value);
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
    std::vector<asmjit::x86::Gp> needToPush;

    for (int i = 0; i < 4 && i < (int)params.size(); i++) {
        if (params[i].usesReg() && clobberedReg[params[i].reg->hardwareReg()]) {
            needToPush.push_back(R64(params[i].reg));
            pushedReg[i] = true;
        }
        clobberedReg[this->params[i].id()] = true;
    }
    if (needToPush.size()) {
        for (int i = (int)needToPush.size() - 1; i >= 0; i--) {
            compiler.push(needToPush[i]);
        }
    }

    for (int i = 0; i < 4 && i < (int)params.size(); i++) {
        if (params[i].usesReg() && pushedReg[i]) {
            compiler.pop(R64(this->params[i].id()));
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

void JitX86CodeGen::callLoadCache() {
    compiler.call((DYN_PTR_SIZE)cpu->thread->process->syncFromHost);
}

void JitX86CodeGen::callWriteCache() {
    compiler.call((DYN_PTR_SIZE)cpu->thread->process->syncToHost);
}

void JitX86CodeGen::callHostFunction(void* address, const std::vector<DynParam>& params, bool restoreCache, bool saveCache) {
    U32 stackAdjust = 0;
    std::vector<U8> pushedRegs;

    for (int i = 0; i < NUMBER_OF_REGS; i++) {
        if (isVolitile[i] && isTmp[i] && regUsed[i]) {
            compiler.push(RN(i));
            pushedRegs.push_back(i);
        }
    }
    if (saveCache) {
        callWriteCache();
    }
    setParams(params);    
#ifdef BOXEDWINE_64   
    if ((pushedRegs.size() % 2) == 1) {
        stackAdjust = 8;
    }
    // part of the x64 windows ABI, shadow store
#ifdef BOXEDWINE_MSVC
    compiler.sub(asmjit::x86::rsp, 32 + stackAdjust);
#else
    if (stackAdjust) {
        compiler.push(PARAM_CALL_TMP);
    }
#endif
    compiler.mov(PARAM_CALL_TMP, (U64)address);
    compiler.call(PARAM_CALL_TMP);
#ifdef BOXEDWINE_MSVC
    compiler.add(asmjit::x86::rsp, 32 + stackAdjust);
#else
    if (stackAdjust) {
        compiler.pop(PARAM_CALL_TMP);
    }
#endif
    if (params.size() > 4) {
        kpanic("JitX86CodeGen::callHostFunction x64 doesn't support passing more than 4 parameters");
    }
#else
    compiler.call(address);

    if (params.size()) {
        compiler.add(regEsp, sizeof(void*) * params.size());
    }
#endif
    if (restoreCache) {
        callLoadCache();
    }
    for (int i = (int)pushedRegs.size() - 1; i >= 0; i--) {
        compiler.pop(RN(pushedRegs[i]));
    }
}

void JitX86CodeGen::nakedCall(RegPtr reg) {
    compiler.call(RN(reg));
}

void JitX86CodeGen::nakedReturn() {
    compiler.ret();
}

void JitX86CodeGen::callHostFunctionWithResult(RegPtr result, void* address, const std::vector<DynParam>& params) {
    U32 stackAdjust = 0; // if 64-bit stack isn't aligned to 16-bytes, things like FPU::F2XM1()
    std::vector<U8> pushedRegs;

    for (int i = 0; i < NUMBER_OF_REGS; i++) {
        if (isVolitile[i] && isTmp[i] && regUsed[i] && result->hardwareReg() != i) {
            compiler.push(RN(i));
            pushedRegs.push_back(i);
        }
    }

    callWriteCache();
    setParams(params);
    
#ifdef BOXEDWINE_64
    if ((pushedRegs.size() % 2) == 1) {
        stackAdjust = 8;
    }
    // part of the x64 windows ABI, shadow store
#ifdef BOXEDWINE_MSVC
    compiler.sub(asmjit::x86::rsp, 32 + stackAdjust);
#else
    if (stackAdjust) {
        compiler.push(PARAM_CALL_TMP);
    }
#endif
    compiler.mov(PARAM_CALL_TMP, (U64)address);
    compiler.call(PARAM_CALL_TMP);
#ifdef BOXEDWINE_MSVC
    compiler.add(asmjit::x86::rsp, 32 + stackAdjust);
#else
    if (stackAdjust) {
        compiler.pop(PARAM_CALL_TMP);
    }
#endif
#else
    compiler.call(address);

    if (params.size()) {
        compiler.add(regEsp, sizeof(void*) * params.size());
    } 
#endif  
    compiler.mov(RN(result), RN(0));
    for (int i = (int)pushedRegs.size() - 1; i >= 0; i--) {
        compiler.pop(RN(pushedRegs[i]));
    }
    callLoadCache();
}

void JitX86CodeGen::If(JitWidth regWidth, RegPtr reg) {
    Label label = compiler.new_label();
    ifLabels.push_back(label);

    if (regWidth == JitWidth::b32) {
        compiler.test(R32(reg), R32(reg));
    } else if (regWidth == JitWidth::b16) {
        compiler.test(R16(reg), R16(reg));
    } else if (regWidth == JitWidth::b8) {
        compiler.test(R8(get8bitReg(reg)), R8(get8bitReg(reg)));
#ifdef BOXEDWINE_64
    } else if (regWidth == JitWidth::b64) {
        compiler.test(R64(reg), R64(reg));
#endif
    } else {
        kpanic_fmt("JitX86CodeGen::If unexpected width: %d", (U32)regWidth);
    }
    compiler.jz(label);
}

void JitX86CodeGen::IfTest(JitWidth regWidth, RegPtr reg, RegPtr mask) {
    Label label = compiler.new_label();
    ifLabels.push_back(label);

    if (regWidth == JitWidth::b8) {
        compiler.test(R32(get8bitReg(reg)), R32(get8bitReg(mask)));
    } else {
        compiler.test(R32(reg), R32(mask));
    }
    compiler.jz(label);
}

void JitX86CodeGen::IfTest(JitWidth regWidth, RegPtr reg, U32 value) {
    Label label = compiler.new_label();
    ifLabels.push_back(label);

    if (regWidth == JitWidth::b8) {
        compiler.test(R32(get8bitReg(reg)), value);
    } else {
        compiler.test(R32(reg), value);
    }
    compiler.jz(label);
}

void JitX86CodeGen::IfTestBit(JitWidth regWidth, RegPtr reg, U32 bitPos) {
    Label label = compiler.new_label();
    ifLabels.push_back(label);

    if (regWidth == JitWidth::b8) {
        compiler.test(R32(get8bitReg(reg)), 1 << bitPos);
    } else {
        compiler.test(R32(reg), 1 << bitPos);
    }
    compiler.jz(label);
}

void JitX86CodeGen::IfNotTestBit(JitWidth regWidth, RegPtr reg, U32 bitPos) {
    Label label = compiler.new_label();
    ifLabels.push_back(label);

    if (regWidth == JitWidth::b8) {
        compiler.test(R32(get8bitReg(reg)), 1 << bitPos);
    } else {
        compiler.test(R32(reg), 1 << bitPos);
    }
    compiler.jnz(label);
}

void JitX86CodeGen::IfEqual(JitWidth regWidth, RegPtr reg1, RegPtr reg2) {
    Label label = compiler.new_label();
    ifLabels.push_back(label);

    if (regWidth == JitWidth::b32) {
        compiler.cmp(R32(reg1), R32(reg2));
    } else if (regWidth == JitWidth::b16) {
        compiler.cmp(R16(reg1), R16(reg2));
    } else if (regWidth == JitWidth::b8) {
        compiler.cmp(R8(get8bitReg(reg1)), R8(get8bitReg(reg2)));
    } else {
        kpanic_fmt("JitX86CodeGen::IfEqual unexpected width: %d", (U32)regWidth);
    }
    compiler.jnz(label);
}

void JitX86CodeGen::IfEqual(JitWidth regWidth, RegPtr reg, DYN_PTR_SIZE value) {
    Label label = compiler.new_label();
    ifLabels.push_back(label);

    if (regWidth == JitWidth::b32) {
        compiler.cmp(R32(reg), (U32)value);
    } else if (regWidth == JitWidth::b16) {
        compiler.cmp(R16(reg), (U16)value);
    } else if (regWidth == JitWidth::b8) {
        compiler.cmp(R8(get8bitReg(reg)), (U8)value);
#ifdef BOXEDWINE_64
    } else if (regWidth == JitWidth::b64) {
        if (value <= 0xffffffffl) {
            compiler.cmp(R64(reg), (U32)value);
        } else {
            RegPtr r = getTmpReg();
            compiler.mov(R64(r), value);
            compiler.cmp(R64(reg), R64(r));
        }
#endif
    } else {
        kpanic_fmt("JitX86CodeGen::IfEqual unexpected width: %d", (U32)regWidth);
    }
    compiler.jnz(label);
}

void JitX86CodeGen::IfNotEqual(JitWidth regWidth, RegPtr reg, DYN_PTR_SIZE value) {
    Label label = compiler.new_label();
    ifLabels.push_back(label);

    if (regWidth == JitWidth::b32) {
        compiler.cmp(R32(reg), (U32)value);
    } else if (regWidth == JitWidth::b16) {
        compiler.cmp(R16(reg), (U16)value);
    } else if (regWidth == JitWidth::b8) {
        compiler.cmp(R8(get8bitReg(reg)), (U8)value);
#ifdef BOXEDWINE_64
    } else if (regWidth == JitWidth::b64) {
        if (value <= 0xffffffffl) {
            compiler.cmp(R64(reg), (U32)value);
        } else {
            RegPtr r = getTmpReg();
            compiler.mov(R64(r), value);
            compiler.cmp(R64(reg), R64(r));
        }
#endif
    } else {
        kpanic_fmt("JitX86CodeGen::IfNotEqual unexpected width: %d", (U32)regWidth);
    }
    compiler.jz(label);
}

void JitX86CodeGen::IfNotEqual(JitWidth regWidth, RegPtr reg, RegPtr reg2) {
    Label label = compiler.new_label();
    ifLabels.push_back(label);

    if (regWidth == JitWidth::b32) {
        compiler.cmp(R32(reg), R32(reg2));
    } else if (regWidth == JitWidth::b16) {
        compiler.cmp(R16(reg), R16(reg2));
    } else if (regWidth == JitWidth::b8) {
        compiler.cmp(R8(get8bitReg(reg)), R8(get8bitReg(reg2)));
    } else {
        kpanic_fmt("JitX86CodeGen::IfNotEqual unexpected width: %d", (U32)regWidth);
    }
    compiler.jz(label);
}

void JitX86CodeGen::IfLessThan2(JitWidth regWidth, RegPtr reg, U32 value) {
    Label label = compiler.new_label();
    ifLabels.push_back(label);

    if (regWidth == JitWidth::b32) {
        compiler.cmp(R32(reg), value);
    } else if (regWidth == JitWidth::b16) {
        compiler.cmp(R16(reg), (U16)value);
    } else if (regWidth == JitWidth::b8) {
        compiler.cmp(R8(get8bitReg(reg)), (U8)value);
    } else {
        kpanic_fmt("JitX86CodeGen::IfLessThan unexpected width: %d", (U32)regWidth);
    }
    compiler.jnb(label);
}

void JitX86CodeGen::IfLessThan2(JitWidth regWidth, RegPtr reg1, RegPtr reg2) {
    Label label = compiler.new_label();
    ifLabels.push_back(label);

    if (regWidth == JitWidth::b32) {
        compiler.cmp(R32(reg1), R32(reg2));
    } else if (regWidth == JitWidth::b16) {
        compiler.cmp(R16(reg1), R16(reg2));
    } else if (regWidth == JitWidth::b8) {
        compiler.cmp(R8(get8bitReg(reg1)), R8(get8bitReg(reg2)));
    } else {
        kpanic_fmt("JitX86CodeGen::IfLessThan unexpected width: %d", (U32)regWidth);
    }
    compiler.jnb(label);
}

void JitX86CodeGen::IfGreaterThanOrEqual(JitWidth regWidth, RegPtr reg1, RegPtr reg2) {
    Label label = compiler.new_label();
    ifLabels.push_back(label);

    if (regWidth == JitWidth::b32) {
        compiler.cmp(R32(reg1), R32(reg2));
    } else if (regWidth == JitWidth::b16) {
        compiler.cmp(R16(reg1), R16(reg2));
    } else if (regWidth == JitWidth::b8) {
        compiler.cmp(R8(get8bitReg(reg1)), R8(get8bitReg(reg2)));
    } else {
        kpanic_fmt("JitX86CodeGen::IfGreaterThanOrEqual unexpected width: %d", (U32)regWidth);
    }
    compiler.jb(label);
}

void JitX86CodeGen::IfGreaterThanOrEqual(JitWidth regWidth, RegPtr reg1, U32 value) {
    Label label = compiler.new_label();
    ifLabels.push_back(label);

    if (regWidth == JitWidth::b32) {
        compiler.cmp(R32(reg1), value);
    } else if (regWidth == JitWidth::b16) {
        compiler.cmp(R16(reg1), (U16)value);
    } else if (regWidth == JitWidth::b8) {
        compiler.cmp(R8(get8bitReg(reg1)), (U8)value);
    } else {
        kpanic_fmt("JitX86CodeGen::IfGreaterThanOrEqual unexpected width: %d", (U32)regWidth);
    }
    compiler.jb(label);
}


void JitX86CodeGen::IfNot(JitWidth regWidth, RegPtr reg) {
    Label label = compiler.new_label();
    ifLabels.push_back(label);

    if (regWidth == JitWidth::b32) {
        compiler.test(R32(reg), R32(reg));
    } else if (regWidth == JitWidth::b16) {
        compiler.test(R16(reg), R16(reg));
    } else if (regWidth == JitWidth::b8) {
        compiler.test(R8(get8bitReg(reg)), R8(get8bitReg(reg)));
#ifdef BOXEDWINE_64
    } else if (regWidth == JitWidth::b64) {
        compiler.test(R64(reg), R64(reg));
#endif
    } else {
        kpanic_fmt("JitX86CodeGen::IfNot unexpected width: %d", (U32)regWidth);
    }
    compiler.jnz(label);
}

void JitX86CodeGen::IfNotCPU(JitWidth regWidth, RegPtr sib, U8 lsl, U32 offset) {
    Label label = compiler.new_label();
    ifLabels.push_back(label);

    if (regWidth == JitWidth::b32) {
        compiler.cmp(Mem32(HOST_CPU, RN(sib), lsl, offset), 0);
    } else if (regWidth == JitWidth::b16) {
        compiler.cmp(Mem16(HOST_CPU, RN(sib), lsl, offset), 0);
    } else if (regWidth == JitWidth::b8) {
        compiler.cmp(Mem8(HOST_CPU, RN(sib), lsl, offset), 0);
    } else {
        kpanic_fmt("JitX86CodeGen::IfNotCPU unexpected width: %d", (U32)regWidth);
    }
    compiler.jnz(label);
}

void JitX86CodeGen::clearIfSpansPage(JitWidth width, RegPtr offset, RegPtr reg) {    
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
        kpanic_fmt("JitX86CodeGen::clearIfSpansPage unknown width %d", (U32)width);
    }    
    RegPtr tmpReg;
    
    if (offset.use_count() == 1) {
        tmpReg = offset;
    } else {
        tmpReg = getTmpReg();
    }
    movValue(JitWidth::b32, tmpReg, 0);
    compiler.cmovnl(RN(reg), RN(tmpReg));
}

void JitX86CodeGen::clearMMUPermissionIfSpansPage(JitWidth width, RegPtr offset, RegPtr reg) {
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
        kpanic_fmt("JitX86CodeGen::clearRegIfSpansPage unknown width %d", (U32)width);
    }
    // since esp is guaranteed to be aligned, it works to use it as a way to clear the bottom 2 bits
    compiler.cmovnl(RN(reg), regEsp);
}

void JitX86CodeGen::JumpIfCondition(JitConditional condition, U32 address) {
    Label label;

    bool negative = false;
    RegPtr reg;

    preIfCondition(condition, negative, reg);

    if (!opLabels.get(address, label)) {
        label = compiler.new_label();
        opLabels.set(address, label);
    }
    compiler.test(R32(reg), R32(reg));
    if (negative) {
        compiler.jz(label);
    } else {
        compiler.jnz(label);
    }
}

void JitX86CodeGen::IfDF() {
    Label label = compiler.new_label();
    ifLabels.push_back(label);

    compiler.test(Mem32(HOST_CPU, offsetof(CPU, flags)), DF);
    compiler.jz(label);

    //compiler.bt(Mem(HOST_CPU, offsetof(CPU, flags)), 10);
    //compiler.jnb();
}

void JitX86CodeGen::IfSmallStack() {
    RegPtr reg = getTmpReg();
    compiler.mov(R32(reg), Mem(HOST_CPU, offsetof(CPU, stackNotMask)));
    If(JitWidth::b32, reg);
}

U32 JitX86CodeGen::MarkJumpLocation() {
    Label label = compiler.new_label();
    labels.push_back(label);
    compiler.bind(label);
    return (U32)labels.size();
}

void JitX86CodeGen::Goto(U32 location) {
    compiler.jmp(labels[location - 1]);
}

void JitX86CodeGen::jmpHost(DYN_PTR_SIZE address) {
    compiler.jmp(address);
}

void JitX86CodeGen::jmpHost(RegPtr reg) {
    compiler.jmp(RN(reg));
}

void JitX86CodeGen::writeCPU(JitWidth width, U32 offset, RegPtr src) {
    if (width == JitWidth::b32) {
        compiler.mov(Mem(HOST_CPU, offset), R32(src));
    } else if (width == JitWidth::b16) {
        compiler.mov(Mem(HOST_CPU, offset), R16(src));
    } else if (width == JitWidth::b8) {
        compiler.mov(Mem(HOST_CPU, offset), R8(get8bitReg(src)));
#ifdef BOXEDWINE_64
    } else if (width == JitWidth::b64) {
        compiler.mov(Mem(HOST_CPU, offset), R64(src));
#endif
    } else {
        kpanic_fmt("unknown dstWidth in JitX86CodeGen::writeCPU %d", width);
    }
}

void JitX86CodeGen::writeCPU(JitWidth width, RegPtr sib, U8 lsl, U32 offset, RegPtr src) {
    if (width == JitWidth::b32) {
        compiler.mov(Mem(HOST_CPU, RN(sib), lsl, offset), R32(src));
    } else if (width == JitWidth::b16) {
        compiler.mov(Mem(HOST_CPU, RN(sib), lsl, offset), R16(src));
    } else if (width == JitWidth::b8) {
        compiler.mov(Mem(HOST_CPU, RN(sib), lsl, offset), R8(get8bitReg(src)));
#ifdef BOXEDWINE_64
    } else if (width == JitWidth::b64) {
        compiler.mov(Mem(HOST_CPU, RN(sib), lsl, offset), R64(src));
#endif
    } else {
        kpanic_fmt("unknown dstWidth in JitX86CodeGen::writeCPU %d", width);
    }
}

void JitX86CodeGen::IfSseLessThan(SSERegPtr src1, SSERegPtr src2) {
    Label label = compiler.new_label();
    ifLabels.push_back(label);

    compiler.ucomiss(XMM(src1->hardwareReg()), XMM(src2->hardwareReg()));
    compiler.jb(label);
}

void JitX86CodeGen::storeCpuXMMReg(SSERegPtr reg, U32 index) {
    if (index >= 8) {
        kpanic("JitX86CodeGen::storeCpuXMMReg");
        return;
    }
    if (xmmCache[index] == INVALID_REG) {
        compiler.movaps(Mem(HOST_CPU, index * 16 + offsetof(CPU, xmm)), XMM(reg->hardwareReg()));
    } else if (xmmCache[index] != reg->hardwareReg()) {
        compiler.movaps(XMM(xmmCache[index]), XMM(reg->hardwareReg()));
    }    
}

void JitX86CodeGen::storeXMMToMem128(SSERegPtr reg, MemPtr address) {
    compiler.movups(createMem(JitWidth::b128, address), XMM(reg->hardwareReg()));
}

void JitX86CodeGen::storeXMMToMem64(SSERegPtr reg, MemPtr address) {
    compiler.movlps(createMem(JitWidth::b64, address), XMM(reg->hardwareReg()));
}

void JitX86CodeGen::storeXMMToMem32(SSERegPtr reg, MemPtr address) {
    compiler.movss(createMem(JitWidth::b32, address), XMM(reg->hardwareReg()));
}

void JitX86CodeGen::storeHighXMMToMem64(SSERegPtr reg, MemPtr address) {
    compiler.movhps(createMem(JitWidth::b64, address), XMM(reg->hardwareReg()));
}

SSERegPtr JitX86CodeGen::getXMM(U8 index, bool load) {
    if (index != INVALID_REG && index >= 8) {
        kpanic("JitX86CodeGen::getXMM");
        return nullptr;
    }
    if (index != INVALID_REG && xmmCache[index] != INVALID_REG) {
        return std::make_shared<SSERegInternal>(xmmCache[index], index);
    }
    SSERegPtr result = std::shared_ptr<SSERegInternal>(new SSERegInternal(findTmpXMM(), index), [this](SSERegInternal* p) {
        xmmUsed[p->hardwareReg()] = false;
        delete p;
    });
    if (load && index != INVALID_REG) {
        compiler.movaps(XMM(result->hardwareReg()), Mem(HOST_CPU, index * 16 + offsetof(CPU, xmm)));
    }
    return result;
}

SSERegPtr JitX86CodeGen::loadCpuXMMReg(U8 index) {        
    return getXMM(index, true);
}

SSERegPtr JitX86CodeGen::loadXMMFromMem128(U8 index, MemPtr address) {
    SSERegPtr reg = getXMM(index, false);
    compiler.movups(XMM(reg->hardwareReg()), createMem(JitWidth::b128, address));
    return reg;
}

SSERegPtr JitX86CodeGen::loadXMMFromMem64(U8 index, MemPtr address) {
    SSERegPtr reg = getXMM(index, true);
    compiler.movq(XMM(reg->hardwareReg()), createMem(JitWidth::b64, address));
    return reg;
}

SSERegPtr JitX86CodeGen::loadLowXMMFromMem64(U8 index, MemPtr address) {
    SSERegPtr reg = getXMM(index, true);
    compiler.movlps(XMM(reg->hardwareReg()), createMem(JitWidth::b64, address));
    return reg;
}

SSERegPtr JitX86CodeGen::loadHighXMMFromMem64(U8 index, MemPtr address) {
    SSERegPtr reg = getXMM(index, true);
    compiler.movhps(XMM(reg->hardwareReg()), createMem(JitWidth::b64, address));
    return reg;
}

SSERegPtr JitX86CodeGen::loadXMMFromMem32(U8 index, MemPtr address) {
    SSERegPtr reg = getXMM(index, true);
    compiler.movss(XMM(reg->hardwareReg()), createMem(JitWidth::b32, address));
    return reg;
}

void JitX86CodeGen::addpsXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.addps(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::addssXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.addss(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::subpsXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.subps(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::subssXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.subss(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::mulpsXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.mulps(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::mulssXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.mulss(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::divpsXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.divps(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::divssXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.divss(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::rcppsXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.rcpps(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::rcpssXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.rcpss(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::sqrtpsXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.sqrtps(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::sqrtssXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.sqrtss(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::rsqrtpsXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.rsqrtps(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::rsqrtssXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.rsqrtss(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::maxpsXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.maxps(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::maxssXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.maxss(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::minpsXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.minps(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::minssXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.minss(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::pavgbMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    compiler.pavgb(getMMXReg(dst), getMMXReg(src));
}

void JitX86CodeGen::pavgwMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    compiler.pavgw(getMMXReg(dst), getMMXReg(src));
}

void JitX86CodeGen::psadbwMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    compiler.psadbw(getMMXReg(dst), getMMXReg(src));
}

void JitX86CodeGen::pextrwRegMmx(RegPtr dst, MMXRegPtr src, U8 srcIndex) {
    compiler.pextrw(R32(dst), getMMXReg(src), srcIndex);
}

void JitX86CodeGen::pinsrwMmxReg(MMXRegPtr dst, RegPtr src, U8 dstIndex) {
    compiler.pinsrw(getMMXReg(dst), R32(src->hardwareReg()), dstIndex);
}

void JitX86CodeGen::pmaxswMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    compiler.pmaxsw(getMMXReg(dst), getMMXReg(src));
}

void JitX86CodeGen::pmaxubMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    compiler.pmaxub(getMMXReg(dst), getMMXReg(src));
}

void JitX86CodeGen::pminswMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    compiler.pminsw(getMMXReg(dst), getMMXReg(src));
}

void JitX86CodeGen::pminubMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    compiler.pminub(getMMXReg(dst), getMMXReg(src));
}

void JitX86CodeGen::pmovmskbMmxMmx(RegPtr dst, MMXRegPtr src) {
    compiler.pmovmskb(R32(dst), getMMXReg(src));
}

void JitX86CodeGen::pmulhuwMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    compiler.pmulhuw(getMMXReg(dst), getMMXReg(src));
}

void JitX86CodeGen::pshufwMmxMmx(MMXRegPtr dst, MMXRegPtr src, U8 order) {
    compiler.pshuflw(getMMXReg(dst), getMMXReg(src), order);
}

void JitX86CodeGen::maskmovq(MMXRegPtr src, MMXRegPtr mask, MemPtr destAddress) {
    currentOp->flags2 |= OP_FLAG2_SAVED_TMP_REG;
#ifndef BOXEDWINE_64
    static U32 tmp7;
    compiler.mov(Mem((U64)&tmp7), RN(7));
#endif
    compiler.mov(Mem(HOST_CPU, offsetof(CPU, tmpReg)), RN(7));
    compiler.mov(RN(7), RN(calculateAddress(destAddress)));
    // this works because the top 64-bits of the mask should be 0's since its used for MMX
    compiler.maskmovdqu(getMMXReg(src), getMMXReg(mask));
#ifndef BOXEDWINE_64
    compiler.mov(RN(7), Mem((U64)&tmp7));
#endif
    compiler.mov(RN(7), Mem(HOST_CPU, offsetof(CPU, tmpReg)));
}

void JitX86CodeGen::paddqMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    compiler.paddq(getMMXReg(dst), getMMXReg(src));
}

void JitX86CodeGen::psubqMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    compiler.psubq(getMMXReg(dst), getMMXReg(src));
}

void JitX86CodeGen::pmuludqMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    compiler.pmuludq(getMMXReg(dst), getMMXReg(src));
}

void JitX86CodeGen::andnpsXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.andnps(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::andpsXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.andps(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::orpsXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.orps(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::xorpsXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.xorps(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::cvtpi2psXmmMmx(SSERegPtr dst, MMXRegPtr src) {
    // cvtpi2ps need to keep top 64-bits of the xmm dst
    SSERegPtr tmp = getTmpSSE();
    compiler.cvtdq2ps(XMM(tmp->hardwareReg()), getMMXReg(src));
    compiler.movsd(XMM(dst->hardwareReg()), XMM(tmp->hardwareReg()));
}

void JitX86CodeGen::cvtps2piMmxXmm(MMXRegPtr dst, SSERegPtr src) {
    compiler.cvtps2dq(getMMXReg(dst), XMM(src->hardwareReg()));
}

void JitX86CodeGen::cvtsi2ssXmmR32(SSERegPtr dst, RegPtr src) {
    compiler.cvtsi2ss(XMM(dst->hardwareReg()), R32(src));
}

void JitX86CodeGen::cvtss2siR32Xmm(RegPtr dst, SSERegPtr src) {
    compiler.cvtss2si(R32(dst), XMM(src->hardwareReg()));
}

void JitX86CodeGen::cvttps2piMmxXmm(MMXRegPtr dst, SSERegPtr src) {
    compiler.cvttps2dq(getMMXReg(dst), XMM(src->hardwareReg()));
}

void JitX86CodeGen::cvttss2siR32Xmm(RegPtr dst, SSERegPtr src) {
    compiler.cvttss2si(R32(dst), XMM(src->hardwareReg()));
}

void JitX86CodeGen::movhlpsXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.movhlps(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::movlhpsXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.movlhps(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::movssXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.movss(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::shufpsXmmXmm(SSERegPtr dst, SSERegPtr src, U32 imm) {
    compiler.shufps(XMM(dst->hardwareReg()), XMM(src->hardwareReg()), (U8)imm);
}

void JitX86CodeGen::cmppsXmmXmm(SSERegPtr dst, SSERegPtr src, U32 imm) {
    compiler.cmpps(XMM(dst->hardwareReg()), XMM(src->hardwareReg()), (U8)imm);
}

void JitX86CodeGen::cmpssXmmXmm(SSERegPtr dst, SSERegPtr src, U32 imm) {
    compiler.cmpss(XMM(dst->hardwareReg()), XMM(src->hardwareReg()), (U8)imm);
}

void JitX86CodeGen::setFlags(RegPtr flags, U32 mask) {
    if (mask != FMASK_TEST) {
        call_RI(common_setFlags, JitWidth::b32, flags, mask);
    } else {
        RegPtr reg = getTmpReg();
        compiler.mov(R32(reg), Mem(HOST_CPU, offsetof(CPU, flags)));
        compiler.and_(R32(reg), ~mask);
        compiler.and_(R32(flags), mask);
        compiler.or_(R32(reg), R32(flags));
        compiler.mov(Mem(HOST_CPU, offsetof(CPU, flags)), R32(reg));
        storeLazyFlagType(FLAGS_NONE);
    }
    currentLazyFlags = FLAGS_NONE;
}

RegPtr JitX86CodeGen::getReadOnlyFlags(RegPtr tmp) {
    if (!tmp) {
        tmp = getTmpReg8(); // lahf will do 8-bit on this
    }
    compiler.mov(R32(tmp), Mem(HOST_CPU, offsetof(CPU, flags)));
    //orValue(JitWidth::b32, tmp, 2);
    //andValue(JitWidth::b32, tmp, 0xFCFFFF);
    return tmp;
}

void JitX86CodeGen::updateFlagsIfNecessary() {
    U32 neededFlags = currentOp->needsToSetFlags(cpu);
    if (neededFlags) {        
        RegPtr flags = readCPU(JitWidth::b32, offsetof(CPU, flags));
        RegPtr tmp = getTmpReg8();
        
        if (neededFlags == CF) {
            movValue(JitWidth::b32, tmp, 0); // not xor, we don't want to affect flags
            compiler.setb(R8(tmp));
            andValue(JitWidth::b32, flags, ~CF);
            orReg(JitWidth::b32, flags, tmp);
        } else if (neededFlags == ZF) {
            movValue(JitWidth::b32, tmp, 0); // not xor, we don't want to affect flags
            compiler.setz(R8(tmp));
            shlValue(JitWidth::b32, tmp, 6);
            andValue(JitWidth::b32, flags, ~ZF);
            orReg(JitWidth::b32, flags, tmp);
        } else if (neededFlags == SF) {
            movValue(JitWidth::b32, tmp, 0); // not xor, we don't want to affect flags
            compiler.sets(R8(tmp));
            shlValue(JitWidth::b32, tmp, 7);
            andValue(JitWidth::b32, flags, ~SF);
            orReg(JitWidth::b32, flags, tmp);
        } else if (neededFlags == OF) {
            movValue(JitWidth::b32, tmp, 0); // not xor, we don't want to affect flags
            compiler.seto(R8(tmp));
            shlValue(JitWidth::b32, tmp, 11);
            andValue(JitWidth::b32, flags, ~OF);
            orReg(JitWidth::b32, flags, tmp);
        } else if (neededFlags == PF) {
            movValue(JitWidth::b32, tmp, 0); // not xor, we don't want to affect flags
            compiler.setp(R8(tmp));
            shlValue(JitWidth::b32, tmp, 2);
            andValue(JitWidth::b32, flags, ~PF);
            orReg(JitWidth::b32, flags, tmp);
        } else {
            bool savedEAX = false;

            if (regUsed[0] || regCache[0] == 0) {
                compiler.xchg(RN(0), RN(tmp));
                savedEAX = true;
            }
            if (neededFlags & OF) {
                compiler.lahf();
                compiler.seto(R8(0));
                compiler.shl(R8(0), 3);
                compiler.xchg(asmjit::x86::gp8_hi(0), R8(0));
            } else {
                compiler.lahf();
                compiler.shr(R16(0), 8);
            }
            // mask so we don't clobber DF
            andValue(JitWidth::b32, flags, ~FMASK_TEST);
            compiler.or_(R16(flags), regAx);
            if (savedEAX) {
                compiler.xchg(RN(0), RN(tmp));
            }
        }
        writeCPU(JitWidth::b32, offsetof(CPU, flags), flags);
        storeLazyFlagType(FLAGS_NONE);
        currentLazyFlags = FLAGS_NONE;
    }
}

void JitX86CodeGen::comissXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.comiss(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
    updateFlagsIfNecessary();
}

void JitX86CodeGen::ucomissXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.ucomiss(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
    updateFlagsIfNecessary();
}

void JitX86CodeGen::stmxcsr(MemPtr address) {
    compiler.stmxcsr(Mem(RN(calculateAddress(address)), 0));
}

void JitX86CodeGen::ldmxcsr(MemPtr address) {
    compiler.ldmxcsr(Mem(RN(calculateAddress(address)), 0));
}

void JitX86CodeGen::sfence() {
    compiler.sfence();
}

void JitX86CodeGen::unpckhpsXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.unpckhps(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::unpcklpsXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.unpcklps(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::movmskpsR32Xmm(RegPtr dst, SSERegPtr src) {
    compiler.movmskps(R32(dst), XMM(src->hardwareReg()));
}

MMXRegPtr JitX86CodeGen::loadMMXFromReg(RegPtr src) {
    MMXRegPtr tmp = getTmpMMX();
    compiler.movd(getMMXReg(tmp), R32(src));
    return tmp;
}

void JitX86CodeGen::storeCpuMMXReg(MMXRegPtr reg, U32 index) {
    compiler.movq(Mem(HOST_CPU, index * cpu->fpu.sizeofRegInRegsArray() + offsetof(CPU, fpu.regs[0].signif)), getMMXReg(reg));
}

void JitX86CodeGen::storeMMXToReg(MMXRegPtr src, RegPtr dst) {
    compiler.movd(R32(dst), getMMXReg(src));
}

MMXRegPtr JitX86CodeGen::loadCpuMMXReg(U8 index) {
    MMXRegPtr tmp = getTmpMMX();
    compiler.movq(getMMXReg(tmp), Mem(HOST_CPU, index * cpu->fpu.sizeofRegInRegsArray() + offsetof(CPU, fpu.regs[0].signif)));
    return tmp;
}

MMXRegPtr JitX86CodeGen::loadMMXFromMem32(U8 index, MemPtr address) {
    MMXRegPtr tmp = getTmpMMX();
    compiler.movd(getMMXReg(tmp), createMem(JitWidth::b32, address));
    return tmp;
}

MMXRegPtr JitX86CodeGen::loadMMXFromMem64(U8 index, MemPtr address) {
    MMXRegPtr tmp = getTmpMMX();
    compiler.movq(getMMXReg(tmp), createMem(JitWidth::b64, address));
    return tmp;
}

void JitX86CodeGen::storeMMXToMem32(MMXRegPtr reg, MemPtr address) {
    compiler.movd(createMem(JitWidth::b32, address), getMMXReg(reg));
}

void JitX86CodeGen::storeMMXToMem64(MMXRegPtr reg, MemPtr address) {
    compiler.movq(createMem(JitWidth::b64, address), getMMXReg(reg));
}

void JitX86CodeGen::xorMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    compiler.pxor(getMMXReg(dst), getMMXReg(src));
}

void JitX86CodeGen::orMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    compiler.por(getMMXReg(dst), getMMXReg(src));
}

void JitX86CodeGen::andMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    compiler.pand(getMMXReg(dst), getMMXReg(src));
}

void JitX86CodeGen::andnMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    compiler.pandn(getMMXReg(dst), getMMXReg(src));
}

void JitX86CodeGen::psllwMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    compiler.psllw(getMMXReg(dst), getMMXReg(src));
}

void JitX86CodeGen::psrlwMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    compiler.psrlw(getMMXReg(dst), getMMXReg(src));
}

void JitX86CodeGen::psrawMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    compiler.psraw(getMMXReg(dst), getMMXReg(src));
}

void JitX86CodeGen::psllwMmx(MMXRegPtr dst, U32 imm) {
    compiler.psllw(getMMXReg(dst), imm);
}

void JitX86CodeGen::psrlwMmx(MMXRegPtr dst, U32 imm) {
    compiler.psrlw(getMMXReg(dst), imm);
}

void JitX86CodeGen::psrawMmx(MMXRegPtr dst, U32 imm) {
    compiler.psraw(getMMXReg(dst), imm);
}

void JitX86CodeGen::pslldMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    compiler.pslld(getMMXReg(dst), getMMXReg(src));
}

void JitX86CodeGen::psrldMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    compiler.psrld(getMMXReg(dst), getMMXReg(src));
}

void JitX86CodeGen::psradMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    compiler.psrad(getMMXReg(dst), getMMXReg(src));
}

void JitX86CodeGen::pslldMmx(MMXRegPtr dst, U32 imm) {
    compiler.pslld(getMMXReg(dst), imm);
}

void JitX86CodeGen::psrldMmx(MMXRegPtr dst, U32 imm) {
    compiler.psrld(getMMXReg(dst), imm);
}

void JitX86CodeGen::psradMmx(MMXRegPtr dst, U32 imm) {
    compiler.psrad(getMMXReg(dst), imm);
}

void JitX86CodeGen::psllqMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    compiler.psllq(getMMXReg(dst), getMMXReg(src));
}

void JitX86CodeGen::psrlqMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    compiler.psrlq(getMMXReg(dst), getMMXReg(src));
}

void JitX86CodeGen::psllqMmx(MMXRegPtr dst, U32 imm) {
    compiler.psllq(getMMXReg(dst), imm);
}

void JitX86CodeGen::psrlqMmx(MMXRegPtr dst, U32 imm) {
    compiler.psrlq(getMMXReg(dst), imm);
}

void JitX86CodeGen::paddbMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    compiler.paddb(getMMXReg(dst), getMMXReg(src));
}

void JitX86CodeGen::paddwMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    compiler.paddw(getMMXReg(dst), getMMXReg(src));
}

void JitX86CodeGen::padddMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    compiler.paddd(getMMXReg(dst), getMMXReg(src));
}

void JitX86CodeGen::paddsbMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    compiler.paddsb(getMMXReg(dst), getMMXReg(src));
}

void JitX86CodeGen::paddswMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    compiler.paddsw(getMMXReg(dst), getMMXReg(src));
}

void JitX86CodeGen::paddusbMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    compiler.paddusb(getMMXReg(dst), getMMXReg(src));
}

void JitX86CodeGen::padduswMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    compiler.paddusw(getMMXReg(dst), getMMXReg(src));
}

void JitX86CodeGen::psubbMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    compiler.psubb(getMMXReg(dst), getMMXReg(src));
}

void JitX86CodeGen::psubwMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    compiler.psubw(getMMXReg(dst), getMMXReg(src));
}

void JitX86CodeGen::psubdMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    compiler.psubd(getMMXReg(dst), getMMXReg(src));
}

void JitX86CodeGen::psubsbMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    compiler.psubsb(getMMXReg(dst), getMMXReg(src));
}

void JitX86CodeGen::psubswMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    compiler.psubsw(getMMXReg(dst), getMMXReg(src));
}

void JitX86CodeGen::psubusbMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    compiler.psubusb(getMMXReg(dst), getMMXReg(src));
}

void JitX86CodeGen::psubuswMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    compiler.psubusw(getMMXReg(dst), getMMXReg(src));
}

void JitX86CodeGen::pmulhwMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    compiler.pmulhw(getMMXReg(dst), getMMXReg(src));
}

void JitX86CodeGen::pmullwMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    compiler.pmullw(getMMXReg(dst), getMMXReg(src));
}

void JitX86CodeGen::pmaddwdMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    compiler.pmaddwd(getMMXReg(dst), getMMXReg(src));
}

void JitX86CodeGen::pcmpeqbMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    compiler.pcmpeqb(getMMXReg(dst), getMMXReg(src));
}

void JitX86CodeGen::pcmpeqwMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    compiler.pcmpeqw(getMMXReg(dst), getMMXReg(src));
}

void JitX86CodeGen::pcmpeqdMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    compiler.pcmpeqd(getMMXReg(dst), getMMXReg(src));
}

void JitX86CodeGen::pcmpgtbMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    compiler.pcmpgtb(getMMXReg(dst), getMMXReg(src));
}

void JitX86CodeGen::pcmpgtwMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    compiler.pcmpgtw(getMMXReg(dst), getMMXReg(src));
}

void JitX86CodeGen::pcmpgtdMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    compiler.pcmpgtd(getMMXReg(dst), getMMXReg(src));
}

void JitX86CodeGen::packsswbMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    // xmm is twice as big as mm, so we need to adjust a little before the pack
    compiler.movlhps(getMMXReg(dst), getMMXReg(src));
    compiler.packsswb(getMMXReg(dst), getMMXReg(dst));
}

void JitX86CodeGen::packssdwMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    // xmm is twice as big as mm, so we need to adjust a little before the pack
    compiler.movlhps(getMMXReg(dst), getMMXReg(src));
    compiler.packssdw(getMMXReg(dst), getMMXReg(dst));
}

void JitX86CodeGen::packuswbMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    // xmm is twice as big as mm, so we need to adjust a little before the pack
    compiler.movlhps(getMMXReg(dst), getMMXReg(src));
    compiler.packuswb(getMMXReg(dst), getMMXReg(dst));
}

void JitX86CodeGen::punpckhbwMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    // :TODO: maybe move bytes 4-7 to 8-11 instead of 0-7 to 8-15 so that we don't have to do the movhlps to mov them back down?
    compiler.movlhps(getMMXReg(src), getMMXReg(src));
    compiler.movlhps(getMMXReg(dst), getMMXReg(dst));
    compiler.punpckhbw(getMMXReg(dst), getMMXReg(src));
    compiler.movhlps(getMMXReg(dst), getMMXReg(dst));
}

void JitX86CodeGen::punpckhwdMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    compiler.movlhps(getMMXReg(src), getMMXReg(src));
    compiler.movlhps(getMMXReg(dst), getMMXReg(dst));
    compiler.punpckhwd(getMMXReg(dst), getMMXReg(src));
    compiler.movhlps(getMMXReg(dst), getMMXReg(dst));
}

void JitX86CodeGen::punpckhdqMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    compiler.movlhps(getMMXReg(src), getMMXReg(src));
    compiler.movlhps(getMMXReg(dst), getMMXReg(dst));
    compiler.punpckhdq(getMMXReg(dst), getMMXReg(src));
    compiler.movhlps(getMMXReg(dst), getMMXReg(dst));
}

void JitX86CodeGen::punpcklbwMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    compiler.punpcklbw(getMMXReg(dst), getMMXReg(src));
}

void JitX86CodeGen::punpcklwdMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    compiler.punpcklwd(getMMXReg(dst), getMMXReg(src));
}

void JitX86CodeGen::punpckldqMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    compiler.punpckldq(getMMXReg(dst), getMMXReg(src));
}

#ifdef BOXEDWINE_64
void JitX86CodeGen::cvtsi2sdXmmR64(SSERegPtr dst, RegPtr src) {
    compiler.cvtsi2sd(XMM(dst->hardwareReg()), R64(src));
}
#endif

void JitX86CodeGen::addpdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.addpd(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::addsdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.addsd(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::subpdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.subpd(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::subsdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.subsd(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::mulpdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.mulpd(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::mulsdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.mulsd(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::divpdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.divpd(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::divsdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.divsd(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::maxpdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.maxpd(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::maxsdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.maxsd(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::minpdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.minpd(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::minsdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.minsd(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::paddbXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.paddb(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::paddwXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.paddw(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::padddXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.paddd(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::paddqXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.paddq(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::paddsbXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.paddsb(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::paddswXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.paddsw(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::paddusbXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.paddusb(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::padduswXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.paddusw(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::psubbXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.psubb(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::psubwXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.psubw(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::psubdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.psubd(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::psubqXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.psubq(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::psubsbXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.psubsb(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::psubswXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.psubsw(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::psubusbXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.psubusb(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::psubuswXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.psubusw(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::pmaddwdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.pmaddwd(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::pmulhwXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.pmulhw(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::pmullwXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.pmullw(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::pmuludqXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.pmuludq(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::sqrtpdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.sqrtpd(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::sqrtsdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.sqrtsd(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::andnpdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.andnpd(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::andpdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.andpd(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::pandXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.pand(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::pandnXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.pandn(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::porXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.por(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::pslldqXmm(SSERegPtr dst, U32 imm) {
    compiler.pslldq(XMM(dst->hardwareReg()), imm);
}

void JitX86CodeGen::psllqXmm(SSERegPtr dst, U32 imm) {
    compiler.psllq(XMM(dst->hardwareReg()), imm);
}

void JitX86CodeGen::pslldXmm(SSERegPtr dst, U32 imm) {
    compiler.pslld(XMM(dst->hardwareReg()), imm);
}

void JitX86CodeGen::psllwXmm(SSERegPtr dst, U32 imm) {
    compiler.psllw(XMM(dst->hardwareReg()), imm);
}

void JitX86CodeGen::psradXmm(SSERegPtr dst, U32 imm) {
    compiler.psrad(XMM(dst->hardwareReg()), imm);
}

void JitX86CodeGen::psrawXmm(SSERegPtr dst, U32 imm) {
    compiler.psraw(XMM(dst->hardwareReg()), imm);
}

void JitX86CodeGen::psrldqXmm(SSERegPtr dst, U32 imm) {
    compiler.psrldq(XMM(dst->hardwareReg()), imm);
}

void JitX86CodeGen::psrlqXmm(SSERegPtr dst, U32 imm) {
    compiler.psrlq(XMM(dst->hardwareReg()), imm);
}

void JitX86CodeGen::psrldXmm(SSERegPtr dst, U32 imm) {
    compiler.psrld(XMM(dst->hardwareReg()), imm);
}

void JitX86CodeGen::psrlwXmm(SSERegPtr dst, U32 imm) {
    compiler.psrlw(XMM(dst->hardwareReg()), imm);
}

void JitX86CodeGen::psllqXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.psllq(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::pslldXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.pslld(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::psllwXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.psllw(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::psradXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.psrad(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::psrawXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.psraw(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::psrlqXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.psrlq(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::psrldXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.psrld(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::psrlwXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.psrlw(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::pxorXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.pxor(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::orpdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.orpd(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::xorpdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.xorpd(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::cmppdXmmXmm(SSERegPtr dst, SSERegPtr src, U32 imm) {
    compiler.cmppd(XMM(dst->hardwareReg()), XMM(src->hardwareReg()), (U8)imm);
}

void JitX86CodeGen::cmpsdXmmXmm(SSERegPtr dst, SSERegPtr src, U32 imm) {
    compiler.cmpsd(XMM(dst->hardwareReg()), XMM(src->hardwareReg()), (U8)imm);
}

void JitX86CodeGen::comisdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.comisd(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
    updateFlagsIfNecessary();
}

void JitX86CodeGen::ucomisdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.ucomisd(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
    updateFlagsIfNecessary();
}

void JitX86CodeGen::pcmpgtbXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.pcmpgtb(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::pcmpgtwXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.pcmpgtw(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::pcmpgtdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.pcmpgtd(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::pcmpeqbXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.pcmpeqb(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::pcmpeqwXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.pcmpeqw(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::pcmpeqdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.pcmpeqd(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::cvtdq2pdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.cvtdq2pd(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::cvtdq2psXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.cvtdq2ps(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::cvtpd2piMmxXmm(MMXRegPtr dst, SSERegPtr src) {
    compiler.cvtpd2dq(getMMXReg(dst), XMM(src->hardwareReg()));
}

void JitX86CodeGen::cvtpi2pdXmmMmx(SSERegPtr dst, MMXRegPtr src) {
    compiler.cvtdq2pd(XMM(dst->hardwareReg()), getMMXReg(src));
}

void JitX86CodeGen::cvtpd2dqXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.cvtpd2dq(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::cvtpd2psXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.cvtpd2ps(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::cvttpd2piMmxXmm(MMXRegPtr dst, SSERegPtr src) {
    compiler.cvttpd2dq(getMMXReg(dst), XMM(src->hardwareReg()));
}

void JitX86CodeGen::cvtps2dqXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.cvtps2dq(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::cvtps2pdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.cvtps2pd(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::cvtsd2siR32Xmm(RegPtr dst, SSERegPtr src) {
    compiler.cvtsd2si(R32(dst), XMM(src->hardwareReg()));
}

void JitX86CodeGen::cvtsd2ssXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.cvtsd2ss(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::cvtsi2sdXmmR32(SSERegPtr dst, RegPtr src) {
    compiler.cvtsi2sd(XMM(dst->hardwareReg()), R32(src));
}

void JitX86CodeGen::cvtss2sdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.cvtss2sd(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::cvttpd2dqXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.cvttpd2dq(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::cvttps2dqXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.cvttps2dq(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::cvttsd2siR32Xmm(RegPtr dst, SSERegPtr src) {
    compiler.cvttsd2si(R32(dst), XMM(src->hardwareReg()));
}

void JitX86CodeGen::movsdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.movsd(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::movupdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.movdqu(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::movmskpd(RegPtr dst, SSERegPtr src) {
    compiler.movmskpd(R32(dst), XMM(src->hardwareReg()));
}

void JitX86CodeGen::movd(RegPtr dst, SSERegPtr src) {
    compiler.movd(R32(dst), XMM(src->hardwareReg()));
}

void JitX86CodeGen::movd(SSERegPtr dst, RegPtr src) {
    compiler.movd(XMM(dst->hardwareReg()), R32(src));
}

void JitX86CodeGen::movdq2q(MMXRegPtr dst, SSERegPtr src) {
    compiler.movq(getMMXReg(dst), XMM(src->hardwareReg()));
}

void JitX86CodeGen::movq2dq(SSERegPtr dst, MMXRegPtr src) {
    compiler.movq(XMM(dst->hardwareReg()), getMMXReg(src));
}

void JitX86CodeGen::movq(SSERegPtr dst, SSERegPtr src) {
    compiler.movq(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::maskmovdqu(SSERegPtr src, SSERegPtr mask, MemPtr address) {
    static U32 tmp7;

    currentOp->flags2 |= OP_FLAG2_SAVED_TMP_REG;
#ifndef BOXEDWINE_64
    compiler.mov(Mem((U64)&tmp7), RN(7));
#endif
    compiler.mov(Mem(HOST_CPU, offsetof(CPU, tmpReg)), RN(7));
    compiler.mov(RN(7), RN(calculateAddress(address)));
    compiler.maskmovdqu(XMM(src->hardwareReg()), XMM(mask->hardwareReg()));
#ifndef BOXEDWINE_64
    compiler.mov(RN(7), Mem((U64)&tmp7));
#endif
    compiler.mov(RN(7), Mem(HOST_CPU, offsetof(CPU, tmpReg)));
}

void JitX86CodeGen::pshufdXmmXmm(SSERegPtr dst, SSERegPtr src, U32 imm) {
    compiler.pshufd(XMM(dst->hardwareReg()), XMM(src->hardwareReg()), imm);
}

void JitX86CodeGen::pshufhwXmmXmm(SSERegPtr dst, SSERegPtr src, U32 imm) {
    compiler.pshufhw(XMM(dst->hardwareReg()), XMM(src->hardwareReg()), imm);
}

void JitX86CodeGen::pshuflwXmmXmm(SSERegPtr dst, SSERegPtr src, U32 imm) {
    compiler.pshuflw(XMM(dst->hardwareReg()), XMM(src->hardwareReg()), imm);
}

void JitX86CodeGen::shufpdXmmXmm(SSERegPtr dst, SSERegPtr src, U32 imm) {
    compiler.shufpd(XMM(dst->hardwareReg()), XMM(src->hardwareReg()), imm);
}

void JitX86CodeGen::unpckhpdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.unpckhpd(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::unpcklpdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.unpcklpd(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::punpckhbwXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.punpckhbw(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::punpckhwdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.punpckhwd(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::punpckhdqXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.punpckhdq(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::punpckhqdqXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.punpckhqdq(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::punpcklbwXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.punpcklbw(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::punpcklwdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.punpcklwd(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::punpckldqXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.punpckldq(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::punpcklqdqXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.punpcklqdq(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::packssdwXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.packssdw(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::packsswbXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.packsswb(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::packuswbXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.packuswb(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::pavgbXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.pavgb(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::pavgwXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.pavgw(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::psadbwXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.psadbw(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::pmaxswXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.pmaxsw(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::pmaxubXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.pmaxub(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::pminswXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.pminsw(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::pminubXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.pminub(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::pmulhuwXmmXmm(SSERegPtr dst, SSERegPtr src) {
    compiler.pmulhuw(XMM(dst->hardwareReg()), XMM(src->hardwareReg()));
}

void JitX86CodeGen::lfence() {
    compiler.lfence();
}

void JitX86CodeGen::mfence() {
    compiler.mfence();
}

void JitX86CodeGen::clflush(MemPtr address) {
    compiler.clflush(createMem(JitWidth::b32, address));
}

void JitX86CodeGen::pause() {
    compiler.pause();
}

void JitX86CodeGen::pextrwR32Xmm(RegPtr dst, SSERegPtr src, U32 imm) {
    compiler.pextrw(R32(dst), XMM(src->hardwareReg()), (U8)imm);
}

void JitX86CodeGen::pinsrwXmmR32(SSERegPtr dst, RegPtr src, U32 imm) {
    compiler.pinsrw(XMM(dst->hardwareReg()), R32(src), (U8)imm);
}

void JitX86CodeGen::pmovmskbR32Xmm(RegPtr dst, SSERegPtr src) {
    compiler.pmovmskb(R32(dst), XMM(src->hardwareReg()));
}

void JitX86CodeGen::updateFPURounding() {
    compiler.stmxcsr(Mem(HOST_CPU, offsetof(CPU, sseControlStateTmp)));

    RegPtr sse = readCPU(JitWidth::b32, offsetof(CPU, sseControlStateTmp));
    RegPtr fpu = readCPU(JitWidth::b32, offsetof(CPU, fpu.round));

    andValue(JitWidth::b32, sse, ~0x6000); // clear rounding
    shlValue(JitWidth::b32, fpu, 13);
    orReg(JitWidth::b32, sse, fpu); // set rounding in SSE

    // there is no way to set sse rounding from a register
    writeCPU(JitWidth::b32, offsetof(CPU, sseControlStateTmp2), sse);

    compiler.ldmxcsr(Mem(HOST_CPU, offsetof(CPU, sseControlStateTmp2)));
}

void JitX86CodeGen::restoreFPURounding() {
    compiler.ldmxcsr(Mem(HOST_CPU, offsetof(CPU, sseControlStateTmp)));
};

void JitX86CodeGen::storeCpuFpuReg(FPURegPtr reg, RegPtr index) {
    compiler.movsd(Mem(HOST_CPU, R32(index), 3, offsetof(CPU, fpu.regCache[0].d)), getFPUReg(reg));
}

void JitX86CodeGen::loadCpuFpuReg(FPURegPtr reg, RegPtr index) {
    compiler.movsd(getFPUReg(reg), Mem(HOST_CPU, R32(index), 3, offsetof(CPU, fpu.regCache[0].d)));
}

void JitX86CodeGen::loadCpuFpuRegConst(FPURegPtr reg, U32 offset) {
    compiler.movsd(getFPUReg(reg), Mem(HOST_CPU, offset));
}

RegPtr JitX86CodeGen::fpuRegToInt32(FPURegPtr fpuRegSrc, bool truncate) {
    RegPtr result = getTmpReg();
    if (truncate) {
        compiler.cvttsd2si(R32(result), getFPUReg(fpuRegSrc));
    } else {
        compiler.cvtsd2si(R32(result), getFPUReg(fpuRegSrc));
    }
    return result;
}

void JitX86CodeGen::roundFPUToInt64(FPURegPtr src) {
    fpuRegToInt64(src, src, false);
    fpuRegInt64To64(src, src);
}

void JitX86CodeGen::storeFPUToInt64(FPURegPtr src, MemPtr address, bool truncate) {
    fpuRegToInt64(src, src, truncate);
    storeFpuReg(src, address, DYN_FPU_64_BIT);
}

void JitX86CodeGen::fpuRegToInt64(FPURegPtr regDst, FPURegPtr fpuRegSrc, bool truncate) {
    if (truncate) {
        compiler.cvttpd2dq(getFPUReg(regDst), getFPUReg(fpuRegSrc));
    } else {
        compiler.cvtpd2dq(getFPUReg(regDst), getFPUReg(fpuRegSrc));
    }
}

void JitX86CodeGen::fpuRegInt64To64(FPURegPtr regDst, FPURegPtr fpuRegSrc) {
    compiler.cvtdq2pd(getFPUReg(regDst), getFPUReg(fpuRegSrc));
}

void JitX86CodeGen::storeFpuReg(FPURegPtr reg, MemPtr address, DynFpuWidth width) {
    if (width == DYN_FPU_64_BIT) {
        compiler.movsd(createMem(JitWidth::b64, address), getFPUReg(reg));
    } else {
        compiler.movss(createMem(JitWidth::b32, address), getFPUReg(reg));
    }
}

void JitX86CodeGen::loadFpuReg(FPURegPtr reg, MemPtr address, DynFpuWidth width) {
    if (width == DYN_FPU_64_BIT) {
        compiler.movsd(getFPUReg(reg), createMem(JitWidth::b64, address));
    } else {
        compiler.movss(getFPUReg(reg), createMem(JitWidth::b32, address));
    }
}

void JitX86CodeGen::fpuRegExtend32To64(FPURegPtr dst, FPURegPtr src) {
    compiler.cvtss2sd(getFPUReg(dst), getFPUReg(src));
}

void JitX86CodeGen::fpuReg64To32(FPURegPtr dst, FPURegPtr src) {
    compiler.cvtsd2ss(getFPUReg(dst), getFPUReg(src));
}

void JitX86CodeGen::loadFpuRegFromInt(FPURegPtr reg, MemPtr address) {
    compiler.cvtsi2sd(getFPUReg(reg), createMem(JitWidth::b32, address));
}

void JitX86CodeGen::regToFpuReg(FPURegPtr dst, RegPtr src) {
    compiler.cvtsi2sd(getFPUReg(dst), R32(src));
}

#ifdef BOXEDWINE_64
void JitX86CodeGen::regToFpuReg64(FPURegPtr dst, RegPtr src) {
    compiler.cvtsi2sd(getFPUReg(dst), R64(src));
}
#endif
void JitX86CodeGen::fpuAdd(FPURegPtr dst, FPURegPtr src) {
    compiler.addsd(getFPUReg(dst), getFPUReg(src));
}

void JitX86CodeGen::fpuMul(FPURegPtr dst, FPURegPtr src) {
    compiler.mulsd(getFPUReg(dst), getFPUReg(src));
}

void JitX86CodeGen::fpuSub(FPURegPtr dst, FPURegPtr src) {
    compiler.subsd(getFPUReg(dst), getFPUReg(src));
}

void JitX86CodeGen::fpuDiv(FPURegPtr dst, FPURegPtr src) {
    compiler.divsd(getFPUReg(dst), getFPUReg(src));
}

void JitX86CodeGen::fpuXor(FPURegPtr dst, FPURegPtr src) {
    compiler.xorpd(getFPUReg(dst), getFPUReg(src));
}

void JitX86CodeGen::fpuAnd(FPURegPtr dst, FPURegPtr src) {
    compiler.andpd(getFPUReg(dst), getFPUReg(src));
}

void JitX86CodeGen::fpuSqrt(FPURegPtr dst, FPURegPtr src) {
    compiler.sqrtsd(getFPUReg(dst), getFPUReg(src));
}

void JitX86CodeGen::IfPF() {
    Label label = compiler.new_label();
    ifLabels.push_back(label);
    compiler.jnp(label);
}

void JitX86CodeGen::IfZF() {
    Label label = compiler.new_label();
    ifLabels.push_back(label);
    compiler.jnz(label);
}

void JitX86CodeGen::IfCF() {
    Label label = compiler.new_label();
    ifLabels.push_back(label);
    compiler.jnb(label);
}

void JitX86CodeGen::fcompare(FPURegPtr fpuReg1, FPURegPtr fpuReg2, RegPtr ordTags, const std::function<void()>& pfnEqual, const std::function<void()>& pfnLessThan, const std::function<void()>& pfnGreaterThan, const std::function<void()>& pfnInvalid) {
    subValue(JitWidth::b8, ordTags, TAG_Empty);
    IfNot(JitWidth::b8, ordTags);
        pfnInvalid();
    StartElse();
        compiler.ucomisd(getFPUReg(fpuReg2), getFPUReg(fpuReg1));
        IfPF();
            pfnInvalid();
        StartElse();
            IfZF();
                pfnEqual();
            StartElse();
                IfCF();
                    pfnLessThan();
                StartElse();
                    pfnGreaterThan();
                EndIf();
            EndIf();
        EndIf();
    EndIf();
}

U32 JitX86CodeGen::getBufferSize() {
    code.flatten();
    return (U32)code.code_size();
}

void JitX86CodeGen::copyBuffer(U8* dst, U32 size) {
    code.relocate_to_base((DYN_PTR_SIZE)dst);
    code.copy_flattened_data(dst, size);
}

U32 JitX86CodeGen::getBufferLocation(U32 id) {
    return (U32)code.label_offset(labels.at(id));
}

U32 JitX86CodeGen::markBufferLocation() {
    asmjit::Label label = compiler.new_label();
    compiler.bind(label);
    labels.push_back(label);
    return (U32)labels.size() - 1;
}

void JitX86CodeGen::onBlockPreCommit(DecodedOp* op) {
    if (pendingLabels.size()) {
        kpanic("JitX86CodeGen::onBlockPreCommit");
    }
}

U32 JitX86CodeGen::getIfJumpSize() {
    return (U32)ifLabels.size();
}
   
U8* JitX86CodeGen::createBlockExit() {
    compiler.ret();
    return createDynamicExecutableMemory();
}

void JitX86CodeGen::blockExit() {
    compiler.ret();
}

void JitX86CodeGen::setCC(asmjit::x86::Gp reg, JitEvaluate condition) {

    switch (condition) {
    case JitEvaluate::EQUALS:
        compiler.setz(reg);
        break;
    case JitEvaluate::NOT_EQUALS:
        compiler.setnz(reg);
        break;
    case JitEvaluate::LESS_THAN_UNSIGNED:
        compiler.setb(reg);
        break;
    case JitEvaluate::LESS_THAN_EQUAL_UNSIGNED:
        compiler.setbe(reg);
        break;
    case JitEvaluate::GREATER_THAN_UNSIGNED:
        compiler.setnbe(reg);
        break;
    case JitEvaluate::GREATER_THAN_EQUAL_UNSIGNED:
        compiler.setnb(reg);
        break;    
    case JitEvaluate::LESS_THAN_SIGNED:
        compiler.setl(reg);
        break;
    case JitEvaluate::GREATER_THAN_EQUAL_SIGNED:
        compiler.setnl(reg);
        break;
    case JitEvaluate::LESS_THAN_EQUAL_SIGNED:
        compiler.setle(reg);
        break;
    case JitEvaluate::GREATER_THAN_SIGNED:
        compiler.setnle(reg);
        break;
    // no default, should get compiler error if not all enum cases handled
    }
}

void JitX86CodeGen::JumpInBlock(U32 address) {
    Label label;
    if (!opLabels.get(address, label)) {
        label = compiler.new_label();
        opLabels.set(address, label);
    }

    compiler.jmp(label);
}

void JitX86CodeGen::StartElse() {
    Label label = compiler.new_label();
    compiler.jmp(label);
    compiler.bind(ifLabels.back());
    ifLabels.pop_back();
    ifLabels.push_back(label);
}

void JitX86CodeGen::EndIf() {
    compiler.bind(ifLabels.back());
    ifLabels.pop_back();
}

void JitX86CodeGen::dynamic_rdtsc(DecodedOp* op) {
    compiler.rdtsc();
    getReg(0, -1, false); // will store EAX
    getReg(2, -1, false); // will store EDX
}

void JitX86CodeGen::updateHardwareFlags(U32 flags) {
    fillFlags();

    if (flags == CF) {
        compiler.bt(Mem8(HOST_CPU, offsetof(CPU, flags)), 0);
        return;
    }
    bool eaxPushed = false;
    RegPtr reg;

#ifdef BOXEDWINE_64
    if (!isTmp[0]) {
        compiler.push(RN(0));
        eaxPushed = true;
        reg = getReg(0);
    } else
#endif
    {
        if (!isTmpRegAvailable()) {
            compiler.push(RN(0));
            eaxPushed = true;
            regUsed[0] = false;
        }
        reg = getTmpRegForCallResult();
    }

    compiler.mov(R32(reg), Mem(HOST_CPU, offsetof(CPU, flags)));
    if (reg->hardwareReg() > 3) {
        kpanic("updateHardwareFlags");
    }
    compiler.xchg(asmjit::x86::gp8_lo((reg->hardwareReg())), asmjit::x86::gp8_hi((reg->hardwareReg())));
    if (flags & OF) {
        compiler.shr(R8(reg), 3);
    }
    if (reg->hardwareReg() == 0) {
        if (flags & OF) {
            compiler.add(regAl, 127); // (will restore OF)
        }
        compiler.sahf();
    } else {
        compiler.xchg(R32(reg), regEax);
        if (flags & OF) {
            compiler.add(regAl, 127); // (will restore OF)
        }
        compiler.sahf();
        compiler.xchg(R32(reg), regEax);
    }
    reg = nullptr;
    if (eaxPushed) {
        compiler.pop(RN(0));
        if (isTmp[0]) {
            regUsed[0] = true;
        }
    }
}

void JitX86CodeGen::dynamic_cmpxchg8b_lock(DecodedOp* op) {
    JitCodeGen::write(JitWidth::b64, calculateEaa(op), nullptr, [op, this](MemPtr address) {
        if (currentOp->getNeededFlagsAfter(PF | SF | AF | CF | OF)) { // The ZF flag is set if the destination operand and EDX:EAX are equal; otherwise it is cleared. The CF, PF, AF, SF, and OF flags are unaffected.
            updateHardwareFlags(PF | SF | AF | CF | OF);
        }
#ifdef BOXEDWINE_64
        this->compiler.lock();
        this->compiler.cmpxchg8b(createMem(JitWidth::b64, address));
#else
        callWriteCache();
        this->compiler.mov(RN(5), RN(address->rm));
        this->compiler.mov(RN(6), RN(address->sib));
        for (int i = 0; i < 4; i++) {
            compiler.mov(R32(i), Mem(HOST_CPU, CPU::offsetofReg32(i)));
        }
        this->compiler.lock();
        this->compiler.cmpxchg8b(Mem(RN(6), RN(5), 0, 0));
        for (int i = 0; i < 4; i++) {
            compiler.mov(Mem(HOST_CPU, CPU::offsetofReg32(i)), R32(i));
        }
        callLoadCache();
#endif
        updateFlagsIfNecessary();
    });
}
void JitX86CodeGen::dynamic_cmpxchge32r32_lock(DecodedOp* op) {
    JitCodeGen::write(JitWidth::b32, calculateEaa(op), nullptr, [op, this](MemPtr address) {
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
        this->compiler.lock();
        this->compiler.cmpxchg(createMem(JitWidth::b32, address), R32(reg));
        address = nullptr;
        updateFlagsIfNecessary();
    });
}
void JitX86CodeGen::dynamic_cmpxchge16r16_lock(DecodedOp* op) {
    JitCodeGen::write(JitWidth::b16, calculateEaa(op), nullptr, [op, this](MemPtr address) {
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
        this->compiler.lock();
        this->compiler.cmpxchg(createMem(JitWidth::b16, address), R16(reg));
        address = nullptr;
        updateFlagsIfNecessary();     
    });
}
void JitX86CodeGen::dynamic_cmpxchge8r8_lock(DecodedOp* op) {
    JitCodeGen::write(JitWidth::b8, calculateEaa(op), nullptr, [op, this](MemPtr address) {
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
        this->compiler.lock();
        this->compiler.cmpxchg(createMem(JitWidth::b8, address), R8(get8bitReg(reg)));
        address = nullptr;
        updateFlagsIfNecessary();
    });
}

void JitX86CodeGen::dynamic_xchge32r32_lock(DecodedOp* op) {
    JitCodeGen::write(JitWidth::b32, calculateEaa(op), nullptr, [op, this](MemPtr address) {
        RegPtr reg = getReg(op->reg);
        this->compiler.xchg(R32(reg), createMem(JitWidth::b32, address));
    });
}

void JitX86CodeGen::dynamic_xchge16r16_lock(DecodedOp* op) {
    JitCodeGen::write(JitWidth::b16, calculateEaa(op), nullptr, [op, this](MemPtr address) {
        RegPtr reg = getReg(op->reg);
        this->compiler.xchg(R16(reg), createMem(JitWidth::b16, address));
    });
}

void JitX86CodeGen::dynamic_xchge8r8_lock(DecodedOp* op) {
    JitCodeGen::write(JitWidth::b8, calculateEaa(op), nullptr, [op, this](MemPtr address) {
        RegPtr reg = getReg8(op->reg);
        this->compiler.xchg(R8(get8bitReg(reg)), createMem(JitWidth::b32, address));        
    });
}

void JitX86CodeGen::dynamic_arithE32R32_lock(DecodedOp* op, std::function<void(RegPtr dest, MemPtr address)> callback, bool writeReg) {
    JitCodeGen::write(JitWidth::b32, calculateEaa(op), nullptr, [writeReg, op, callback, this](MemPtr address) {
        RegPtr reg;
        
        if (writeReg) {
            reg = getReg(op->reg);
        } else {
            reg = getReadOnlyReg(op->reg);
        }
        callback(reg, address);        
        updateFlagsIfNecessary();
    });
}

void JitX86CodeGen::dynamic_arithE16R16_lock(DecodedOp* op, std::function<void(RegPtr dest, MemPtr address)> callback, bool writeReg) {
    JitCodeGen::write(JitWidth::b16, calculateEaa(op), nullptr, [writeReg, op, callback, this](MemPtr address) {
        RegPtr reg;

        if (writeReg) {
            reg = getReg(op->reg);
        } else {
            reg = getReadOnlyReg(op->reg);
        }
        callback(reg, address);        
        updateFlagsIfNecessary();
    });
}
void JitX86CodeGen::dynamic_arithE8R8_lock(DecodedOp* op, std::function<void(RegPtr dest, MemPtr address)> callback, bool writeReg) {
    JitCodeGen::write(JitWidth::b8, calculateEaa(op), nullptr, [writeReg, op, callback, this](MemPtr address) {
        RegPtr reg;

        if (writeReg) {
            reg = getReg8(op->reg);
        } else {
            reg = getReadOnlyReg8(op->reg);
        }
        callback(reg, address);        
        updateFlagsIfNecessary();
    });
}

void JitX86CodeGen::dynamic_arithE32_lock(DecodedOp* op, std::function<void(MemPtr address)> callback) {
    JitCodeGen::write(JitWidth::b32, calculateEaa(op), nullptr, [op, callback, this](MemPtr address) {
        callback(address);
        updateFlagsIfNecessary();
    });
}

void JitX86CodeGen::dynamic_arithE16_lock(DecodedOp* op, std::function<void(MemPtr address)> callback) {
    JitCodeGen::write(JitWidth::b16, calculateEaa(op), nullptr, [op, callback, this](MemPtr address) {
        callback(address);
        updateFlagsIfNecessary();
    });
}
void JitX86CodeGen::dynamic_arithE8_lock(DecodedOp* op, std::function<void(MemPtr address)> callback) {
    JitCodeGen::write(JitWidth::b8, calculateEaa(op), nullptr, [op, callback, this](MemPtr address) {
        callback(address);
        updateFlagsIfNecessary();
    });
}

void JitX86CodeGen::dynamic_xaddr32e32_lock(DecodedOp* op) {
    dynamic_arithE32R32_lock(op, [this](RegPtr dest, MemPtr address) {
        this->compiler.lock();
        this->compiler.xadd(createMem(JitWidth::b32, address), R32(dest));
    }, true);
}

void JitX86CodeGen::dynamic_xaddr16e16_lock(DecodedOp* op) {
    dynamic_arithE16R16_lock(op, [this](RegPtr dest, MemPtr address) {
        this->compiler.lock();
        this->compiler.xadd(createMem(JitWidth::b16, address), R16(dest));
    }, true);
}
void JitX86CodeGen::dynamic_xaddr8e8_lock(DecodedOp* op) {
    dynamic_arithE8R8_lock(op, [this](RegPtr dest, MemPtr address) {
        this->compiler.lock();
        this->compiler.xadd(createMem(JitWidth::b8, address), R8(get8bitReg(dest)));
    }, true);
}

void JitX86CodeGen::dynamic_adde32r32_lock(DecodedOp* op) {
    dynamic_arithE32R32_lock(op, [this](RegPtr dest, MemPtr address) {
        this->compiler.lock();
        this->compiler.add(createMem(JitWidth::b32, address), R32(dest));
    });
}
void JitX86CodeGen::dynamic_adde16r16_lock(DecodedOp* op) {
    dynamic_arithE16R16_lock(op, [this](RegPtr dest, MemPtr address) {
        this->compiler.lock();
        this->compiler.add(createMem(JitWidth::b16, address), R16(dest));
    });
}
void JitX86CodeGen::dynamic_adde8r8_lock(DecodedOp* op) {
    dynamic_arithE8R8_lock(op, [this](RegPtr dest, MemPtr address) {
        this->compiler.lock();
        this->compiler.add(createMem(JitWidth::b8, address), R8(get8bitReg(dest)));
    });
}
void JitX86CodeGen::dynamic_add32_mem_lock(DecodedOp* op) {
    dynamic_arithE32_lock(op, [op, this](MemPtr address) {
        this->compiler.lock();
        this->compiler.add(createMem(JitWidth::b32, address), op->imm);
    });
}
void JitX86CodeGen::dynamic_add16_mem_lock(DecodedOp* op) {
    dynamic_arithE16_lock(op, [op, this](MemPtr address) {
        this->compiler.lock();
        this->compiler.add(createMem(JitWidth::b16, address), (U16)op->imm);
    });
}
void JitX86CodeGen::dynamic_add8_mem_lock(DecodedOp* op) {
    dynamic_arithE8_lock(op, [op, this](MemPtr address) {
        this->compiler.lock();
        this->compiler.add(createMem(JitWidth::b8, address), (U8)op->imm);
    });
}

void JitX86CodeGen::dynamic_sube32r32_lock(DecodedOp* op) {
    dynamic_arithE32R32_lock(op, [this](RegPtr dest, MemPtr address) {
        this->compiler.lock();
        this->compiler.sub(createMem(JitWidth::b32, address), R32(dest));
    });
}
void JitX86CodeGen::dynamic_sube16r16_lock(DecodedOp* op) {
    dynamic_arithE16R16_lock(op, [this](RegPtr dest, MemPtr address) {
        this->compiler.lock();
        this->compiler.sub(createMem(JitWidth::b16, address), R16(dest));
    });
}
void JitX86CodeGen::dynamic_sube8r8_lock(DecodedOp* op) {
    dynamic_arithE8R8_lock(op, [this](RegPtr dest, MemPtr address) {
        this->compiler.lock();
        this->compiler.sub(createMem(JitWidth::b8, address), R8(get8bitReg(dest)));
    });
}
void JitX86CodeGen::dynamic_sub32_mem_lock(DecodedOp* op) {
    dynamic_arithE32_lock(op, [op, this](MemPtr address) {
        this->compiler.lock();
        this->compiler.sub(createMem(JitWidth::b32, address), op->imm);
    });
}
void JitX86CodeGen::dynamic_sub16_mem_lock(DecodedOp* op) {
    dynamic_arithE16_lock(op, [op, this](MemPtr address) {
        this->compiler.lock();
        this->compiler.sub(createMem(JitWidth::b16, address), (U16)op->imm);
    });
}
void JitX86CodeGen::dynamic_sub8_mem_lock(DecodedOp* op) {
    dynamic_arithE8_lock(op, [op, this](MemPtr address) {
        this->compiler.lock();
        this->compiler.sub(createMem(JitWidth::b8, address), (U8)op->imm);
    });
}
void JitX86CodeGen::dynamic_ore32r32_lock(DecodedOp* op) {
    dynamic_arithE32R32_lock(op, [this](RegPtr dest, MemPtr address) {
        this->compiler.lock();
        this->compiler.or_(createMem(JitWidth::b32, address), R32(dest));
    });
}
void JitX86CodeGen::dynamic_ore16r16_lock(DecodedOp* op) {
    dynamic_arithE16R16_lock(op, [this](RegPtr dest, MemPtr address) {
        this->compiler.lock();
        this->compiler.or_(createMem(JitWidth::b16, address), R16(dest));
    });
}
void JitX86CodeGen::dynamic_ore8r8_lock(DecodedOp* op) {
    dynamic_arithE8R8_lock(op, [this](RegPtr dest, MemPtr address) {
        this->compiler.lock();
        this->compiler.or_(createMem(JitWidth::b8, address), R8(get8bitReg(dest)));
    });
}
void JitX86CodeGen::dynamic_or32_mem_lock(DecodedOp* op) {
    dynamic_arithE32_lock(op, [op, this](MemPtr address) {
        this->compiler.lock();
        this->compiler.or_(createMem(JitWidth::b32, address), op->imm);
    });
}
void JitX86CodeGen::dynamic_or16_mem_lock(DecodedOp* op) {
    dynamic_arithE16_lock(op, [op, this](MemPtr address) {
        this->compiler.lock();
        this->compiler.or_(createMem(JitWidth::b16, address), (U16)op->imm);
    });
}
void JitX86CodeGen::dynamic_or8_mem_lock(DecodedOp* op) {
    dynamic_arithE8_lock(op, [op, this](MemPtr address) {
        this->compiler.lock();
        this->compiler.or_(createMem(JitWidth::b8, address), (U8)op->imm);
    });
}
void JitX86CodeGen::dynamic_ande32r32_lock(DecodedOp* op) {
    dynamic_arithE32R32_lock(op, [this](RegPtr dest, MemPtr address) {
        this->compiler.lock();
        this->compiler.and_(createMem(JitWidth::b32, address), R32(dest));
    });
}
void JitX86CodeGen::dynamic_ande16r16_lock(DecodedOp* op) {
    dynamic_arithE16R16_lock(op, [this](RegPtr dest, MemPtr address) {
        this->compiler.lock();
        this->compiler.and_(createMem(JitWidth::b16, address), R16(dest));
    });
}
void JitX86CodeGen::dynamic_ande8r8_lock(DecodedOp* op) {
    dynamic_arithE8R8_lock(op, [this](RegPtr dest, MemPtr address) {
        this->compiler.lock();
        this->compiler.and_(createMem(JitWidth::b8, address), R8(get8bitReg(dest)));
    });
}
void JitX86CodeGen::dynamic_and32_mem_lock(DecodedOp* op) {
    dynamic_arithE32_lock(op, [op, this](MemPtr address) {
        this->compiler.lock();
        this->compiler.and_(createMem(JitWidth::b32, address), op->imm);
        });
}
void JitX86CodeGen::dynamic_and16_mem_lock(DecodedOp* op) {
    dynamic_arithE16_lock(op, [op, this](MemPtr address) {
        this->compiler.lock();
        this->compiler.and_(createMem(JitWidth::b16, address), (U16)op->imm);
    });
}
void JitX86CodeGen::dynamic_and8_mem_lock(DecodedOp* op) {
    dynamic_arithE8_lock(op, [op, this](MemPtr address) {
        this->compiler.lock();
        this->compiler.and_(createMem(JitWidth::b8, address), (U8)op->imm);
    });
}
void JitX86CodeGen::dynamic_xore32r32_lock(DecodedOp* op) {
    dynamic_arithE32R32_lock(op, [this](RegPtr dest, MemPtr address) {
        this->compiler.lock();
        this->compiler.xor_(createMem(JitWidth::b32, address), R32(dest));
    });
}
void JitX86CodeGen::dynamic_xore16r16_lock(DecodedOp* op) {
    dynamic_arithE16R16_lock(op, [this](RegPtr dest, MemPtr address) {
        this->compiler.lock();
        this->compiler.xor_(createMem(JitWidth::b16, address), R16(dest));
    });
}
void JitX86CodeGen::dynamic_xore8r8_lock(DecodedOp* op) {
    dynamic_arithE8R8_lock(op, [this](RegPtr dest, MemPtr address) {
        this->compiler.lock();
        this->compiler.xor_(createMem(JitWidth::b8, address), R8(get8bitReg(dest)));
    });
}
void JitX86CodeGen::dynamic_xor32_mem_lock(DecodedOp* op) {
    dynamic_arithE32_lock(op, [op, this](MemPtr address) {
        this->compiler.lock();
        this->compiler.xor_(createMem(JitWidth::b32, address), op->imm);
    });
}
void JitX86CodeGen::dynamic_xor16_mem_lock(DecodedOp* op) {
    dynamic_arithE16_lock(op, [op, this](MemPtr address) {
        this->compiler.lock();
        this->compiler.xor_(createMem(JitWidth::b16, address), (U16)op->imm);
    });
}
void JitX86CodeGen::dynamic_xor8_mem_lock(DecodedOp* op) {
    dynamic_arithE8_lock(op, [op, this](MemPtr address) {
        this->compiler.lock();
        this->compiler.xor_(createMem(JitWidth::b8, address), (U8)op->imm);
    });
}
void JitX86CodeGen::dynamic_inc32_mem32_lock(DecodedOp* op) {
    dynamic_arithE32_lock(op, [op, this](MemPtr address) {
        if (op->getNeededFlagsAfter(CF)) {
            updateHardwareFlags(CF);
        }
        this->compiler.lock();
        this->compiler.inc(createMem(JitWidth::b32, address));
    });
}
void JitX86CodeGen::dynamic_inc16_mem16_lock(DecodedOp* op) {
    dynamic_arithE16_lock(op, [op, this](MemPtr address) {
        if (currentOp->getNeededFlagsAfter(CF)) {
            updateHardwareFlags(CF);
        }
        this->compiler.lock();
        this->compiler.inc(createMem(JitWidth::b16, address));
    });
}
void JitX86CodeGen::dynamic_inc8_mem8_lock(DecodedOp* op) {
    dynamic_arithE8_lock(op, [op, this](MemPtr address) {
        if (currentOp->getNeededFlagsAfter(CF)) {
            updateHardwareFlags(CF);
        }
        this->compiler.lock();
        this->compiler.inc(createMem(JitWidth::b8, address));
    });
}
void JitX86CodeGen::dynamic_dec32_mem32_lock(DecodedOp* op) {
    dynamic_arithE32_lock(op, [op, this](MemPtr address) {
        if (currentOp->getNeededFlagsAfter(CF)) {
            updateHardwareFlags(CF);
        }
        this->compiler.lock();
        this->compiler.dec(createMem(JitWidth::b32, address));
    });
}
void JitX86CodeGen::dynamic_dec16_mem16_lock(DecodedOp* op) {
    dynamic_arithE16_lock(op, [op, this](MemPtr address) {
        if (currentOp->getNeededFlagsAfter(CF)) {
            updateHardwareFlags(CF);
        }
        this->compiler.lock();
        this->compiler.dec(createMem(JitWidth::b16, address));
    });
}
void JitX86CodeGen::dynamic_dec8_mem8_lock(DecodedOp* op) {
    dynamic_arithE8_lock(op, [op, this](MemPtr address) {
        if (currentOp->getNeededFlagsAfter(CF)) {
            updateHardwareFlags(CF);
        }
        this->compiler.lock();
        this->compiler.dec(createMem(JitWidth::b8, address));
    });
}

void JitX86CodeGen::dynamic_note32_lock(DecodedOp* op) {
    dynamic_arithE32_lock(op, [op, this](MemPtr address) {
        this->compiler.lock();
        this->compiler.not_(createMem(JitWidth::b32, address));
    });
}

void JitX86CodeGen::dynamic_note16_lock(DecodedOp* op) {
    dynamic_arithE16_lock(op, [op, this](MemPtr address) {
        this->compiler.lock();
        this->compiler.not_(createMem(JitWidth::b16, address));
    });
}

void JitX86CodeGen::dynamic_note8_lock(DecodedOp* op) {
    dynamic_arithE8_lock(op, [op, this](MemPtr address) {
        this->compiler.lock();
        this->compiler.not_(createMem(JitWidth::b8, address));
    });
}

void JitX86CodeGen::dynamic_nege32_lock(DecodedOp* op) {
    dynamic_arithE32_lock(op, [op, this](MemPtr address) {
        this->compiler.lock();
        this->compiler.neg(createMem(JitWidth::b32, address));
    });
}

void JitX86CodeGen::dynamic_nege16_lock(DecodedOp* op) {
    dynamic_arithE16_lock(op, [op, this](MemPtr address) {
        this->compiler.lock();
        this->compiler.neg(createMem(JitWidth::b16, address));
    });
}

void JitX86CodeGen::dynamic_nege8_lock(DecodedOp* op) {
    dynamic_arithE8_lock(op, [op, this](MemPtr address) {
        this->compiler.lock();
        this->compiler.neg(createMem(JitWidth::b8, address));
    });
}

void JitX86CodeGen::dynamic_btse32_lock(DecodedOp* op) {
    JitCodeGen::write(JitWidth::b32, calculateEaa(op), nullptr, [op, this](MemPtr address) {
        // imm is the mask, not the bitshift
        U8 imm = std::countr_zero(op->imm);
        this->compiler.lock();
        this->compiler.bts(createMem(JitWidth::b32, address), (U8)imm);
        updateFlagsIfNecessary();
    });
}

void JitX86CodeGen::dynamic_btse16_lock(DecodedOp* op) {
    JitCodeGen::write(JitWidth::b16, calculateEaa(op), nullptr, [op, this](MemPtr address) {
        // imm is the mask, not the bitshift
        U8 imm = std::countr_zero(op->imm);
        this->compiler.lock();
        this->compiler.bts(createMem(JitWidth::b16, address), (U8)imm);
        updateFlagsIfNecessary();
    });
}

void JitX86CodeGen::dynamic_btse32r32_lock(DecodedOp* op) {
    JitCodeGen::write(JitWidth::b32, calculateEffectiveEaa32(op), nullptr, [op, this](MemPtr address) {
        RegPtr reg = getTmpReg(op->reg);
        andValue(JitWidth::b32, reg, 0x1f);
        this->compiler.lock();
        this->compiler.bts(createMem(JitWidth::b32, address), R32(reg));
        updateFlagsIfNecessary();
    });
}

void JitX86CodeGen::dynamic_btse16r16_lock(DecodedOp* op) {
    JitCodeGen::write(JitWidth::b16, calculateEffectiveEaa16(op), nullptr, [op, this](MemPtr address) {
        RegPtr reg = getTmpReg(op->reg);
        andValue(JitWidth::b16, reg, 0xf);
        this->compiler.lock();
        this->compiler.bts(createMem(JitWidth::b16, address), R16(reg));
        updateFlagsIfNecessary();
    });
}

void JitX86CodeGen::dynamic_btre32_lock(DecodedOp* op) {
    JitCodeGen::write(JitWidth::b32, calculateEaa(op), nullptr, [op, this](MemPtr address) {
        // imm is the mask, not the bitshift
        U8 imm = std::countr_zero(op->imm);
        this->compiler.lock();
        this->compiler.btr(createMem(JitWidth::b32, address), (U8)imm);
        updateFlagsIfNecessary();
    });
}
void JitX86CodeGen::dynamic_btre16_lock(DecodedOp* op) {
    JitCodeGen::write(JitWidth::b16, calculateEaa(op), nullptr, [op, this](MemPtr address) {
        // imm is the mask, not the bitshift
        U8 imm = std::countr_zero(op->imm);
        this->compiler.lock();
        this->compiler.btr(createMem(JitWidth::b16, address), (U8)imm);
        updateFlagsIfNecessary();
    });
}
void JitX86CodeGen::dynamic_btre32r32_lock(DecodedOp* op) {
    JitCodeGen::write(JitWidth::b32, calculateEffectiveEaa32(op), nullptr, [op, this](MemPtr address) {
        RegPtr reg = getTmpReg(op->reg);
        andValue(JitWidth::b32, reg, 0x1f);
        this->compiler.lock();
        this->compiler.btr(createMem(JitWidth::b32, address), R32(reg));
        updateFlagsIfNecessary();
    });
}
void JitX86CodeGen::dynamic_btre16r16_lock(DecodedOp* op) {
    JitCodeGen::write(JitWidth::b16, calculateEffectiveEaa16(op), nullptr, [op, this](MemPtr address) {
        RegPtr reg = getTmpReg(op->reg);
        andValue(JitWidth::b16, reg, 0xf);
        this->compiler.lock();
        this->compiler.btr(createMem(JitWidth::b16, address), R16(reg));
        updateFlagsIfNecessary();
    });
}

void JitX86CodeGen::dynamic_btce32_lock(DecodedOp* op) {
    JitCodeGen::write(JitWidth::b32, calculateEaa(op), nullptr, [op, this](MemPtr address) {
        // imm is the mask, not the bitshift
        U8 imm = std::countr_zero(op->imm);
        this->compiler.lock();
        this->compiler.btc(createMem(JitWidth::b32, address), (U8)imm);
        updateFlagsIfNecessary();
    });
}
void JitX86CodeGen::dynamic_btce16_lock(DecodedOp* op) {
    JitCodeGen::write(JitWidth::b16, calculateEaa(op), nullptr, [op, this](MemPtr address) {
        // imm is the mask, not the bitshift
        U8 imm = std::countr_zero(op->imm);
        this->compiler.lock();
        this->compiler.btc(createMem(JitWidth::b16, address), (U8)imm);
        updateFlagsIfNecessary();
    });
}
void JitX86CodeGen::dynamic_btce32r32_lock(DecodedOp* op) {
    JitCodeGen::write(JitWidth::b32, calculateEffectiveEaa32(op), nullptr, [op, this](MemPtr address) {
        RegPtr reg = getTmpReg(op->reg);
        andValue(JitWidth::b32, reg, 0x1f);
        this->compiler.lock();
        this->compiler.btc(createMem(JitWidth::b32, address), R32(reg));
        updateFlagsIfNecessary();
    });
}
void JitX86CodeGen::dynamic_btce16r16_lock(DecodedOp* op) {
    JitCodeGen::write(JitWidth::b16, calculateEffectiveEaa16(op), nullptr, [op, this](MemPtr address) {
        RegPtr reg = getTmpReg(op->reg);
        andValue(JitWidth::b16, reg, 0xf);
        this->compiler.lock();
        this->compiler.btc(createMem(JitWidth::b16, address), R16(reg));
        updateFlagsIfNecessary();
    });
}

RegPtr JitX86CodeGen::calculateEaa(DecodedOp* op, U32 popEspAmount) {
    if (op->ea16) {
        // cpu->seg[op->base].address + (U16)(cpu->reg[op->rm].u16 + (S16)cpu->reg[op->sibIndex].u16 + op->disp)
        RegPtr result = getTmpReg();
        U32 disp = op->data.disp;
        RegPtr rm;
        RegPtr sibIndex;

        if (popEspAmount && op->rm == 4) {
            disp += popEspAmount;
        }
        if (popEspAmount && op->sibIndex == 4) {
            disp += popEspAmount;
        }
        disp &= 0xffff;

        if (op->rm != 8 && op->sibIndex == op->rm) {
            rm = getReadOnlyReg(op->rm);
            sibIndex = rm;
        } else {
            if (op->rm != 8) {
                rm = getReadOnlyReg(op->rm);
            }
            if (op->sibIndex != 8) {
                sibIndex = getReadOnlyReg(op->sibIndex);
            }
        }
        if (op->base < 6) {
            RegPtr seg = getReadOnlySegAddress(op->base);

            if (op->rm != 8 && op->sibIndex != 8) {
                compiler.lea(R32(result->hardwareReg()), Mem(R32(sibIndex->hardwareReg()), R32(rm->hardwareReg()), 0, disp));
                compiler.and_(R32(result->hardwareReg()), 0xffff);
            } else if (op->rm != 8) {
                if (disp) {
                    compiler.lea(R32(result->hardwareReg()), Mem(R32(rm->hardwareReg()), disp));
                    compiler.and_(R32(result->hardwareReg()), 0xffff);
                } else {
                    compiler.lea(R32(result->hardwareReg()), Mem(R32(seg->hardwareReg()), R32(rm->hardwareReg()), 0, 0));
                    return result;
                }
            } else if (op->sibIndex != 8) {
                if (disp) {
                    compiler.lea(R32(result->hardwareReg()), Mem(R32(sibIndex->hardwareReg()), disp));
                    compiler.and_(R32(result->hardwareReg()), 0xffff);
                } else {
                    compiler.lea(R32(result->hardwareReg()), Mem(R32(seg->hardwareReg()), R32(sibIndex->hardwareReg()), 0, 0));
                    return result;
                }
            } else if (disp) {
                compiler.lea(R32(result->hardwareReg()), Mem(R32(seg->hardwareReg()), disp));
                return result;
            } else {
                compiler.mov(R32(result->hardwareReg()), R32(seg->hardwareReg()));
                return result;
            }
            compiler.lea(R32(result->hardwareReg()), Mem(R32(seg->hardwareReg()), R32(result->hardwareReg()), 0, 0));
        } else {
            if (op->rm != 8 && op->sibIndex != 8) {
                compiler.lea(R32(result->hardwareReg()), Mem(R32(rm->hardwareReg()), R32(sibIndex->hardwareReg()), 0, disp));
                compiler.and_(R32(result->hardwareReg()), 0xffff);
            } else if (op->rm != 8) {
                if (disp) {
                    compiler.lea(R32(result->hardwareReg()), Mem(R32(rm->hardwareReg()), disp));
                    compiler.and_(R32(result->hardwareReg()), 0xffff);
                } else {
                    compiler.mov(R32(result->hardwareReg()), 0);
                    compiler.mov(R16(result->hardwareReg()), R16(rm->hardwareReg()));
                }
            } else if (op->sibIndex != 8) {
                if (disp) {
                    compiler.lea(R32(result->hardwareReg()), Mem(R32(sibIndex->hardwareReg()), disp));
                    compiler.and_(R32(result->hardwareReg()), 0xffff);
                } else {
                    compiler.mov(R32(result->hardwareReg()), 0);
                    compiler.mov(R16(result->hardwareReg()), R16(sibIndex->hardwareReg()));
                }
            } else {
                compiler.mov(R32(result->hardwareReg()), 0);
                if (disp) {
                    compiler.mov(R16(result->hardwareReg()), disp);
                }
            }
        }

        return result;
    } else {
        RegPtr result = getTmpReg();
        U32 disp = op->data.disp;

        if (popEspAmount && op->rm == 4) {
            disp += popEspAmount;
        }
        if (popEspAmount && op->sibIndex == 4) {
            disp += (popEspAmount << op->sibIndex);
        }

        // seg[6] is always 0
        RegPtr rm;
        RegPtr sibIndex;

        if (op->rm != 8 && op->sibIndex == op->rm) {
            rm = getReadOnlyReg(op->rm);
            sibIndex = rm;
        } else {
            if (op->rm != 8) {
                rm = getReadOnlyReg(op->rm);
            }
            if (op->sibIndex != 8) {
                sibIndex = getReadOnlyReg(op->sibIndex);
            }
        }
        if (op->base < 6 && KThread::currentThread()->process->hasSetSeg[op->base]) {
            RegPtr seg = getReadOnlySegAddress(op->base);

            if (op->rm != 8 && op->sibIndex != 8) {
                compiler.lea(R32(result->hardwareReg()), Mem(R32(seg->hardwareReg()), R32(rm->hardwareReg()), 0, 0));
                compiler.lea(R32(result->hardwareReg()), Mem(R32(result->hardwareReg()), R32(sibIndex->hardwareReg()), op->sibScale, disp));
            } else if (op->rm != 8) {
                compiler.lea(R32(result->hardwareReg()), Mem(R32(seg->hardwareReg()), R32(rm->hardwareReg()), 0, disp));
            } else if (op->sibIndex != 8) {
                compiler.lea(R32(result->hardwareReg()), Mem(R32(seg->hardwareReg()), R32(sibIndex->hardwareReg()), op->sibScale, disp));
            } else {
                compiler.lea(R32(result->hardwareReg()), Mem(R32(seg->hardwareReg()), disp));
            }
        } else {
            if (op->rm != 8 && op->sibIndex != 8) {
                compiler.lea(R32(result->hardwareReg()), Mem(R32(rm->hardwareReg()), R32(sibIndex->hardwareReg()), op->sibScale, disp));
            } else if (op->rm != 8) {
                compiler.lea(R32(result->hardwareReg()), Mem(R32(rm->hardwareReg()), disp));
            } else if (op->sibIndex != 8) {
                compiler.mov(R32(result->hardwareReg()), 0);
                compiler.lea(R32(result->hardwareReg()), Mem(R32(result->hardwareReg()), R32(sibIndex->hardwareReg()), op->sibScale, disp));
            } else {
                compiler.mov(R32(result->hardwareReg()), disp);
            }
        }
        return result;
    }
}

bool JitX86CodeGen::directDoesAffectFlags(DecodedOp* op) {
    //read/write does things like address >> K_PAGE_SHIFT which will affect flags, perhaps we can make special versions of read/write for x86 if needed
    switch (op->inst) {
    case MovR8R8:
    //case MovE8R8:
    //case MovR8E8:
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
    //case MovAlOb:
    //case MovAxOw:
    //case MovEaxOd:
    //case MovObAl:
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

asmjit::x86::CondCode JitX86CodeGen::getCondCode(JitConditional condition) {
    switch (condition) {
    case JitConditional::O: return asmjit::x86::CondCode::kO;
    case JitConditional::NO: return asmjit::x86::CondCode::kNO;
    case JitConditional::B: return asmjit::x86::CondCode::kB;
    case JitConditional::NB: return asmjit::x86::CondCode::kNB;
    case JitConditional::Z: return asmjit::x86::CondCode::kZ;
    case JitConditional::NZ: return asmjit::x86::CondCode::kNZ;
    case JitConditional::BE: return asmjit::x86::CondCode::kBE;
    case JitConditional::NBE: return asmjit::x86::CondCode::kNBE;
    case JitConditional::S: return asmjit::x86::CondCode::kS;
    case JitConditional::NS: return asmjit::x86::CondCode::kNS;
    case JitConditional::P: return asmjit::x86::CondCode::kP;
    case JitConditional::NP: return asmjit::x86::CondCode::kNP;
    case JitConditional::L: return asmjit::x86::CondCode::kL;
    case JitConditional::NL: return asmjit::x86::CondCode::kNL;
    case JitConditional::LE: return asmjit::x86::CondCode::kLE;
    case JitConditional::NLE: return asmjit::x86::CondCode::kNLE;
    default:
        kpanic("JitX86CodeGen::getCondCode");
        return asmjit::x86::CondCode::kO;
    }
}

void JitX86CodeGen::direct_cmov(JitWidth width, JitConditional condition, RegPtr dst, RegPtr src) {
    compiler.cmov(getCondCode(condition), R(width, dst), R(width, src));
}

void JitX86CodeGen::direct_setcc(JitConditional condition, RegPtr dst) {
#ifdef BOXEDWINE_64
    compiler.set(getCondCode(condition), R8(dst));
#else
    compiler.set(getCondCode(condition), R8(dst->hardwareReg() + (dst->isHigh ? 4 : 0)));
#endif
}

void JitX86CodeGen::direct_jump(JitConditional condition, U32 address) {
    Label label;

    if (!opLabels.get(address, label)) {
        label = compiler.new_label();
        opLabels.set(address, label);
        pendingLabels.set(address, label);
    }

    compiler.j(getCondCode(condition), label);
}

void JitX86CodeGen::direct_cmp(JitWidth width, RegPtr left, RegPtr right) {
    if (width == JitWidth::b32) {
        compiler.cmp(R32(left), R32(right));
    } else if (width == JitWidth::b16) {
        compiler.cmp(R16(left), R16(right));
    } else if (width == JitWidth::b8) {
        compiler.cmp(R8(left), R8(right));
    } else {
        kpanic("JitX86CodeGen::direct_cmp");
    }

}

void JitX86CodeGen::direct_cmp(JitWidth width, RegPtr left, U32 right) {
    if (width == JitWidth::b32) {
        compiler.cmp(R32(left), right);
    } else if (width == JitWidth::b16) {
        compiler.cmp(R16(left), right);
    } else if (width == JitWidth::b8) {
        compiler.cmp(R8(left), right);
    } else {
        kpanic("JitX86CodeGen::direct_cmp");
    }
}

void JitX86CodeGen::direct_test(JitWidth width, RegPtr left, RegPtr right) {
    if (width == JitWidth::b32) {
        compiler.test(R32(left), R32(right));
    } else if (width == JitWidth::b16) {
        compiler.test(R16(left), R16(right));
    } else if (width == JitWidth::b8) {
        compiler.test(R8(left), R8(right));
    } else {
        kpanic("JitX86CodeGen::direct_test");
    }

}

void JitX86CodeGen::direct_test(JitWidth width, RegPtr left, U32 right) {
    if (width == JitWidth::b32) {
        compiler.test(R32(left), right);
    } else if (width == JitWidth::b16) {
        compiler.test(R16(left), right);
    } else if (width == JitWidth::b8) {
        compiler.test(R8(left), right);
    } else {
        kpanic("JitX86CodeGen::direct_test");
    }
}

U8* JitX86CodeGen::createStartJITCode() {
#ifdef BOXEDWINE_64
    for (int i = 0; i < NUMBER_OF_REGS; i++) {
        if (!isVolitile[i]) {
            compiler.mov(Mem(params[0], offsetof(CPU, storedRegs[0]) + sizeof(U64) * i), R64(i));
        }
    }
    RegPtr tmpReg = getTmpReg();
    compiler.mov(R64(tmpReg), R64(4));
    compiler.and_(R64(tmpReg), 0xf);
    IfNot(JitWidth::b32, tmpReg); {
        compiler.push(asmjit::x86::rax);
    } EndIf();    
    compiler.mov(HOST_MMU, (U64)getMemData(KThread::currentThread()->memory)->mmu);
    
    compiler.mov(HOST_CPU, params[0]);
    compiler.mov(RN(tmps[0]), Mem(params[1], offsetof(DecodedOp, pfnJitCode)));

    loadCache();

    // jmp ((DecodedOp*)rdx)->pfn    
    compiler.call(RN(tmps[0]));

#else
    compiler.push(regEbx);
    compiler.push(regEdi);
    compiler.push(regEsi);
    compiler.push(regEbp);
    // on win32 ecx contains cpu
    compiler.mov(HOST_CPU, regEcx);

    loadCache();

    // :TODO: what about other x86 platforms that use a different calling convention
    // 
    // jmp ((DecodedOp*)edx)->pfn
    compiler.mov(RN(tmps[0]), Mem(regEdx, offsetof(DecodedOp, pfnJitCode)));
    compiler.call(RN(tmps[0]));
#endif

    writeCache();

#ifdef BOXEDWINE_64
    compiler.mov(R64(0), HOST_CPU);
    for (int i = 0; i < NUMBER_OF_REGS; i++) {
        if (!isVolitile[i]) {
            compiler.mov(R64(i), Mem(R64(0), offsetof(CPU, storedRegs[0]) + sizeof(U64) * i));
        }
    }
#else
    compiler.pop(regEbp);
    compiler.pop(regEsi);
    compiler.pop(regEdi);
    compiler.pop(regEbx);
#endif
    compiler.ret();
    return createDynamicExecutableMemory();
}

void JitX86CodeGen::loadCache() {
    for (int i = 0; i < 8; i++) {
        if (regCache[i] != INVALID_REG) {
            compiler.mov(R32(regCache[i]), Mem(HOST_CPU, offsetof(CPU, reg[0].u32) + sizeof(U32) * i));
        }
    }
    for (int i = 0; i < 8; i++) {
        if (xmmCache[i] != INVALID_REG) {
            compiler.movaps(XMM(xmmCache[i]), Mem(HOST_CPU, i * 16 + offsetof(CPU, xmm)));
        }
    }
}

void JitX86CodeGen::writeCache() {
    for (int i = 0; i < 8; i++) {
        if (regCache[i] != INVALID_REG) {
            compiler.mov(Mem(HOST_CPU, offsetof(CPU, reg[0].u32) + sizeof(U32) * i), R32(regCache[i]));
        }
    }    
    for (int i = 0; i < 8; i++) {
        if (xmmCache[i] != INVALID_REG) {
            compiler.movaps(Mem(HOST_CPU, i * 16 + offsetof(CPU, xmm)), XMM(xmmCache[i]));
        }
    }
}

U8* JitX86CodeGen::createSyncToHost() {
    writeCache();
    compiler.ret();
    return createDynamicExecutableMemory();
}

U8* JitX86CodeGen::createSyncFromHost() {
    loadCache();
    compiler.ret();
    return createDynamicExecutableMemory();
}

JitCodeGen* startNewJIT(CPU* cpu) {
    return new JitX86CodeGen(cpu);
}

void startNewJIT(CPU* cpu, U32 address, DecodedOp* op) {
    JitX86CodeGen data(cpu);
    data.doJIT(address, op);
}

void clearJitBlock(void* p, U32 len) {
#ifdef BOXEDWINE_HOST_EXCEPTIONS
    ::memset(p, 0xcd, len);
#endif
}

#endif
