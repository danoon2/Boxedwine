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
#define NUMBER_OF_TMPS 5
#ifdef BOXEDWINE_64

#if 0
// this is a tiny bit faster, but there is a bug, ff installer crashes
// 38.2 vs 37.8 fps for Quake 2
static U8 regCache[] = { 8, 9, 11, 12, 10, 5, 6, 7 };
static bool isTmp[] = { true, true, true, true, false, false, false, false, false, false, false, false, false, false, true, false };
static U8 tmps[] = { 14, 3, 2, 1, 0 };
// should be a volitile tmp reg
#define PARAM_CALL_TMP x86.rdx
#else
static U8 regCache[] = { 0, 1, 2, 3, 10, 5, 6, INVALID_REG };
static U8 xmmCache[] = { 0, 1, 2, 3, 4, 5, 6, 7 };
static bool isTmp[] = { false, false, false, false, false, false, false, false, true, true, false, true, true, false, true, false };
static U8 tmps[] = { 14, 12, 11, 9, 8 };
static U8 XMMtmps[] = { 12, 13, 14, 15 };
// should be a volitile tmp reg
#define PARAM_CALL_TMP x86.r11
#endif
#define HOST_RAM x86.r15
#define HOST_MMU x86.rdi
#define HOST_CPU x86.r13
#define NUMBER_OF_REGS 16
#define NUMBER_OF_XMM_REG 16
#define NUMBER_OF_XMM_TMPS 4
#define MEM_PTR X86Asm::Mem64
#define RN(x) X86Asm::Reg64::from(x)
static bool isVolitile[] = { true, true, true, false, false, false, false, false, true, true, true, true, false, false, false, false };
#else
static U8 regCache[] = { 5, INVALID_REG, INVALID_REG, INVALID_REG, INVALID_REG, INVALID_REG, INVALID_REG, INVALID_REG };
static U8 xmmCache[] = { INVALID_REG, INVALID_REG, INVALID_REG, INVALID_REG, INVALID_REG, INVALID_REG, INVALID_REG, INVALID_REG };
static bool isVolitile[] = { true, true, true, false, false, false, false, false };
static bool isTmp[] = { true, true, true, true, false, false, true, false };
static U8 tmps[] = { 6, 3, 2, 1, 0 };
static U8 XMMtmps[] = { 4, 5, 6, 7 };
#define HOST_CPU x86.edi
#define NUMBER_OF_REGS 8
#define NUMBER_OF_XMM_REG 8
#define NUMBER_OF_XMM_TMPS 4
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
    RegPtr readEip() override;
    void writeEip(RegPtr eip) override;
    void writeEip(U32 eip) override;
    void pushReg(RegPtr reg) override;
    void popReg(RegPtr reg) override;
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

    void readRamPage(RegPtr dest, RegPtr index) override;
    void readMMU(RegPtr dest, RegPtr index) override;
    void readMMU(RegPtr dest, U32 index) override;
    void read(JitWidth width, RegPtr dest, RegPtr reg, U8 lsl, U32 disp) override;
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
    void IfNotTest(JitWidth regWidth, RegPtr reg, U32 value) override;
    void IfBitTest(RegPtr reg, U8 bit) override;
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
    void IfCompareReg(JitWidth regWidth, RegPtr reg1, RegPtr reg2, JitEvaluate condition) override;
    U32 MarkJumpLocation() override;
    void Goto(U32 location) override;
    void jmp(RegPtr reg) override;
    void updateFlagsIfNecessary();    
    RegPtr getReadOnlyFlags() override;
    void setFlags(RegPtr flags, U32 mask) override;

    void callHostFunction(void* address, const std::vector<DynParam>& params, bool restoreCache = true) override;
    void callHostFunctionWithResult(RegPtr result, void* address, const std::vector<DynParam>& params) override;
    void nakedCall(RegPtr reg) override;
    void nakedReturn() override;
#ifdef BOXEDWINE_64
    void setParam(X86Asm::Reg reg, const DynParam& param);
#else
    void pushParam(const DynParam& param);
#endif

    std::array<bool, NUMBER_OF_REGS> regUsed2{ 0 };
    std::array<bool, NUMBER_OF_XMM_REG> xmmUsed2{ 0 };

    void JumpInBlock(U32 address) override;
    void StartElse() override;
    void EndIf() override;
    void blockExit(bool syncCache = true) override;

    // FPU
    FPURegPtr getFPUTmp() override;
    void storeCpuFpuReg(FPURegPtr reg, RegPtr index) override;
    void loadCpuFpuReg(FPURegPtr reg, RegPtr index) override;
    void loadCpuFpuRegConst(FPURegPtr reg, U32 offset) override;
    void loadFpuRegFromInt(FPURegPtr reg, RegPtr rm, RegPtr sib, U8 lsl, U32 disp) override;
    void storeFpuReg(FPURegPtr reg, RegPtr rm, RegPtr sib, U8 lsl, U32 disp, DynFpuWidth width = DYN_FPU_64_BIT) override;
    RegPtr fpuRegToInt32(FPURegPtr fpuRegSrc, bool truncate) override;
    void fpuRegToInt64(FPURegPtr regDst, FPURegPtr fpuRegSrc, bool truncate) override;
    void fpuRegInt64To64(FPURegPtr regDst, FPURegPtr fpuRegSrc) override;
    void loadFpuReg(FPURegPtr reg, RegPtr rm, RegPtr sib, U8 lsl, U32 disp, DynFpuWidth width = DYN_FPU_64_BIT) override;
    void fpuRegExtend32To64(FPURegPtr dst, FPURegPtr src) override;
    void fpuReg64To32(FPURegPtr dst, FPURegPtr src) override;
    void regToFpuReg(FPURegPtr dst, RegPtr src) override;
#ifdef BOXEDWINE_64
    void regToFpuReg64(FPURegPtr dst, RegPtr src) override;
#endif
    void updateFPURounding() override;
    void restoreFPURounding() override;

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
    MMXRegPtr loadMMXFromReg(U8 index, RegPtr reg) override;
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
    bool isHintAvailable(S8 hint);
    U8 findTmpXMM();
    SSERegPtr getXMM(U8 index, bool load);

    X86Asm::RegXMM getMMXReg(MMXRegPtr reg);
    X86Asm::RegXMM getFPUReg(FPURegPtr reg);

    X86Asm x86;
#ifdef BOXEDWINE_64
    X86Asm::Reg64 params[4];
#endif
};

void JitX86CodeGen::preOp(DecodedOp* op) {
    regUsed2.fill(false);
    xmmUsed2.fill(false);
    currentOp = op;
}

U8 JitX86CodeGen::findTmpXMM() {
    for (int i = 0; i < NUMBER_OF_XMM_TMPS; i++) {
        U8 tmp = XMMtmps[i];
        if (!xmmUsed2[tmp]) {
            xmmUsed2[tmp] = true;
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

X86Asm::RegXMM JitX86CodeGen::getFPUReg(FPURegPtr reg) {
    return X86Asm::XMM(reg->hardwareReg());
}

SSERegPtr JitX86CodeGen::getTmpSSE() {
    return std::shared_ptr<SSERegInternal>(new SSERegInternal(findTmpXMM(), 0xff), [this](SSERegInternal* p) {
        xmmUsed2[p->hardwareReg()] = false;
        delete p;
    });
}

X86Asm::RegXMM JitX86CodeGen::getMMXReg(MMXRegPtr reg) {
    return X86Asm::XMM(reg->hardwareReg());
}

MMXRegPtr JitX86CodeGen::getTmpMMX() {
    return std::shared_ptr<MMXRegInternal>(new MMXRegInternal(findTmpXMM(), 0xff), [this](MMXRegInternal* p) {
        xmmUsed2[p->hardwareReg()] = false;
        delete p;
    });
}

FPURegPtr JitX86CodeGen::getFPUTmp() {
    return std::shared_ptr<FPURegInternal>(new FPURegInternal(findTmpXMM()), [this](FPURegInternal* p) {
        xmmUsed2[p->hardwareReg()] = false;
        delete p;
    });
}

void JitX86CodeGen::emulateSingleOp() {
    writeCurrentEip(0);
#ifdef BOXEDWINE_64
    x86.mov(PARAM_CALL_TMP, (DYN_PTR_SIZE)cpu->thread->process->emulateSingleOp);
    x86.jmp(PARAM_CALL_TMP);
#else
    x86.mov(x86.eax, (U32)cpu->thread->process->emulateSingleOp);
    x86.jmp(x86.eax);
#endif
}

bool JitX86CodeGen::isHintAvailable(S8 hint) {
    return (hint >= 0 && isTmp[hint] && !regUsed2[hint]);
}

U8 JitX86CodeGen::findTmpReg(bool needs8bitReg, S8 hint, bool allowInvalidReturn) {
#ifdef BOXEDWINE_64    
    if (isHintAvailable(hint)) {
        regUsed2[hint] = true;
        return (U8)hint;
    }
    for (int i = 0; i < NUMBER_OF_TMPS; i++) {
        U8 tmp = tmps[i];
        if (needs8bitReg && tmp > 3) {
            continue;
        }
        if (!regUsed2[tmp]) {
            regUsed2[tmp] = true;
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
    if (tmpReg == 0xff && !allowInvalidReturn) {
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
    U8 reg;

    if (regUsed2[0] || !isTmp[0]) {
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

RegPtr JitX86CodeGen::readEip() {
    RegPtr result = getTmpReg();
    x86.mov(R32(result->hardwareReg()), X86Asm::Mem32(HOST_CPU, offsetof(CPU, eip.u32)));    
    return result;
}

void JitX86CodeGen::writeEip(RegPtr reg) {
    x86.mov(X86Asm::Mem32(HOST_CPU, offsetof(CPU, eip.u32)), R32(reg->hardwareReg()));
}

void JitX86CodeGen::writeEip(U32 eip) {
    x86.mov(X86Asm::Mem32(HOST_CPU, offsetof(CPU, eip.u32)), eip);
}

void JitX86CodeGen::pushReg(RegPtr reg) {
    x86.push(RN(reg->hardwareReg()));
}

void JitX86CodeGen::popReg(RegPtr reg) {
    x86.pop(RN(reg->hardwareReg()));
}

bool JitX86CodeGen::isTmpRegAvailable() {
    U8 found = findTmpReg(false, -1, true);
    if (found == 0xff) {
        return false;
    }
    regUsed2[found] = false;
    return true;
}

void JitX86CodeGen::forceSyncBackIfNotCached(RegPtr reg) {
    if (reg->emulatedReg != 0xff && regCache[reg->emulatedReg] == INVALID_REG) {
        x86.mov(X86Asm::Mem32(HOST_CPU, CPU::offsetofReg32(reg->emulatedReg)), R32(reg->hardwareReg()));
    }
}

RegPtr JitX86CodeGen::getReg(U8 reg, S8 hint, bool load) {
    if (regCache[reg] == INVALID_REG) {
        RegPtr result = std::shared_ptr<JitReg>(new JitReg(findTmpReg(false, hint), reg), [this](JitReg* p) {
            x86.mov(X86Asm::Mem32(HOST_CPU, CPU::offsetofReg32(p->emulatedReg)), R32(p->hardwareReg()));
            regUsed2[p->hardwareReg()] = false;
            delete p;
            });
        if (load) {
            x86.mov(R32(result->hardwareReg()), X86Asm::Mem32(HOST_CPU, CPU::offsetofReg32(reg)));
        }
        return result;
    } else {
        if (isHintAvailable(hint)) {
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
#ifdef BOXEDWINE_64
    } else if (regWidth == JitWidth::b64) {
        x86.shr(R64(get8bitReg(reg)), (U8)imm);
#endif
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
#ifdef BOXEDWINE_64
    } else if (regWidth == JitWidth::b64) {
        if (rm->hardwareReg() != 1) {
            x86.mov(x86.cl, R8(get8bitReg(rm)));
        }
        x86.shl(R64(reg->hardwareReg()), x86.cl);
#endif
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

RegPtr JitX86CodeGen::testZeroReg(JitWidth regWidth, RegPtr reg, RegPtr result) {
    if (regWidth == JitWidth::b32) {
        x86.test(R32(reg->hardwareReg()), R32(reg->hardwareReg()));
    } else if (regWidth == JitWidth::b16) {
        x86.test(R16(reg->hardwareReg()), R16(reg->hardwareReg()));
    } else if (regWidth == JitWidth::b8) {
        x86.test(R8(get8bitReg(reg)), R8(get8bitReg(reg)));
    } else {
        kpanic_fmt("JitX86CodeGen::compareReg reg width %d", regWidth);
    }
    if (!result) {
        result = getTmpReg8();
    }
    x86.setz(R8(result->hardwareReg()));
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

void JitX86CodeGen::imulRR(JitWidth regWidth, RegPtr dst, RegPtr src, RegPtr overflow) {
    if (regWidth == JitWidth::b32) {
        if (!overflow) {
            x86.imul(R32(dst->hardwareReg()), R32(src->hardwareReg()));
        } else {
#ifdef BOXEDWINE_64
            x86.movsx(R64(src->hardwareReg()), R32(src->hardwareReg()));
            x86.movsx(R64(dst->hardwareReg()), R32(dst->hardwareReg()));
            x86.imul(R64(dst->hardwareReg()), R64(src->hardwareReg()));
            x86.mov(R64(overflow->hardwareReg()), R64(dst->hardwareReg()));
            x86.shr(R64(overflow->hardwareReg()), 32);
            x86.mov(R32(dst->hardwareReg()), R32(dst->hardwareReg())); // clear out top 32-bits
            x86.mov(R32(src->hardwareReg()), R32(src->hardwareReg())); // clear out top 32-bits (undo prev sign extend)
#else
            if (dst->hardwareReg() != 0 || overflow->hardwareReg() != 2) {
                kpanic("JitX86CodeGen::imulRRI overflow");
            }
            x86.imulEax(R32(src->hardwareReg()));
#endif
        }
    } else if (regWidth == JitWidth::b16) {
        if (overflow) {
            kpanic("JitX86CodeGen::imulRR overflow");
        }
        x86.imul(R16(dst->hardwareReg()), R16(src->hardwareReg()));
    } else {
        kpanic("JitX86CodeGen::imulRR");
    }
}

void JitX86CodeGen::imulRRI(JitWidth regWidth, RegPtr dst, RegPtr src, U32 src2, RegPtr overflow) {    
    if (regWidth == JitWidth::b32) {
        if (!overflow) {
            x86.imul(R32(dst->hardwareReg()), R32(src->hardwareReg()), src2);
        } else {
#ifdef BOXEDWINE_64
            x86.movsx(R64(src->hardwareReg()), R32(src->hardwareReg()));
            x86.movsx(R64(dst->hardwareReg()), R32(dst->hardwareReg()));
            x86.imul(R64(dst->hardwareReg()), R64(src->hardwareReg()), src2);
            x86.mov(R64(overflow->hardwareReg()), R64(dst->hardwareReg()));
            x86.shr(R64(overflow->hardwareReg()), 32);
            x86.mov(R32(dst->hardwareReg()), R32(dst->hardwareReg())); // clear out top 32-bits
            x86.mov(R32(src->hardwareReg()), R32(src->hardwareReg())); // clear out top 32-bits (undo prev sign extend)
#else
            if (dst->hardwareReg() != 0 || overflow->hardwareReg() != 2) {
                kpanic("JitX86CodeGen::imulRRI overflow");
            }
            x86.mov(x86.eax, R32(src->hardwareReg()));
            RegPtr tmp = getTmpReg();
            movValue(JitWidth::b32, tmp, src2);
            x86.imulEax(R32(tmp->hardwareReg()));
#endif
        }
    } else if (regWidth == JitWidth::b16) {
        if (overflow) {
            kpanic("JitX86CodeGen::imulRRI overflow");
        }
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

void JitX86CodeGen::absReg(JitWidth regWidth, RegPtr reg) {
    RegPtr tmp = getTmpReg();
#ifdef BOXEDWINE_64
    x86.mov(R64(tmp->hardwareReg()), R64(reg->hardwareReg()));
#else
    x86.mov(R32(tmp->hardwareReg()), R32(reg->hardwareReg()));    
#endif
    if (regWidth == JitWidth::b8) {
        x86.neg(R8(reg->hardwareReg()));
    } else if (regWidth == JitWidth::b16) {
        x86.neg(R16(reg->hardwareReg()));
    } else if (regWidth == JitWidth::b32) {
        x86.neg(R32(reg->hardwareReg()));
#ifdef BOXEDWINE_64
    } else if (regWidth == JitWidth::b64) {
        x86.neg(R64(reg->hardwareReg()));
        x86.cmovl(R64(reg->hardwareReg()), R64(tmp->hardwareReg()));
        return;
#endif
    } else {
        kpanic("JitX86CodeGen::divRegRegWithRemainder");
    }
    x86.cmovl(R32(reg->hardwareReg()), R32(tmp->hardwareReg()));
}

void JitX86CodeGen::clzReg(JitWidth regWidth, RegPtr result, RegPtr reg) {
    if (regWidth == JitWidth::b16) {
        x86.lzcnt(R16(result->hardwareReg()), R16(reg->hardwareReg()));
    } else if (regWidth == JitWidth::b32) {
        x86.lzcnt(R32(result->hardwareReg()), R32(reg->hardwareReg()));
#ifdef BOXEDWINE_64
    } else if (regWidth == JitWidth::b64) {
        x86.lzcnt(R64(result->hardwareReg()), R64(reg->hardwareReg()));
#endif
    } else {
        kpanic("JitX86CodeGen::clzReg");
    }
}

void JitX86CodeGen::divRegRegWithRemainder(JitWidth regWidth, RegPtr dest, RegPtr destHighAndRemainder, RegPtr src) {
    if (regWidth == JitWidth::b8) {
        x86.div(R8(get8bitReg(src)));
    } else if (regWidth == JitWidth::b16) {
        x86.div(R16(src->hardwareReg()));
    } else if (regWidth == JitWidth::b32) {
        x86.div(R32(src->hardwareReg()));
    } else {
        kpanic("JitX86CodeGen::divRegRegWithRemainder");
    }    
}

void JitX86CodeGen::idivRegRegWithRemainder(JitWidth regWidth, RegPtr dest, RegPtr destHighAndRemainder, RegPtr src) {
    if (regWidth == JitWidth::b8) {
        x86.idiv(R8(get8bitReg(src)));
    } else if (regWidth == JitWidth::b16) {
        x86.idiv(R16(src->hardwareReg()));
    } else if (regWidth == JitWidth::b32) {
        x86.idiv(R32(src->hardwareReg()));
    } else {
        kpanic("JitX86CodeGen::idivRegRegWithRemainder");
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
    x86.mov(R32(dest->hardwareReg()), X86Asm::Mem32(HOST_MMU, R(index->hardwareReg()), 2, 0));
#else
    read(JitWidth::b32, dest, index, 2, (U32)getMemData(KThread::currentThread()->memory)->mmu);
#endif
}

void JitX86CodeGen::readMMU(RegPtr dest, U32 index) {
#ifdef BOXEDWINE_64
    x86.mov(R32(dest->hardwareReg()), X86Asm::Mem32(HOST_MMU, index * 4));
#else
    x86.mov(R32(dest->hardwareReg()), X86Asm::Mem32((U32)getMemData(KThread::currentThread()->memory)->mmu + index * 4));
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
        if (width == JitWidth::b8) {
            reg = getTmpReg8();
        } else {
            reg = getTmpReg();
        }
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

RegPtr JitX86CodeGen::readCPU(JitWidth width, RegPtr sib, U8 lsl, U32 offset, RegPtr reg) {
    if (!reg) {
        if (width == JitWidth::b8) {
            reg = getTmpReg8();
        } else {
            reg = getTmpReg();
        }
    }
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
    U32 stackAdjust = 0;
    std::vector<U8> pushedRegs;

    for (int i = 0; i < NUMBER_OF_REGS; i++) {
        if (isVolitile[i] && isTmp[i] && regUsed2[i]) {
            x86.push(RN(i));
            pushedRegs.push_back(i);
        }
    }

    writeCache();
    setParams(params);    
#ifdef BOXEDWINE_64   
    if ((pushedRegs.size() % 2) == 0) {
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
        loadCache();
    }
    for (int i = pushedRegs.size() - 1; i >= 0; i--) {
        x86.pop(RN(pushedRegs[i]));
    }
}

void JitX86CodeGen::nakedCall(RegPtr reg) {
    x86.call(RN(reg->hardwareReg()));
}

void JitX86CodeGen::nakedReturn() {
    x86.ret();
}

void JitX86CodeGen::callHostFunctionWithResult(RegPtr result, void* address, const std::vector<DynParam>& params) {
    U32 stackAdjust = 0; // if 64-bit stack isn't aligned to 16-bytes, things like FPU::F2XM1()
    std::vector<U8> pushedRegs;

    for (int i = 0; i < NUMBER_OF_REGS; i++) {
        if (isVolitile[i] && isTmp[i] && regUsed2[i] && result->hardwareReg() != i) {
            x86.push(RN(i));
            pushedRegs.push_back(i);
        }
    }

    writeCache();
    setParams(params);
    
#ifdef BOXEDWINE_64
    if ((pushedRegs.size() % 2) == 0) {
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
    x86.mov(R32(result->hardwareReg()), x86.eax);
    for (int i = pushedRegs.size() - 1; i >= 0; i--) {
        x86.pop(RN(pushedRegs[i]));
    }
    loadCache();
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
        x86.test(R32(get8bitReg(reg)), R32(get8bitReg(mask)));
    } else {
        x86.test(R32(reg->hardwareReg()), R32(mask->hardwareReg()));
    }
    x86.jz();
}

void JitX86CodeGen::IfTest(JitWidth regWidth, RegPtr reg, U32 value) {
    if (regWidth == JitWidth::b8) {
        x86.test(R32(get8bitReg(reg)), value);
    } else {
        x86.test(R32(reg->hardwareReg()), value);
    }
    x86.jz();
}

void JitX86CodeGen::IfNotTest(JitWidth regWidth, RegPtr reg, U32 value) {
    if (regWidth == JitWidth::b8) {
        x86.test(R32(get8bitReg(reg)), value);
    } else {
        x86.test(R32(reg->hardwareReg()), value);
    }
    x86.jnz();
}

void JitX86CodeGen::IfBitTest(RegPtr reg, U8 bit) {
    x86.bt(R32(reg->hardwareReg()), bit);
    x86.jnb();
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

void JitX86CodeGen::IfGreaterThanOrEqual(JitWidth regWidth, RegPtr reg1, RegPtr reg2) {
    if (regWidth == JitWidth::b32) {
        x86.cmp(R32(reg1->hardwareReg()), R32(reg2->hardwareReg()));
    } else if (regWidth == JitWidth::b16) {
        x86.cmp(R16(reg1->hardwareReg()), R16(reg2->hardwareReg()));
    } else if (regWidth == JitWidth::b8) {
        x86.cmp(R8(get8bitReg(reg1)), R8(get8bitReg(reg2)));
    } else {
        kpanic_fmt("JitX86CodeGen::IfGreaterThanOrEqual unexpected width: %d", (U32)regWidth);
    }
    x86.jb();
}

void JitX86CodeGen::IfGreaterThanOrEqual(JitWidth regWidth, RegPtr reg1, U32 value) {
    if (regWidth == JitWidth::b32) {
        x86.cmp(R32(reg1->hardwareReg()), value);
    } else if (regWidth == JitWidth::b16) {
        x86.cmp(R16(reg1->hardwareReg()), (U16)value);
    } else if (regWidth == JitWidth::b8) {
        x86.cmp(R8(get8bitReg(reg1)), (U8)value);
    } else {
        kpanic_fmt("JitX86CodeGen::IfGreaterThanOrEqual unexpected width: %d", (U32)regWidth);
    }
    x86.jb();
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

void JitX86CodeGen::IfDF() {
    x86.test(X86Asm::Mem32(HOST_CPU, offsetof(CPU, flags)), DF);
    x86.jz();

    //x86.bt(X86Asm::Mem32(HOST_CPU, offsetof(CPU, flags)), 10);
    //x86.jnb();
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
    S32 amount = (S32)location - (U32)x86.buffer.size() - 1;

    if (amount > 127 || amount < -127) {
        amount = location - (U32)x86.buffer.size() - 5;
        x86.goto32(amount);
    } else {
        x86.goto8(amount - 1);
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

void JitX86CodeGen::IfSseLessThan(SSERegPtr src1, SSERegPtr src2) {
    x86.ucomiss(X86Asm::XMM(src1->hardwareReg()), X86Asm::XMM(src2->hardwareReg()));
    x86.IfCF();
}

void JitX86CodeGen::storeCpuXMMReg(SSERegPtr reg, U32 index) {
    if (index >= 8) {
        kpanic("JitX86CodeGen::storeCpuXMMReg");
        return;
    }
    if (xmmCache[index] == INVALID_REG) {
        x86.movaps(X86Asm::Mem128(HOST_CPU, index * 16 + offsetof(CPU, xmm)), X86Asm::XMM(reg->hardwareReg()));
    } else if (xmmCache[index] != reg->hardwareReg()) {
        x86.movaps(X86Asm::XMM(xmmCache[index]), X86Asm::XMM(reg->hardwareReg()));
    }    
}

void JitX86CodeGen::storeXMMToMem128(SSERegPtr reg, RegPtr rm, RegPtr sib, U8 lsl, U32 disp) {
    x86.movups(X86Asm::Mem128(R(rm->hardwareReg()), R(sib->hardwareReg()), lsl, disp), X86Asm::XMM(reg->hardwareReg()));
}

void JitX86CodeGen::storeXMMToMem64(SSERegPtr reg, RegPtr rm, RegPtr sib, U8 lsl, U32 disp) {
    x86.movlps(X86Asm::Mem64(R(rm->hardwareReg()), R(sib->hardwareReg()), lsl, disp), X86Asm::XMM(reg->hardwareReg()));
}

void JitX86CodeGen::storeXMMToMem32(SSERegPtr reg, RegPtr rm, RegPtr sib, U8 lsl, U32 disp) {
    x86.movss(X86Asm::Mem32(R(rm->hardwareReg()), R(sib->hardwareReg()), lsl, disp), X86Asm::XMM(reg->hardwareReg()));
}

void JitX86CodeGen::storeHighXMMToMem64(SSERegPtr reg, RegPtr rm, RegPtr sib, U8 lsl, U32 disp) {
    x86.movhps(X86Asm::Mem64(R(rm->hardwareReg()), R(sib->hardwareReg()), lsl, disp), X86Asm::XMM(reg->hardwareReg()));
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
        xmmUsed2[p->hardwareReg()] = false;
        delete p;
    });
    if (load && index != INVALID_REG) {
        x86.movaps(X86Asm::XMM(result->hardwareReg()), X86Asm::Mem128(HOST_CPU, index * 16 + offsetof(CPU, xmm)));
    }
    return result;
}

SSERegPtr JitX86CodeGen::loadCpuXMMReg(U8 index) {        
    return getXMM(index, true);
}

SSERegPtr JitX86CodeGen::loadXMMFromMem128(U8 index, RegPtr rm, RegPtr sib, U8 lsl, U32 disp) {
    SSERegPtr reg = getXMM(index, false);
    x86.movups(X86Asm::XMM(reg->hardwareReg()), X86Asm::Mem128(R(rm->hardwareReg()), R(sib->hardwareReg()), lsl, disp));
    return reg;
}

SSERegPtr JitX86CodeGen::loadXMMFromMem64(U8 index, RegPtr rm, RegPtr sib, U8 lsl, U32 disp) {
    SSERegPtr reg = getXMM(index, true);
    x86.movq(X86Asm::XMM(reg->hardwareReg()), X86Asm::Mem64(R(rm->hardwareReg()), R(sib->hardwareReg()), lsl, disp));
    return reg;
}

SSERegPtr JitX86CodeGen::loadLowXMMFromMem64(U8 index, RegPtr rm, RegPtr sib, U8 lsl, U32 disp) {
    SSERegPtr reg = getXMM(index, true);
    x86.movlps(X86Asm::XMM(reg->hardwareReg()), X86Asm::Mem64(R(rm->hardwareReg()), R(sib->hardwareReg()), lsl, disp));
    return reg;
}

SSERegPtr JitX86CodeGen::loadHighXMMFromMem64(U8 index, RegPtr rm, RegPtr sib, U8 lsl, U32 disp) {
    SSERegPtr reg = getXMM(index, true);
    x86.movhps(X86Asm::XMM(reg->hardwareReg()), X86Asm::Mem64(R(rm->hardwareReg()), R(sib->hardwareReg()), lsl, disp));
    return reg;
}

SSERegPtr JitX86CodeGen::loadXMMFromMem32(U8 index, RegPtr rm, RegPtr sib, U8 lsl, U32 disp) {
    SSERegPtr reg = getXMM(index, true);
    x86.movss(X86Asm::XMM(reg->hardwareReg()), X86Asm::Mem32(R(rm->hardwareReg()), R(sib->hardwareReg()), lsl, disp));
    return reg;
}

void JitX86CodeGen::addpsXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.addps(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::addssXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.addss(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::subpsXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.subps(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::subssXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.subss(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::mulpsXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.mulps(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::mulssXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.mulss(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::divpsXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.divps(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::divssXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.divss(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::rcppsXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.rcpps(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::rcpssXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.rcpss(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::sqrtpsXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.sqrtps(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::sqrtssXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.sqrtss(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::rsqrtpsXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.rsqrtps(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::rsqrtssXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.rsqrtss(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::maxpsXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.maxps(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::maxssXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.maxss(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::minpsXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.minps(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::minssXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.minss(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::pavgbMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    x86.pavgb(getMMXReg(dst), getMMXReg(src));
}

void JitX86CodeGen::pavgwMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    x86.pavgw(getMMXReg(dst), getMMXReg(src));
}

void JitX86CodeGen::psadbwMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    x86.psadbw(getMMXReg(dst), getMMXReg(src));
}

void JitX86CodeGen::pextrwRegMmx(RegPtr dst, MMXRegPtr src, U8 srcIndex) {
    x86.pextrw(R32(dst->hardwareReg()), getMMXReg(src), srcIndex);
}

void JitX86CodeGen::pinsrwMmxReg(MMXRegPtr dst, RegPtr src, U8 dstIndex) {
    x86.pinsrw(getMMXReg(dst), R32(src->hardwareReg()), dstIndex);
}

void JitX86CodeGen::pmaxswMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    x86.pmaxsw(getMMXReg(dst), getMMXReg(src));
}

void JitX86CodeGen::pmaxubMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    x86.pmaxub(getMMXReg(dst), getMMXReg(src));
}

void JitX86CodeGen::pminswMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    x86.pminsw(getMMXReg(dst), getMMXReg(src));
}

void JitX86CodeGen::pminubMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    x86.pminub(getMMXReg(dst), getMMXReg(src));
}

void JitX86CodeGen::pmovmskbMmxMmx(RegPtr dst, MMXRegPtr src) {
    x86.pmovmskb(R32(dst->hardwareReg()), getMMXReg(src));
}

void JitX86CodeGen::pmulhuwMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    x86.pmulhuw(getMMXReg(dst), getMMXReg(src));
}

void JitX86CodeGen::pshufwMmxMmx(MMXRegPtr dst, MMXRegPtr src, U8 order) {
    x86.pshuflw(getMMXReg(dst), getMMXReg(src), order);
}

void JitX86CodeGen::maskmovq(MMXRegPtr src, MMXRegPtr mask, RegPtr destAddress) {
    x86.push(RN(7));
    x86.mov(RN(7), RN(destAddress->hardwareReg()));
    // this works because the top 64-bits of the mask should be 0's since its used for MMX
    x86.maskmovdqu(getMMXReg(src), getMMXReg(mask));
    x86.pop(RN(7));
}

void JitX86CodeGen::paddqMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    x86.paddq(getMMXReg(dst), getMMXReg(src));
}

void JitX86CodeGen::psubqMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    x86.psubq(getMMXReg(dst), getMMXReg(src));
}

void JitX86CodeGen::pmuludqMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    x86.pmuludq(getMMXReg(dst), getMMXReg(src));
}

void JitX86CodeGen::andnpsXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.andnps(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::andpsXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.andps(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::orpsXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.orps(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::xorpsXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.xorps(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::cvtpi2psXmmMmx(SSERegPtr dst, MMXRegPtr src) {
    // cvtpi2ps need to keep top 64-bits of the xmm dst
    SSERegPtr tmp = getTmpSSE();
    x86.cvtdq2ps(X86Asm::XMM(tmp->hardwareReg()), getMMXReg(src));
    x86.movsd(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(tmp->hardwareReg()));
}

void JitX86CodeGen::cvtps2piMmxXmm(MMXRegPtr dst, SSERegPtr src) {
    x86.cvtps2dq(getMMXReg(dst), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::cvtsi2ssXmmR32(SSERegPtr dst, RegPtr src) {
    x86.cvtsi2ss(X86Asm::XMM(dst->hardwareReg()), R32(src->hardwareReg()));
}

void JitX86CodeGen::cvtss2siR32Xmm(RegPtr dst, SSERegPtr src) {
    x86.cvtss2si(R32(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::cvttps2piMmxXmm(MMXRegPtr dst, SSERegPtr src) {
    x86.cvttps2dq(getMMXReg(dst), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::cvttss2siR32Xmm(RegPtr dst, SSERegPtr src) {
    x86.cvttss2si(R32(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::movhlpsXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.movhlps(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::movlhpsXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.movlhps(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::movssXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.movss(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::shufpsXmmXmm(SSERegPtr dst, SSERegPtr src, U32 imm) {
    x86.shufps(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()), (U8)imm);
}

void JitX86CodeGen::cmppsXmmXmm(SSERegPtr dst, SSERegPtr src, U32 imm) {
    x86.cmpps(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()), (U8)imm);
}

void JitX86CodeGen::cmpssXmmXmm(SSERegPtr dst, SSERegPtr src, U32 imm) {
    x86.cmpss(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()), (U8)imm);
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

void JitX86CodeGen::comissXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.comiss(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
    updateFlagsIfNecessary();
}

void JitX86CodeGen::ucomissXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.ucomiss(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
    updateFlagsIfNecessary();
}

void JitX86CodeGen::stmxcsr(RegPtr address) {
    x86.stmxcsr(X86Asm::Mem32(RN(address->hardwareReg()), 0));
}

void JitX86CodeGen::ldmxcsr(RegPtr address) {
    x86.ldmxcsr(X86Asm::Mem32(RN(address->hardwareReg()), 0));
}

void JitX86CodeGen::sfence() {
    x86.sfence();
}

void JitX86CodeGen::unpckhpsXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.unpckhps(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::unpcklpsXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.unpcklps(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::movmskpsR32Xmm(RegPtr dst, SSERegPtr src) {
    x86.movmskps(R32(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

MMXRegPtr JitX86CodeGen::loadMMXFromReg(U8 index, RegPtr src) {
    MMXRegPtr tmp = getTmpMMX();
    x86.movd(getMMXReg(tmp), R32(src->hardwareReg()));
    return tmp;
}

void JitX86CodeGen::storeCpuMMXReg(MMXRegPtr reg, U32 index) {
    x86.movq(X86Asm::Mem64(HOST_CPU, index * cpu->fpu.sizeofRegInRegsArray() + offsetof(CPU, fpu.regs[0].signif)), getMMXReg(reg));
}

void JitX86CodeGen::storeMMXToReg(MMXRegPtr src, RegPtr dst) {
    x86.movd(R32(dst->hardwareReg()), getMMXReg(src));
}

MMXRegPtr JitX86CodeGen::loadCpuMMXReg(U8 index) {
    MMXRegPtr tmp = getTmpMMX();
    x86.movq(getMMXReg(tmp), X86Asm::Mem64(HOST_CPU, index * cpu->fpu.sizeofRegInRegsArray() + offsetof(CPU, fpu.regs[0].signif)));
    return tmp;
}

MMXRegPtr JitX86CodeGen::loadMMXFromMem32(U8 index, RegPtr rm, RegPtr sib, U8 lsl, U32 disp) {
    MMXRegPtr tmp = getTmpMMX();
    x86.movd(getMMXReg(tmp), X86Asm::Mem32(RN(rm->hardwareReg()), RN(sib->hardwareReg()), lsl, disp));
    return tmp;
}

MMXRegPtr JitX86CodeGen::loadMMXFromMem64(U8 index, RegPtr rm, RegPtr sib, U8 lsl, U32 disp) {
    MMXRegPtr tmp = getTmpMMX();
    x86.movq(getMMXReg(tmp), X86Asm::Mem64(RN(rm->hardwareReg()), RN(sib->hardwareReg()), lsl, disp));
    return tmp;
}

void JitX86CodeGen::storeMMXToMem32(MMXRegPtr reg, RegPtr rm, RegPtr sib, U8 lsl, U32 disp) {
    x86.movd(X86Asm::Mem32(RN(rm->hardwareReg()), RN(sib->hardwareReg()), lsl, disp), getMMXReg(reg));
}

void JitX86CodeGen::storeMMXToMem64(MMXRegPtr reg, RegPtr rm, RegPtr sib, U8 lsl, U32 disp) {
    x86.movq(X86Asm::Mem64(RN(rm->hardwareReg()), RN(sib->hardwareReg()), lsl, disp), getMMXReg(reg));
}

void JitX86CodeGen::xorMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    x86.pxor(getMMXReg(dst), getMMXReg(src));
}

void JitX86CodeGen::orMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    x86.por(getMMXReg(dst), getMMXReg(src));
}

void JitX86CodeGen::andMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    x86.pand(getMMXReg(dst), getMMXReg(src));
}

void JitX86CodeGen::andnMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    x86.pandn(getMMXReg(dst), getMMXReg(src));
}

void JitX86CodeGen::psllwMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    x86.psllw(getMMXReg(dst), getMMXReg(src));
}

void JitX86CodeGen::psrlwMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    x86.psrlw(getMMXReg(dst), getMMXReg(src));
}

void JitX86CodeGen::psrawMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    x86.psraw(getMMXReg(dst), getMMXReg(src));
}

void JitX86CodeGen::psllwMmx(MMXRegPtr dst, U32 imm) {
    x86.psllw(getMMXReg(dst), imm);
}

void JitX86CodeGen::psrlwMmx(MMXRegPtr dst, U32 imm) {
    x86.psrlw(getMMXReg(dst), imm);
}

void JitX86CodeGen::psrawMmx(MMXRegPtr dst, U32 imm) {
    x86.psraw(getMMXReg(dst), imm);
}

void JitX86CodeGen::pslldMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    x86.pslld(getMMXReg(dst), getMMXReg(src));
}

void JitX86CodeGen::psrldMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    x86.psrld(getMMXReg(dst), getMMXReg(src));
}

void JitX86CodeGen::psradMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    x86.psrad(getMMXReg(dst), getMMXReg(src));
}

void JitX86CodeGen::pslldMmx(MMXRegPtr dst, U32 imm) {
    x86.pslld(getMMXReg(dst), imm);
}

void JitX86CodeGen::psrldMmx(MMXRegPtr dst, U32 imm) {
    x86.psrld(getMMXReg(dst), imm);
}

void JitX86CodeGen::psradMmx(MMXRegPtr dst, U32 imm) {
    x86.psrad(getMMXReg(dst), imm);
}

void JitX86CodeGen::psllqMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    x86.psllq(getMMXReg(dst), getMMXReg(src));
}

void JitX86CodeGen::psrlqMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    x86.psrlq(getMMXReg(dst), getMMXReg(src));
}

void JitX86CodeGen::psllqMmx(MMXRegPtr dst, U32 imm) {
    x86.psllq(getMMXReg(dst), imm);
}

void JitX86CodeGen::psrlqMmx(MMXRegPtr dst, U32 imm) {
    x86.psrlq(getMMXReg(dst), imm);
}

void JitX86CodeGen::paddbMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    x86.paddb(getMMXReg(dst), getMMXReg(src));
}

void JitX86CodeGen::paddwMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    x86.paddw(getMMXReg(dst), getMMXReg(src));
}

void JitX86CodeGen::padddMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    x86.paddd(getMMXReg(dst), getMMXReg(src));
}

void JitX86CodeGen::paddsbMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    x86.paddsb(getMMXReg(dst), getMMXReg(src));
}

void JitX86CodeGen::paddswMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    x86.paddsw(getMMXReg(dst), getMMXReg(src));
}

void JitX86CodeGen::paddusbMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    x86.paddusb(getMMXReg(dst), getMMXReg(src));
}

void JitX86CodeGen::padduswMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    x86.paddusw(getMMXReg(dst), getMMXReg(src));
}

void JitX86CodeGen::psubbMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    x86.psubb(getMMXReg(dst), getMMXReg(src));
}

void JitX86CodeGen::psubwMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    x86.psubw(getMMXReg(dst), getMMXReg(src));
}

void JitX86CodeGen::psubdMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    x86.psubd(getMMXReg(dst), getMMXReg(src));
}

void JitX86CodeGen::psubsbMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    x86.psubsb(getMMXReg(dst), getMMXReg(src));
}

void JitX86CodeGen::psubswMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    x86.psubsw(getMMXReg(dst), getMMXReg(src));
}

void JitX86CodeGen::psubusbMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    x86.psubusb(getMMXReg(dst), getMMXReg(src));
}

void JitX86CodeGen::psubuswMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    x86.psubusw(getMMXReg(dst), getMMXReg(src));
}

void JitX86CodeGen::pmulhwMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    x86.pmulhw(getMMXReg(dst), getMMXReg(src));
}

void JitX86CodeGen::pmullwMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    x86.pmullw(getMMXReg(dst), getMMXReg(src));
}

void JitX86CodeGen::pmaddwdMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    x86.pmaddwd(getMMXReg(dst), getMMXReg(src));
}

void JitX86CodeGen::pcmpeqbMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    x86.pcmpeqb(getMMXReg(dst), getMMXReg(src));
}

void JitX86CodeGen::pcmpeqwMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    x86.pcmpeqw(getMMXReg(dst), getMMXReg(src));
}

void JitX86CodeGen::pcmpeqdMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    x86.pcmpeqd(getMMXReg(dst), getMMXReg(src));
}

void JitX86CodeGen::pcmpgtbMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    x86.pcmpgtb(getMMXReg(dst), getMMXReg(src));
}

void JitX86CodeGen::pcmpgtwMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    x86.pcmpgtw(getMMXReg(dst), getMMXReg(src));
}

void JitX86CodeGen::pcmpgtdMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    x86.pcmpgtd(getMMXReg(dst), getMMXReg(src));
}

void JitX86CodeGen::packsswbMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    // xmm is twice as big as mm, so we need to adjust a little before the pack
    x86.movlhps(getMMXReg(dst), getMMXReg(src));
    x86.packsswb(getMMXReg(dst), getMMXReg(dst));
}

void JitX86CodeGen::packssdwMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    // xmm is twice as big as mm, so we need to adjust a little before the pack
    x86.movlhps(getMMXReg(dst), getMMXReg(src));
    x86.packssdw(getMMXReg(dst), getMMXReg(dst));
}

void JitX86CodeGen::packuswbMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    // xmm is twice as big as mm, so we need to adjust a little before the pack
    x86.movlhps(getMMXReg(dst), getMMXReg(src));
    x86.packuswb(getMMXReg(dst), getMMXReg(dst));
}

void JitX86CodeGen::punpckhbwMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    // :TODO: maybe move bytes 4-7 to 8-11 instead of 0-7 to 8-15 so that we don't have to do the movhlps to mov them back down?
    x86.movlhps(getMMXReg(src), getMMXReg(src));
    x86.movlhps(getMMXReg(dst), getMMXReg(dst));
    x86.punpckhbw(getMMXReg(dst), getMMXReg(src));
    x86.movhlps(getMMXReg(dst), getMMXReg(dst));
}

void JitX86CodeGen::punpckhwdMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    x86.movlhps(getMMXReg(src), getMMXReg(src));
    x86.movlhps(getMMXReg(dst), getMMXReg(dst));
    x86.punpckhwd(getMMXReg(dst), getMMXReg(src));
    x86.movhlps(getMMXReg(dst), getMMXReg(dst));
}

void JitX86CodeGen::punpckhdqMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    x86.movlhps(getMMXReg(src), getMMXReg(src));
    x86.movlhps(getMMXReg(dst), getMMXReg(dst));
    x86.punpckhdq(getMMXReg(dst), getMMXReg(src));
    x86.movhlps(getMMXReg(dst), getMMXReg(dst));
}

void JitX86CodeGen::punpcklbwMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    x86.punpcklbw(getMMXReg(dst), getMMXReg(src));
}

void JitX86CodeGen::punpcklwdMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    x86.punpcklwd(getMMXReg(dst), getMMXReg(src));
}

void JitX86CodeGen::punpckldqMmxMmx(MMXRegPtr dst, MMXRegPtr src) {
    x86.punpckldq(getMMXReg(dst), getMMXReg(src));
}

#ifdef BOXEDWINE_64
void JitX86CodeGen::cvtsi2sdXmmR64(SSERegPtr dst, RegPtr src) {
    x86.cvtsi2sd(X86Asm::XMM(dst->hardwareReg()), R64(src->hardwareReg()));
}
#endif

void JitX86CodeGen::addpdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.addpd(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::addsdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.addsd(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::subpdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.subpd(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::subsdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.subsd(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::mulpdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.mulpd(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::mulsdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.mulsd(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::divpdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.divpd(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::divsdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.divsd(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::maxpdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.maxpd(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::maxsdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.maxsd(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::minpdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.minpd(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::minsdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.minsd(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::paddbXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.paddb(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::paddwXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.paddw(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::padddXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.paddd(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::paddqXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.paddq(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::paddsbXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.paddsb(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::paddswXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.paddsw(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::paddusbXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.paddusb(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::padduswXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.paddusw(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::psubbXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.psubb(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::psubwXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.psubw(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::psubdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.psubd(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::psubqXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.psubq(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::psubsbXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.psubsb(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::psubswXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.psubsw(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::psubusbXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.psubusb(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::psubuswXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.psubusw(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::pmaddwdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.pmaddwd(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::pmulhwXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.pmulhw(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::pmullwXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.pmullw(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::pmuludqXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.pmuludq(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::sqrtpdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.sqrtpd(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::sqrtsdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.sqrtsd(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::andnpdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.andnpd(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::andpdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.andpd(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::pandXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.pand(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::pandnXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.pandn(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::porXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.por(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::pslldqXmm(SSERegPtr dst, U32 imm) {
    x86.pslldq(X86Asm::XMM(dst->hardwareReg()), imm);
}

void JitX86CodeGen::psllqXmm(SSERegPtr dst, U32 imm) {
    x86.psllq(X86Asm::XMM(dst->hardwareReg()), imm);
}

void JitX86CodeGen::pslldXmm(SSERegPtr dst, U32 imm) {
    x86.pslld(X86Asm::XMM(dst->hardwareReg()), imm);
}

void JitX86CodeGen::psllwXmm(SSERegPtr dst, U32 imm) {
    x86.psllw(X86Asm::XMM(dst->hardwareReg()), imm);
}

void JitX86CodeGen::psradXmm(SSERegPtr dst, U32 imm) {
    x86.psrad(X86Asm::XMM(dst->hardwareReg()), imm);
}

void JitX86CodeGen::psrawXmm(SSERegPtr dst, U32 imm) {
    x86.psraw(X86Asm::XMM(dst->hardwareReg()), imm);
}

void JitX86CodeGen::psrldqXmm(SSERegPtr dst, U32 imm) {
    x86.psrldq(X86Asm::XMM(dst->hardwareReg()), imm);
}

void JitX86CodeGen::psrlqXmm(SSERegPtr dst, U32 imm) {
    x86.psrlq(X86Asm::XMM(dst->hardwareReg()), imm);
}

void JitX86CodeGen::psrldXmm(SSERegPtr dst, U32 imm) {
    x86.psrld(X86Asm::XMM(dst->hardwareReg()), imm);
}

void JitX86CodeGen::psrlwXmm(SSERegPtr dst, U32 imm) {
    x86.psrlw(X86Asm::XMM(dst->hardwareReg()), imm);
}

void JitX86CodeGen::psllqXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.psllq(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::pslldXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.pslld(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::psllwXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.psllw(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::psradXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.psrad(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::psrawXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.psraw(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::psrlqXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.psrlq(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::psrldXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.psrld(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::psrlwXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.psrlw(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::pxorXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.pxor(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::orpdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.orpd(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::xorpdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.xorpd(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::cmppdXmmXmm(SSERegPtr dst, SSERegPtr src, U32 imm) {
    x86.cmppd(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()), (U8)imm);
}

void JitX86CodeGen::cmpsdXmmXmm(SSERegPtr dst, SSERegPtr src, U32 imm) {
    x86.cmpsd(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()), (U8)imm);
}

void JitX86CodeGen::comisdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.comisd(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
    updateFlagsIfNecessary();
}

void JitX86CodeGen::ucomisdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.ucomisd(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
    updateFlagsIfNecessary();
}

void JitX86CodeGen::pcmpgtbXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.pcmpgtb(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::pcmpgtwXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.pcmpgtw(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::pcmpgtdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.pcmpgtd(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::pcmpeqbXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.pcmpeqb(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::pcmpeqwXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.pcmpeqw(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::pcmpeqdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.pcmpeqd(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::cvtdq2pdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.cvtdq2pd(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::cvtdq2psXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.cvtdq2ps(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::cvtpd2piMmxXmm(MMXRegPtr dst, SSERegPtr src) {
    x86.cvtpd2dq(getMMXReg(dst), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::cvtpi2pdXmmMmx(SSERegPtr dst, MMXRegPtr src) {
    x86.cvtdq2pd(X86Asm::XMM(dst->hardwareReg()), getMMXReg(src));
}

void JitX86CodeGen::cvtpd2dqXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.cvtpd2dq(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::cvtpd2psXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.cvtpd2ps(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::cvttpd2piMmxXmm(MMXRegPtr dst, SSERegPtr src) {
    x86.cvttpd2dq(getMMXReg(dst), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::cvtps2dqXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.cvtps2dq(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::cvtps2pdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.cvtps2pd(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::cvtsd2siR32Xmm(RegPtr dst, SSERegPtr src) {
    x86.cvtsd2si(R32(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::cvtsd2ssXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.cvtsd2ss(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::cvtsi2sdXmmR32(SSERegPtr dst, RegPtr src) {
    x86.cvtsi2sd(X86Asm::XMM(dst->hardwareReg()), R32(src->hardwareReg()));
}

void JitX86CodeGen::cvtss2sdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.cvtss2sd(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::cvttpd2dqXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.cvttpd2dq(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::cvttps2dqXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.cvttps2dq(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::cvttsd2siR32Xmm(RegPtr dst, SSERegPtr src) {
    x86.cvttsd2si(R32(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::movsdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.movsd(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::movupdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.movdqu(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::movmskpd(RegPtr dst, SSERegPtr src) {
    x86.movmskpd(R32(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::movd(RegPtr dst, SSERegPtr src) {
    x86.movd(R32(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::movd(SSERegPtr dst, RegPtr src) {
    x86.movd(X86Asm::XMM(dst->hardwareReg()), R32(src->hardwareReg()));
}

void JitX86CodeGen::movdq2q(MMXRegPtr dst, SSERegPtr src) {
    x86.movq(getMMXReg(dst), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::movq2dq(SSERegPtr dst, MMXRegPtr src) {
    x86.movq(X86Asm::XMM(dst->hardwareReg()), getMMXReg(src));
}

void JitX86CodeGen::movq(SSERegPtr dst, SSERegPtr src) {
    x86.movq(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::maskmovdqu(SSERegPtr src, SSERegPtr mask, RegPtr address) {
    x86.push(RN(7));
    x86.mov(RN(7), RN(address->hardwareReg()));
    x86.maskmovdqu(X86Asm::XMM(src->hardwareReg()), X86Asm::XMM(mask->hardwareReg()));
    x86.pop(RN(7));
}

void JitX86CodeGen::pshufdXmmXmm(SSERegPtr dst, SSERegPtr src, U32 imm) {
    x86.pshufd(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()), imm);
}

void JitX86CodeGen::pshufhwXmmXmm(SSERegPtr dst, SSERegPtr src, U32 imm) {
    x86.pshufhw(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()), imm);
}

void JitX86CodeGen::pshuflwXmmXmm(SSERegPtr dst, SSERegPtr src, U32 imm) {
    x86.pshuflw(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()), imm);
}

void JitX86CodeGen::shufpdXmmXmm(SSERegPtr dst, SSERegPtr src, U32 imm) {
    x86.shufpd(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()), imm);
}

void JitX86CodeGen::unpckhpdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.unpckhpd(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::unpcklpdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.unpcklpd(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::punpckhbwXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.punpckhbw(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::punpckhwdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.punpckhwd(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::punpckhdqXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.punpckhdq(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::punpckhqdqXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.punpckhqdq(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::punpcklbwXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.punpcklbw(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::punpcklwdXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.punpcklwd(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::punpckldqXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.punpckldq(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::punpcklqdqXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.punpcklqdq(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::packssdwXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.packssdw(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::packsswbXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.packsswb(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::packuswbXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.packuswb(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::pavgbXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.pavgb(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::pavgwXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.pavgw(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::psadbwXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.psadbw(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::pmaxswXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.pmaxsw(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::pmaxubXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.pmaxub(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::pminswXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.pminsw(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::pminubXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.pminub(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
}

void JitX86CodeGen::pmulhuwXmmXmm(SSERegPtr dst, SSERegPtr src) {
    x86.pmulhuw(X86Asm::XMM(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
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

void JitX86CodeGen::pextrwR32Xmm(RegPtr dst, SSERegPtr src, U32 imm) {
    x86.pextrw(R32(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()), (U8)imm);
}

void JitX86CodeGen::pinsrwXmmR32(SSERegPtr dst, RegPtr src, U32 imm) {
    x86.pinsrw(X86Asm::XMM(dst->hardwareReg()), R32(src->hardwareReg()), (U8)imm);
}

void JitX86CodeGen::pmovmskbR32Xmm(RegPtr dst, SSERegPtr src) {
    x86.pmovmskb(R32(dst->hardwareReg()), X86Asm::XMM(src->hardwareReg()));
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

void JitX86CodeGen::storeCpuFpuReg(FPURegPtr reg, RegPtr index) {
    x86.movsd(X86Asm::Mem64(HOST_CPU, R32(index->hardwareReg()), 3, offsetof(CPU, fpu.regCache[0].d)), getFPUReg(reg));
}

void JitX86CodeGen::loadCpuFpuReg(FPURegPtr reg, RegPtr index) {
    x86.movsd(getFPUReg(reg), X86Asm::Mem64(HOST_CPU, R32(index->hardwareReg()), 3, offsetof(CPU, fpu.regCache[0].d)));
}

void JitX86CodeGen::loadCpuFpuRegConst(FPURegPtr reg, U32 offset) {
    x86.movsd(getFPUReg(reg), X86Asm::Mem64(HOST_CPU, offset));
}

RegPtr JitX86CodeGen::fpuRegToInt32(FPURegPtr fpuRegSrc, bool truncate) {
    RegPtr result = getTmpReg();
    if (truncate) {
        x86.cvttsd2si(R32(result->hardwareReg()), getFPUReg(fpuRegSrc));
    } else {
        x86.cvtsd2si(R32(result->hardwareReg()), getFPUReg(fpuRegSrc));
    }
    return result;
}

void JitX86CodeGen::fpuRegToInt64(FPURegPtr regDst, FPURegPtr fpuRegSrc, bool truncate) {
    if (truncate) {
        x86.cvttpd2dq(getFPUReg(regDst), getFPUReg(fpuRegSrc));
    } else {
        x86.cvtpd2dq(getFPUReg(regDst), getFPUReg(fpuRegSrc));
    }
}

void JitX86CodeGen::fpuRegInt64To64(FPURegPtr regDst, FPURegPtr fpuRegSrc) {
    x86.cvtdq2pd(getFPUReg(regDst), getFPUReg(fpuRegSrc));
}

void JitX86CodeGen::storeFpuReg(FPURegPtr reg, RegPtr rm, RegPtr sib, U8 lsl, U32 disp, DynFpuWidth width) {
    if (width == DYN_FPU_64_BIT) {
        x86.movsd(X86Asm::Mem64(RN(rm->hardwareReg()), RN(sib->hardwareReg()), lsl, disp), getFPUReg(reg));
    } else {
        x86.movss(X86Asm::Mem32(RN(rm->hardwareReg()), RN(sib->hardwareReg()), lsl, disp), getFPUReg(reg));
    }
}

void JitX86CodeGen::loadFpuReg(FPURegPtr reg, RegPtr rm, RegPtr sib, U8 lsl, U32 disp, DynFpuWidth width) {
    if (width == DYN_FPU_64_BIT) {
        x86.movsd(getFPUReg(reg), X86Asm::Mem64(RN(rm->hardwareReg()), RN(sib->hardwareReg()), lsl, disp));
    } else {
        x86.movss(getFPUReg(reg), X86Asm::Mem32(RN(rm->hardwareReg()), RN(sib->hardwareReg()), lsl, disp));
    }
}

void JitX86CodeGen::fpuRegExtend32To64(FPURegPtr dst, FPURegPtr src) {
    x86.cvtss2sd(getFPUReg(dst), getFPUReg(src));
}

void JitX86CodeGen::fpuReg64To32(FPURegPtr dst, FPURegPtr src) {
    x86.cvtsd2ss(getFPUReg(dst), getFPUReg(src));
}

void JitX86CodeGen::loadFpuRegFromInt(FPURegPtr reg, RegPtr rm, RegPtr sib, U8 lsl, U32 disp) {
    x86.cvtsi2sd(getFPUReg(reg), X86Asm::Mem32(RN(rm->hardwareReg()), RN(sib->hardwareReg()), lsl, disp));
}

void JitX86CodeGen::regToFpuReg(FPURegPtr dst, RegPtr src) {
    x86.cvtsi2sd(getFPUReg(dst), R32(src->hardwareReg()));
}

#ifdef BOXEDWINE_64
void JitX86CodeGen::regToFpuReg64(FPURegPtr dst, RegPtr src) {
    x86.cvtsi2sd(getFPUReg(dst), R64(src->hardwareReg()));
}
#endif
void JitX86CodeGen::fpuAdd(FPURegPtr dst, FPURegPtr src) {
    x86.addsd(getFPUReg(dst), getFPUReg(src));
}

void JitX86CodeGen::fpuMul(FPURegPtr dst, FPURegPtr src) {
    x86.mulsd(getFPUReg(dst), getFPUReg(src));
}

void JitX86CodeGen::fpuSub(FPURegPtr dst, FPURegPtr src) {
    x86.subsd(getFPUReg(dst), getFPUReg(src));
}

void JitX86CodeGen::fpuDiv(FPURegPtr dst, FPURegPtr src) {
    x86.divsd(getFPUReg(dst), getFPUReg(src));
}

void JitX86CodeGen::fpuXor(FPURegPtr dst, FPURegPtr src) {
    x86.xorpd(getFPUReg(dst), getFPUReg(src));
}

void JitX86CodeGen::fpuAnd(FPURegPtr dst, FPURegPtr src) {
    x86.andpd(getFPUReg(dst), getFPUReg(src));
}

void JitX86CodeGen::fpuSqrt(FPURegPtr dst, FPURegPtr src) {
    x86.sqrtsd(getFPUReg(dst), getFPUReg(src));
}

void JitX86CodeGen::fcompare(FPURegPtr fpuReg1, FPURegPtr fpuReg2, RegPtr ordTags, const std::function<void()>& pfnEqual, const std::function<void()>& pfnLessThan, const std::function<void()>& pfnGreaterThan, const std::function<void()>& pfnInvalid) {
    subValue(JitWidth::b8, ordTags, TAG_Empty);
    IfNot(JitWidth::b8, ordTags);
        pfnInvalid();
    StartElse();
        x86.ucomisd(getFPUReg(fpuReg2), getFPUReg(fpuReg1));
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

void writeBlockExitForJIT(U32 eip, U8* buffer) {
    X86Asm x86;

    x86.mov(X86Asm::Mem32(HOST_CPU, offsetof(CPU, eip.u32)), eip);

    // writeCache
    for (int i = 0; i < 8; i++) {
        if (regCache[i] != INVALID_REG) {
            x86.mov(X86Asm::Mem32(HOST_CPU, CPU::offsetofReg32(i)), R32(regCache[i]));
        }
    }
    for (int i = 0; i < 8; i++) {
        if (xmmCache[i] != INVALID_REG) {
            x86.movaps(X86Asm::Mem128(HOST_CPU, i * 16 + offsetof(CPU, xmm)), X86Asm::XMM(xmmCache[i]));
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
}

void JitX86CodeGen::updateHardwareFlags(U32 flags) {
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

void JitX86CodeGen::dynamic_cmpxchg8b_lock(DecodedOp* op) {
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
        this->x86.cmpxchg(X86Asm::Mem32(RN(address->hardwareReg()), RN(offset->hardwareReg())), R32(reg->hardwareReg()));
        updateFlagsIfNecessary();
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
        this->x86.cmpxchg(X86Asm::Mem16(RN(address->hardwareReg()), RN(offset->hardwareReg())), R16(reg->hardwareReg()));
        updateFlagsIfNecessary();     
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
        this->x86.cmpxchg(X86Asm::Mem8(RN(address->hardwareReg()), RN(offset->hardwareReg())), R8(get8bitReg(reg)));
        updateFlagsIfNecessary();
    });
}

void JitX86CodeGen::dynamic_xchge32r32_lock(DecodedOp* op) {
    JitCodeGen::write(JitWidth::b32, calculateEaa(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        RegPtr reg = getReg(op->reg);
        this->x86.lock();
        this->x86.xchg(R32(reg->hardwareReg()), X86Asm::Mem32(RN(address->hardwareReg()), RN(offset->hardwareReg())));
    });
}

void JitX86CodeGen::dynamic_xchge16r16_lock(DecodedOp* op) {
    JitCodeGen::write(JitWidth::b16, calculateEaa(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        RegPtr reg = getReg(op->reg);
        this->x86.lock();
        this->x86.xchg(R16(reg->hardwareReg()), X86Asm::Mem16(RN(address->hardwareReg()), RN(offset->hardwareReg())));
    });
}

void JitX86CodeGen::dynamic_xchge8r8_lock(DecodedOp* op) {
    JitCodeGen::write(JitWidth::b8, calculateEaa(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        RegPtr reg = getReg8(op->reg);
        this->x86.lock();
        this->x86.xchg(R8(get8bitReg(reg)), X86Asm::Mem8(R32(address->hardwareReg()), R32(offset->hardwareReg())));        
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
    });
}

void JitX86CodeGen::dynamic_arithE32_lock(DecodedOp* op, std::function<void(RegPtr address, RegPtr offset)> callback) {
    JitCodeGen::write(JitWidth::b32, calculateEaa(op), nullptr, [op, callback, this](RegPtr address, RegPtr offset) {
        callback(address, offset);
        updateFlagsIfNecessary();
    });
}

void JitX86CodeGen::dynamic_arithE16_lock(DecodedOp* op, std::function<void(RegPtr address, RegPtr offset)> callback) {
    JitCodeGen::write(JitWidth::b16, calculateEaa(op), nullptr, [op, callback, this](RegPtr address, RegPtr offset) {
        callback(address, offset);
        updateFlagsIfNecessary();
    });
}
void JitX86CodeGen::dynamic_arithE8_lock(DecodedOp* op, std::function<void(RegPtr address, RegPtr offset)> callback) {
    JitCodeGen::write(JitWidth::b8, calculateEaa(op), nullptr, [op, callback, this](RegPtr address, RegPtr offset) {
        callback(address, offset);
        updateFlagsIfNecessary();
    });
}

void JitX86CodeGen::dynamic_xaddr32e32_lock(DecodedOp* op) {
    dynamic_arithE32R32_lock(op, [this](RegPtr dest, RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.xadd(X86Asm::Mem32(RN(addressReg->hardwareReg()), RN(offsetReg->hardwareReg())), R32(dest->hardwareReg()));
    }, true);
}

void JitX86CodeGen::dynamic_xaddr16e16_lock(DecodedOp* op) {
    dynamic_arithE16R16_lock(op, [this](RegPtr dest, RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.xadd(X86Asm::Mem16(RN(addressReg->hardwareReg()), RN(offsetReg->hardwareReg())), R16(dest->hardwareReg()));
    }, true);
}
void JitX86CodeGen::dynamic_xaddr8e8_lock(DecodedOp* op) {
    dynamic_arithE8R8_lock(op, [this](RegPtr dest, RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.xadd(X86Asm::Mem8(RN(addressReg->hardwareReg()), RN(offsetReg->hardwareReg())), R8(get8bitReg(dest)));
    }, true);
}

void JitX86CodeGen::dynamic_adde32r32_lock(DecodedOp* op) {
    dynamic_arithE32R32_lock(op, [this](RegPtr dest, RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.add(X86Asm::Mem32(RN(addressReg->hardwareReg()), RN(offsetReg->hardwareReg())), R32(dest->hardwareReg()));
    });
}
void JitX86CodeGen::dynamic_adde16r16_lock(DecodedOp* op) {
    dynamic_arithE16R16_lock(op, [this](RegPtr dest, RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.add(X86Asm::Mem16(RN(addressReg->hardwareReg()), RN(offsetReg->hardwareReg())), R16(dest->hardwareReg()));
    });
}
void JitX86CodeGen::dynamic_adde8r8_lock(DecodedOp* op) {
    dynamic_arithE8R8_lock(op, [this](RegPtr dest, RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.add(X86Asm::Mem8(RN(addressReg->hardwareReg()), RN(offsetReg->hardwareReg())), R8(get8bitReg(dest)));
    });
}
void JitX86CodeGen::dynamic_add32_mem_lock(DecodedOp* op) {
    dynamic_arithE32_lock(op, [op, this](RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.add(X86Asm::Mem32(RN(addressReg->hardwareReg()), RN(offsetReg->hardwareReg())), op->imm);
    });
}
void JitX86CodeGen::dynamic_add16_mem_lock(DecodedOp* op) {
    dynamic_arithE16_lock(op, [op, this](RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.add(X86Asm::Mem16(RN(addressReg->hardwareReg()), RN(offsetReg->hardwareReg())), (U16)op->imm);
    });
}
void JitX86CodeGen::dynamic_add8_mem_lock(DecodedOp* op) {
    dynamic_arithE8_lock(op, [op, this](RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.add(X86Asm::Mem8(RN(addressReg->hardwareReg()), RN(offsetReg->hardwareReg())), op->imm);
    });
}

void JitX86CodeGen::dynamic_sube32r32_lock(DecodedOp* op) {
    dynamic_arithE32R32_lock(op, [this](RegPtr dest, RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.sub(X86Asm::Mem32(RN(addressReg->hardwareReg()), RN(offsetReg->hardwareReg())), R32(dest->hardwareReg()));
    });
}
void JitX86CodeGen::dynamic_sube16r16_lock(DecodedOp* op) {
    dynamic_arithE16R16_lock(op, [this](RegPtr dest, RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.sub(X86Asm::Mem16(RN(addressReg->hardwareReg()), RN(offsetReg->hardwareReg())), R16(dest->hardwareReg()));
    });
}
void JitX86CodeGen::dynamic_sube8r8_lock(DecodedOp* op) {
    dynamic_arithE8R8_lock(op, [this](RegPtr dest, RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.sub(X86Asm::Mem8(RN(addressReg->hardwareReg()), RN(offsetReg->hardwareReg())), R8(get8bitReg(dest)));
    });
}
void JitX86CodeGen::dynamic_sub32_mem_lock(DecodedOp* op) {
    dynamic_arithE32_lock(op, [op, this](RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.sub(X86Asm::Mem32(RN(addressReg->hardwareReg()), RN(offsetReg->hardwareReg())), op->imm);
    });
}
void JitX86CodeGen::dynamic_sub16_mem_lock(DecodedOp* op) {
    dynamic_arithE16_lock(op, [op, this](RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.sub(X86Asm::Mem16(RN(addressReg->hardwareReg()), RN(offsetReg->hardwareReg())), (U16)op->imm);
    });
}
void JitX86CodeGen::dynamic_sub8_mem_lock(DecodedOp* op) {
    dynamic_arithE8_lock(op, [op, this](RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.sub(X86Asm::Mem8(RN(addressReg->hardwareReg()), RN(offsetReg->hardwareReg())), (U8)op->imm);
    });
}
void JitX86CodeGen::dynamic_ore32r32_lock(DecodedOp* op) {
    dynamic_arithE32R32_lock(op, [this](RegPtr dest, RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.or_(X86Asm::Mem32(RN(addressReg->hardwareReg()), RN(offsetReg->hardwareReg())), R32(dest->hardwareReg()));
    });
}
void JitX86CodeGen::dynamic_ore16r16_lock(DecodedOp* op) {
    dynamic_arithE16R16_lock(op, [this](RegPtr dest, RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.or_(X86Asm::Mem16(RN(addressReg->hardwareReg()), RN(offsetReg->hardwareReg())), R16(dest->hardwareReg()));
    });
}
void JitX86CodeGen::dynamic_ore8r8_lock(DecodedOp* op) {
    dynamic_arithE8R8_lock(op, [this](RegPtr dest, RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.or_(X86Asm::Mem8(RN(addressReg->hardwareReg()), RN(offsetReg->hardwareReg())), R8(get8bitReg(dest)));
    });
}
void JitX86CodeGen::dynamic_or32_mem_lock(DecodedOp* op) {
    dynamic_arithE32_lock(op, [op, this](RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.or_(X86Asm::Mem32(RN(addressReg->hardwareReg()), RN(offsetReg->hardwareReg())), op->imm);
    });
}
void JitX86CodeGen::dynamic_or16_mem_lock(DecodedOp* op) {
    dynamic_arithE16_lock(op, [op, this](RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.or_(X86Asm::Mem16(RN(addressReg->hardwareReg()), RN(offsetReg->hardwareReg())), (U16)op->imm);
    });
}
void JitX86CodeGen::dynamic_or8_mem_lock(DecodedOp* op) {
    dynamic_arithE8_lock(op, [op, this](RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.or_(X86Asm::Mem8(RN(addressReg->hardwareReg()), RN(offsetReg->hardwareReg())), (U8)op->imm);
    });
}
void JitX86CodeGen::dynamic_ande32r32_lock(DecodedOp* op) {
    dynamic_arithE32R32_lock(op, [this](RegPtr dest, RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.and_(X86Asm::Mem32(RN(addressReg->hardwareReg()), RN(offsetReg->hardwareReg())), R32(dest->hardwareReg()));
    });
}
void JitX86CodeGen::dynamic_ande16r16_lock(DecodedOp* op) {
    dynamic_arithE16R16_lock(op, [this](RegPtr dest, RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.and_(X86Asm::Mem16(RN(addressReg->hardwareReg()), RN(offsetReg->hardwareReg())), R16(dest->hardwareReg()));
    });
}
void JitX86CodeGen::dynamic_ande8r8_lock(DecodedOp* op) {
    dynamic_arithE8R8_lock(op, [this](RegPtr dest, RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.and_(X86Asm::Mem8(RN(addressReg->hardwareReg()), RN(offsetReg->hardwareReg())), R8(get8bitReg(dest)));
    });
}
void JitX86CodeGen::dynamic_and32_mem_lock(DecodedOp* op) {
    dynamic_arithE32_lock(op, [op, this](RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.and_(X86Asm::Mem32(RN(addressReg->hardwareReg()), RN(offsetReg->hardwareReg())), op->imm);
        });
}
void JitX86CodeGen::dynamic_and16_mem_lock(DecodedOp* op) {
    dynamic_arithE16_lock(op, [op, this](RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.and_(X86Asm::Mem16(RN(addressReg->hardwareReg()), RN(offsetReg->hardwareReg())), (U16)op->imm);
    });
}
void JitX86CodeGen::dynamic_and8_mem_lock(DecodedOp* op) {
    dynamic_arithE8_lock(op, [op, this](RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.and_(X86Asm::Mem8(RN(addressReg->hardwareReg()), RN(offsetReg->hardwareReg())), (U8)op->imm);
    });
}
void JitX86CodeGen::dynamic_xore32r32_lock(DecodedOp* op) {
    dynamic_arithE32R32_lock(op, [this](RegPtr dest, RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.xor_(X86Asm::Mem32(RN(addressReg->hardwareReg()), RN(offsetReg->hardwareReg())), R32(dest->hardwareReg()));
    });
}
void JitX86CodeGen::dynamic_xore16r16_lock(DecodedOp* op) {
    dynamic_arithE16R16_lock(op, [this](RegPtr dest, RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.xor_(X86Asm::Mem16(RN(addressReg->hardwareReg()), RN(offsetReg->hardwareReg())), R16(dest->hardwareReg()));
    });
}
void JitX86CodeGen::dynamic_xore8r8_lock(DecodedOp* op) {
    dynamic_arithE8R8_lock(op, [this](RegPtr dest, RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.xor_(X86Asm::Mem8(RN(addressReg->hardwareReg()), RN(offsetReg->hardwareReg())), R8(get8bitReg(dest)));
    });
}
void JitX86CodeGen::dynamic_xor32_mem_lock(DecodedOp* op) {
    dynamic_arithE32_lock(op, [op, this](RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.xor_(X86Asm::Mem32(RN(addressReg->hardwareReg()), RN(offsetReg->hardwareReg())), op->imm);
    });
}
void JitX86CodeGen::dynamic_xor16_mem_lock(DecodedOp* op) {
    dynamic_arithE16_lock(op, [op, this](RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.xor_(X86Asm::Mem16(RN(addressReg->hardwareReg()), RN(offsetReg->hardwareReg())), (U16)op->imm);
    });
}
void JitX86CodeGen::dynamic_xor8_mem_lock(DecodedOp* op) {
    dynamic_arithE8_lock(op, [op, this](RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.xor_(X86Asm::Mem8(RN(addressReg->hardwareReg()), RN(offsetReg->hardwareReg())), (U8)op->imm);
    });
}
void JitX86CodeGen::dynamic_inc32_mem32_lock(DecodedOp* op) {
    dynamic_arithE32_lock(op, [op, this](RegPtr addressReg, RegPtr offsetReg) {
        if (op->getNeededFlagsAfter(CF)) {
            updateHardwareFlags(CF);
        }
        this->x86.lock();
        this->x86.inc(X86Asm::Mem32(RN(addressReg->hardwareReg()), RN(offsetReg->hardwareReg())));
    });
}
void JitX86CodeGen::dynamic_inc16_mem16_lock(DecodedOp* op) {
    dynamic_arithE16_lock(op, [op, this](RegPtr addressReg, RegPtr offsetReg) {
        if (currentOp->getNeededFlagsAfter(CF)) {
            updateHardwareFlags(CF);
        }
        this->x86.lock();
        this->x86.inc(X86Asm::Mem16(RN(addressReg->hardwareReg()), RN(offsetReg->hardwareReg())));
    });
}
void JitX86CodeGen::dynamic_inc8_mem8_lock(DecodedOp* op) {
    dynamic_arithE8_lock(op, [op, this](RegPtr addressReg, RegPtr offsetReg) {
        if (currentOp->getNeededFlagsAfter(CF)) {
            updateHardwareFlags(CF);
        }
        this->x86.lock();
        this->x86.inc(X86Asm::Mem8(RN(addressReg->hardwareReg()), RN(offsetReg->hardwareReg())));
    });
}
void JitX86CodeGen::dynamic_dec32_mem32_lock(DecodedOp* op) {
    dynamic_arithE32_lock(op, [op, this](RegPtr addressReg, RegPtr offsetReg) {
        if (currentOp->getNeededFlagsAfter(CF)) {
            updateHardwareFlags(CF);
        }
        this->x86.lock();
        this->x86.dec(X86Asm::Mem32(RN(addressReg->hardwareReg()), RN(offsetReg->hardwareReg())));
    });
}
void JitX86CodeGen::dynamic_dec16_mem16_lock(DecodedOp* op) {
    dynamic_arithE16_lock(op, [op, this](RegPtr addressReg, RegPtr offsetReg) {
        if (currentOp->getNeededFlagsAfter(CF)) {
            updateHardwareFlags(CF);
        }
        this->x86.lock();
        this->x86.dec(X86Asm::Mem16(RN(addressReg->hardwareReg()), RN(offsetReg->hardwareReg())));
    });
}
void JitX86CodeGen::dynamic_dec8_mem8_lock(DecodedOp* op) {
    dynamic_arithE8_lock(op, [op, this](RegPtr addressReg, RegPtr offsetReg) {
        if (currentOp->getNeededFlagsAfter(CF)) {
            updateHardwareFlags(CF);
        }
        this->x86.lock();
        this->x86.dec(X86Asm::Mem8(RN(addressReg->hardwareReg()), RN(offsetReg->hardwareReg())));
    });
}

void JitX86CodeGen::dynamic_note32_lock(DecodedOp* op) {
    dynamic_arithE32_lock(op, [op, this](RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.not_(X86Asm::Mem32(RN(addressReg->hardwareReg()), RN(offsetReg->hardwareReg())));
    });
}

void JitX86CodeGen::dynamic_note16_lock(DecodedOp* op) {
    dynamic_arithE16_lock(op, [op, this](RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.not_(X86Asm::Mem16(RN(addressReg->hardwareReg()), RN(offsetReg->hardwareReg())));
    });
}

void JitX86CodeGen::dynamic_note8_lock(DecodedOp* op) {
    dynamic_arithE8_lock(op, [op, this](RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.not_(X86Asm::Mem8(RN(addressReg->hardwareReg()), RN(offsetReg->hardwareReg())));
    });
}

void JitX86CodeGen::dynamic_nege32_lock(DecodedOp* op) {
    dynamic_arithE32_lock(op, [op, this](RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.neg(X86Asm::Mem32(RN(addressReg->hardwareReg()), RN(offsetReg->hardwareReg())));
    });
}

void JitX86CodeGen::dynamic_nege16_lock(DecodedOp* op) {
    dynamic_arithE16_lock(op, [op, this](RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.neg(X86Asm::Mem16(RN(addressReg->hardwareReg()), RN(offsetReg->hardwareReg())));
    });
}

void JitX86CodeGen::dynamic_nege8_lock(DecodedOp* op) {
    dynamic_arithE8_lock(op, [op, this](RegPtr addressReg, RegPtr offsetReg) {
        this->x86.lock();
        this->x86.neg(X86Asm::Mem8(RN(addressReg->hardwareReg()), RN(offsetReg->hardwareReg())));
    });
}

void JitX86CodeGen::dynamic_btse32_lock(DecodedOp* op) {
    JitCodeGen::write(JitWidth::b32, calculateEaa(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        // imm is the mask, not the bitshift
        U8 imm = std::countr_zero(op->imm);
        this->x86.lock();
        this->x86.bts(X86Asm::Mem32(RN(address->hardwareReg()), RN(offset->hardwareReg())), (U8)imm);
        updateFlagsIfNecessary();
    });
}

void JitX86CodeGen::dynamic_btse16_lock(DecodedOp* op) {
    JitCodeGen::write(JitWidth::b16, calculateEaa(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        // imm is the mask, not the bitshift
        U8 imm = std::countr_zero(op->imm);
        this->x86.lock();
        this->x86.bts(X86Asm::Mem16(RN(address->hardwareReg()), RN(offset->hardwareReg())), (U8)imm);
        updateFlagsIfNecessary();
    });
}

void JitX86CodeGen::dynamic_btse32r32_lock(DecodedOp* op) {
    JitCodeGen::write(JitWidth::b32, calculateEffectiveEaa32(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        RegPtr reg = getTmpReg(op->reg);
        andValue(JitWidth::b32, reg, 0x1f);
        this->x86.lock();
        this->x86.bts(X86Asm::Mem32(RN(address->hardwareReg()), RN(offset->hardwareReg())), R32(reg->hardwareReg()));
        updateFlagsIfNecessary();
    });
}

void JitX86CodeGen::dynamic_btse16r16_lock(DecodedOp* op) {
    JitCodeGen::write(JitWidth::b16, calculateEffectiveEaa16(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        RegPtr reg = getTmpReg(op->reg);
        andValue(JitWidth::b16, reg, 0xf);
        this->x86.lock();
        this->x86.bts(X86Asm::Mem16(RN(address->hardwareReg()), RN(offset->hardwareReg())), R16(reg->hardwareReg()));
        updateFlagsIfNecessary();
    });
}

void JitX86CodeGen::dynamic_btre32_lock(DecodedOp* op) {
    JitCodeGen::write(JitWidth::b32, calculateEaa(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        // imm is the mask, not the bitshift
        U8 imm = std::countr_zero(op->imm);
        this->x86.lock();
        this->x86.btr(X86Asm::Mem32(RN(address->hardwareReg()), RN(offset->hardwareReg())), (U8)imm);
        updateFlagsIfNecessary();
    });
}
void JitX86CodeGen::dynamic_btre16_lock(DecodedOp* op) {
    JitCodeGen::write(JitWidth::b16, calculateEaa(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        // imm is the mask, not the bitshift
        U8 imm = std::countr_zero(op->imm);
        this->x86.lock();
        this->x86.btr(X86Asm::Mem16(RN(address->hardwareReg()), RN(offset->hardwareReg())), (U8)imm);
        updateFlagsIfNecessary();
    });
}
void JitX86CodeGen::dynamic_btre32r32_lock(DecodedOp* op) {
    JitCodeGen::write(JitWidth::b32, calculateEffectiveEaa32(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        RegPtr reg = getTmpReg(op->reg);
        andValue(JitWidth::b32, reg, 0x1f);
        this->x86.lock();
        this->x86.btr(X86Asm::Mem32(RN(address->hardwareReg()), RN(offset->hardwareReg())), R32(reg->hardwareReg()));
        updateFlagsIfNecessary();
    });
}
void JitX86CodeGen::dynamic_btre16r16_lock(DecodedOp* op) {
    JitCodeGen::write(JitWidth::b16, calculateEffectiveEaa16(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        RegPtr reg = getTmpReg(op->reg);
        andValue(JitWidth::b16, reg, 0xf);
        this->x86.lock();
        this->x86.btr(X86Asm::Mem16(RN(address->hardwareReg()), RN(offset->hardwareReg())), R16(reg->hardwareReg()));
        updateFlagsIfNecessary();
    });
}

void JitX86CodeGen::dynamic_btce32_lock(DecodedOp* op) {
    JitCodeGen::write(JitWidth::b32, calculateEaa(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        // imm is the mask, not the bitshift
        U8 imm = std::countr_zero(op->imm);
        this->x86.lock();
        this->x86.btc(X86Asm::Mem32(RN(address->hardwareReg()), RN(offset->hardwareReg())), (U8)imm);
        updateFlagsIfNecessary();
    });
}
void JitX86CodeGen::dynamic_btce16_lock(DecodedOp* op) {
    JitCodeGen::write(JitWidth::b16, calculateEaa(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        // imm is the mask, not the bitshift
        U8 imm = std::countr_zero(op->imm);
        this->x86.lock();
        this->x86.btc(X86Asm::Mem16(RN(address->hardwareReg()), RN(offset->hardwareReg())), (U8)imm);
        updateFlagsIfNecessary();
    });
}
void JitX86CodeGen::dynamic_btce32r32_lock(DecodedOp* op) {
    JitCodeGen::write(JitWidth::b32, calculateEffectiveEaa32(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        RegPtr reg = getTmpReg(op->reg);
        andValue(JitWidth::b32, reg, 0x1f);
        this->x86.lock();
        this->x86.btc(X86Asm::Mem32(RN(address->hardwareReg()), RN(offset->hardwareReg())), R32(reg->hardwareReg()));
        updateFlagsIfNecessary();
    });
}
void JitX86CodeGen::dynamic_btce16r16_lock(DecodedOp* op) {
    JitCodeGen::write(JitWidth::b16, calculateEffectiveEaa16(op), nullptr, [op, this](RegPtr address, RegPtr offset) {
        RegPtr reg = getTmpReg(op->reg);
        andValue(JitWidth::b16, reg, 0xf);
        this->x86.lock();
        this->x86.btc(X86Asm::Mem16(RN(address->hardwareReg()), RN(offset->hardwareReg())), R16(reg->hardwareReg()));
        updateFlagsIfNecessary();
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

    x86.mov(HOST_MMU, (U64)getMemData(KThread::currentThread()->memory)->mmu);
    x86.mov(HOST_RAM, (U64)ramPages);

    // on win32 ecx contains cpu
    x86.mov(HOST_CPU, x86.rcx);

    // before rdx is clobbered
    x86.mov(RN(tmps[0]), X86Asm::Mem64(x86.rdx, offsetof(DecodedOp, pfnJitCode)));

    loadCache();

    // jmp ((DecodedOp*)rdx)->pfn    
    x86.jmp(RN(tmps[0]));

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
    x86.mov(RN(tmps[0]), X86Asm::Mem32(x86.edx, offsetof(DecodedOp, pfnJitCode)));
    x86.jmp(RN(tmps[0]));
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
    for (int i = 0; i < 8; i++) {
        if (xmmCache[i] != INVALID_REG) {
            x86.movaps(X86Asm::XMM(xmmCache[i]), X86Asm::Mem128(HOST_CPU, i * 16 + offsetof(CPU, xmm)));
        }
    }
}

void JitX86CodeGen::writeCache() {
    for (int i = 0; i < 8; i++) {
        if (regCache[i] != INVALID_REG) {
            x86.mov(X86Asm::Mem32(HOST_CPU, offsetof(CPU, reg[i].u32)), R32(regCache[i]));
        }
    }    
    for (int i = 0; i < 8; i++) {
        if (xmmCache[i] != INVALID_REG) {
            x86.movaps(X86Asm::Mem128(HOST_CPU, i * 16 + offsetof(CPU, xmm)), X86Asm::XMM(xmmCache[i]));
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