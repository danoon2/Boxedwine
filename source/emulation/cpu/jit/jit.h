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

#ifndef __JIT_H__
#define __JIT_H__

#include "../common/cpu.h"

enum class JitWidth {
    b8,
    b16,
    b32,
    b64,
    b128,
    b256,
};

enum class JitEvaluate {
    EQUALS,
    NOT_EQUALS,
    LESS_THAN_UNSIGNED,
    LESS_THAN_EQUAL_UNSIGNED,
    GREATER_THAN_UNSIGNED,
    GREATER_THAN_EQUAL_UNSIGNED,
    LESS_THAN_SIGNED,
    LESS_THAN_EQUAL_SIGNED,
    GREATER_THAN_EQUAL_SIGNED,
    GREATER_THAN_SIGNED,
};

enum class JitCallParamType {
    REG_8,
    REG_16,
    REG_32,
    CONST_8,
    CONST_16,
    CONST_32,
    CONST_PTR,
    CPU,
};

enum class JitConditional {
    O,
    NO,
    B,
    NB,
    Z,
    NZ,
    BE,
    NBE,
    S,
    NS,
    P,
    NP,
    L,
    NL,
    LE,
    NLE
};

// the code will guarantee that for a single instruction, 2 DynReg's will never point to the same reg and both be read/write (see dynamic_xchgr8r8), that way we don't have to worry about clobbering
class JitReg {
public:
    JitReg(U8 hardwareReg, U8 emulatedReg, std::function<U8()> delayedLoading = nullptr) : emulatedReg(emulatedReg), isHigh(false), reg(hardwareReg), delayedLoading(delayedLoading) {}
    JitReg(U8 hardwareReg, U8 emulatedReg, bool isHigh, std::function<U8()> delayedLoading = nullptr) : emulatedReg(emulatedReg), isHigh(isHigh), reg(hardwareReg), delayedLoading(delayedLoading) {}

    U8 hardwareReg();
    bool isLoaded() { return reg != 0xff; }
    U8 emulatedReg;
    bool isHigh;

private:
    U8 reg;
    std::function<U8()> delayedLoading;
};

using RegPtr = std::shared_ptr<JitReg>;

#ifdef BOXEDWINE_64
#define DYN_PTR_SIZE U64
#define DYN_PTR JitWidth::b64
#define DYN_PTR_LSL 3
#else
#define DYN_PTR_SIZE U32
#define DYN_PTR JitWidth::b32
#define DYN_PTR_LSL 2
#endif

class JitMem {
public:
    JitMem(U32 address) : offset(address) {}
    JitMem(RegPtr rm, U32 offset = 0, bool emulatedAddress = true) : rm(rm), offset(offset), emulatedAddress(emulatedAddress) {}
    JitMem(RegPtr rm, RegPtr sib, U32 lsl = 0, U32 offset = 0, bool emulatedAddress = true) : rm(rm), sib(sib), lsl(lsl), offset(offset), emulatedAddress(emulatedAddress) {}
	JitMem(const JitMem& mem) : rm(mem.rm), sib(mem.sib), lsl(mem.lsl), offset(mem.offset), emulatedAddress(mem.emulatedAddress) {}

    std::shared_ptr<JitMem> copy() {
        return std::make_shared<JitMem>(*this);
	}

    RegPtr rm;
    RegPtr sib;
    U32 lsl = 0;
    U32 offset = 0;
    bool emulatedAddress = true;
};

using MemPtr = std::shared_ptr<JitMem>;
#define createMemPtr std::make_shared<JitMem>

// API available to dynamic ops
class Jit {
public:
    // v3
    using InstRegRegImm = void(Jit::*)(JitWidth regWidth, RegPtr reg, RegPtr rm, U32 imm);
    using InstRegRegCl = void(Jit::*)(JitWidth regWidth, RegPtr reg, RegPtr rm, RegPtr cl);
    using InstRegReg = void(Jit::*)(JitWidth regWidth, RegPtr reg, RegPtr rm);
    using InstRegRegCF = void(Jit::*)(JitWidth regWidth, RegPtr reg, RegPtr rm, RegPtr cf);
    using InstRegImm = void(Jit::*)(JitWidth regWidth, RegPtr reg, U32 imm);
    using InstRegImmCF = void(Jit::*)(JitWidth regWidth, RegPtr reg, U32 imm, RegPtr cf);
    using InstReg = void(Jit::*)(JitWidth regWidth, RegPtr reg);

    virtual RegPtr getStringRegEcx() = 0;
    virtual RegPtr getStringRegEsi() = 0;
    virtual RegPtr getStringRegEdi() = 0;
    virtual RegPtr getReg(U8 reg, S8 hint = -1, bool load = true) = 0; // for cached regs, they will be used directly, for unchached regs, it will be read from the cpu and written back when done
    virtual RegPtr getReg8(U8 reg, bool load = true) = 0;
    virtual RegPtr getReadOnlyReg(U8 reg, bool delayed = false, S8 hint = -1) = 0; // for cached regs, they will be used directly, for unchached regs, it will be read from the cpu but NOT written back when done
    virtual RegPtr getReadOnlyReg8(U8 reg, bool delayed = false, S8 hint = -1) = 0;
    virtual RegPtr getTmpReg() = 0;
    virtual RegPtr getTmpReg8() = 0; // on x86 JIT, only EAX,ECX,EDX and EBX are capable of 8-bit operations, so this call allows asking for a tmp reg that is capable of 8-bit
    virtual RegPtr getTmpRegWithHint(S8 hint) = 0;
    virtual RegPtr getTmpRegForCallResult() = 0; // just a hint to try and get the same reg used for a call result in order to prevent an extra mov
    virtual RegPtr getTmpReg(U8 reg, bool delayed = false, S8 hint = -1) = 0; // a reg that doesn't represent an emulated reg, but come pre-loaded with the emulated reg's current value
    virtual RegPtr getTmpReg8(U8 reg, bool delayed = false, S8 hint = -1) = 0;
    virtual RegPtr getReadOnlySegAddress(U8 reg) = 0;
    virtual RegPtr getTmpSegAddress(U8 reg) = 0;
    virtual RegPtr getReadOnlySegValue(U8 reg) = 0;
    virtual RegPtr readEip() = 0;
    virtual void writeEip(RegPtr eip) = 0;
    virtual void writeEip(U32 eip) = 0;
    virtual void writeCurrentEip(U32 addAmount) {
        writeEip(currentEip - cpu->seg[CS].address + addAmount);
    }
    virtual bool isTmpRegAvailable() = 0;
    virtual RegPtr calculateEaa(DecodedOp* op, U32 popEspAmount = 0);
    virtual void jmpHost(RegPtr reg) = 0;
    virtual void jmpHost(DYN_PTR_SIZE address) = 0;
    virtual void forceSyncBackIfNotCached(RegPtr reg) = 0;

    RegPtr calculateEffectiveEaa16(DecodedOp* op);
    RegPtr calculateEffectiveEaa32(DecodedOp* op);
    RegPtr calculateEffectiveEaa(DecodedOp* op);

    RegPtr calculateMask16(DecodedOp* op);
    RegPtr calculateMask32(DecodedOp* op);

    virtual void addReg(JitWidth regWidth, RegPtr reg, RegPtr rm) = 0;    
    virtual void addRegWithDest(JitWidth regWidth, RegPtr dst, RegPtr reg, RegPtr rm) = 0;
    virtual void addValue(JitWidth regWidth, RegPtr reg, U32 imm) = 0;
    virtual void addValueWithDest(JitWidth regWidth, RegPtr dst, RegPtr reg, U32 imm) = 0;
    virtual void orReg(JitWidth regWidth, RegPtr reg, RegPtr rm) = 0;
    virtual void orValue(JitWidth regWidth, RegPtr reg, U32 imm) = 0;
    virtual void subReg(JitWidth regWidth, RegPtr reg, RegPtr rm) = 0;
    virtual void subValue(JitWidth regWidth, RegPtr reg, U32 imm) = 0;
    virtual void subValueWithDest(JitWidth regWidth, RegPtr dst, RegPtr reg, U32 imm) = 0;
    virtual void andReg(JitWidth regWidth, RegPtr reg, RegPtr rm) = 0;
    virtual void andValue(JitWidth regWidth, RegPtr reg, U32 immm) = 0;
    virtual void andValueWithDest(JitWidth regWidth, RegPtr dst, RegPtr reg, U32 value) = 0;
#ifdef BOXEDWINE_64
    virtual void andValue64(RegPtr reg, U64 immm) = 0;
#define andValueNative(reg, imm) andValue64(reg, imm);
#else
#define andValueNative(reg, imm) andValue(JitWidth::b32, reg, imm);
#endif
    virtual void xorReg(JitWidth regWidth, RegPtr reg, RegPtr rm) = 0;
    virtual void xorValue(JitWidth regWidth, RegPtr reg, U32 immm) = 0;
    virtual void shrReg(JitWidth regWidth, RegPtr reg, RegPtr rm) = 0;
    virtual void shrValue(JitWidth regWidth, RegPtr reg, U32 immm) = 0;
    virtual void shrValueWithDest(JitWidth regWidth, RegPtr dst, RegPtr reg, U32 value) = 0;
    virtual void shlReg(JitWidth regWidth, RegPtr reg, RegPtr rm) = 0;
    virtual void shlValue(JitWidth regWidth, RegPtr reg, U32 immm) = 0;
    virtual void shlValueWithDest(JitWidth regWidth, RegPtr dst, RegPtr reg, U32 value) = 0;
    virtual void sarReg(JitWidth regWidth, RegPtr reg, RegPtr rm) = 0;
    virtual void sarValue(JitWidth regWidth, RegPtr reg, U32 immm) = 0;
    virtual void sarValueWithDest(JitWidth regWidth, RegPtr dst, RegPtr reg, U32 immm) = 0;
    virtual void notReg2(JitWidth regWidth, RegPtr reg) = 0;
    virtual void negReg2(JitWidth regWidth, RegPtr reg) = 0;
    virtual void bsfReg(JitWidth regWidth, RegPtr reg, RegPtr rm) = 0;
    virtual void bsrReg(JitWidth regWidth, RegPtr reg, RegPtr rm) = 0;
    virtual void rolReg(JitWidth regWidth, RegPtr reg, RegPtr rm) = 0;
    virtual void rolValue(JitWidth regWidth, RegPtr reg, U32 imm) = 0;
    virtual void rorReg(JitWidth regWidth, RegPtr reg, RegPtr rm) = 0;
    virtual void rorValue(JitWidth regWidth, RegPtr reg, U32 imm) = 0;
    virtual void rclReg(JitWidth regWidth, RegPtr reg, RegPtr rm, RegPtr cf) = 0;
    virtual void rclValue(JitWidth regWidth, RegPtr reg, U32 imm, RegPtr cf) = 0;
    virtual void rcrReg(JitWidth regWidth, RegPtr reg, RegPtr rm, RegPtr cf) = 0;
    virtual void rcrValue(JitWidth regWidth, RegPtr reg, U32 imm, RegPtr cf) = 0;
    virtual void shrdReg(JitWidth regWidth, RegPtr reg, RegPtr rm, RegPtr cl) = 0;
    virtual void shrdValue(JitWidth regWidth, RegPtr reg, RegPtr rm, U32 imm) = 0;
    virtual void shldReg(JitWidth regWidth, RegPtr reg, RegPtr rm, RegPtr cl) = 0;
    virtual void shldValue(JitWidth regWidth, RegPtr reg, RegPtr rm, U32 imm) = 0;
    virtual void incReg(JitWidth regWidth, RegPtr dest) = 0;
    virtual void decReg(JitWidth regWidth, RegPtr dest) = 0;
    virtual void xchgReg(JitWidth regWidth, RegPtr dest, RegPtr src) = 0;
    virtual void xaddReg(JitWidth regWidth, RegPtr reg, RegPtr rm) = 0;
    virtual void mulReg(JitWidth regWidth, RegPtr reg) = 0;
    virtual void imulReg(JitWidth regWidth, RegPtr reg) = 0;
    virtual void imulRRI(JitWidth regWidth, RegPtr dst, RegPtr src, U32 src2, RegPtr overflow = nullptr) = 0;
    virtual void imulRR(JitWidth regWidth, RegPtr dst, RegPtr src, RegPtr overflow = nullptr) = 0;
    virtual void divRegRegWithRemainder(JitWidth regWidth, RegPtr dest, RegPtr destHighAndRemainder, RegPtr src) = 0; // src should be checked for 0 before calling
    virtual void idivRegRegWithRemainder(JitWidth regWidth, RegPtr dest, RegPtr destHighAndRemainder, RegPtr src) = 0; // src should be checked for 0 before calling
    virtual void byteSwapReg32(RegPtr reg) = 0;
    virtual RegPtr compareReg(JitWidth regWidth, RegPtr reg1, RegPtr reg2, JitEvaluate condition, RegPtr resultReg = nullptr) = 0; // will return 0 or 1 in result    
    virtual RegPtr compareValue(JitWidth regWidth, RegPtr reg, U32 value, JitEvaluate condition, RegPtr resultReg = nullptr) = 0; // will return 0 or 1 in result
    virtual RegPtr testZeroReg(JitWidth regWidth, RegPtr reg, RegPtr result = nullptr) = 0; // will return 0 or 1 in result
    virtual void absReg(JitWidth regWidth, RegPtr reg) = 0;
    virtual void clzReg(JitWidth regWidth, RegPtr result, RegPtr reg) = 0;

    virtual void mov(JitWidth regWidth, RegPtr dest, RegPtr src) = 0;
    virtual void movzx(JitWidth dstWidth, RegPtr dest, JitWidth srcWidth, RegPtr src) = 0;
    virtual void movsx(JitWidth dstWidth, RegPtr dest, JitWidth srcWidth, RegPtr src) = 0;
    virtual void movValue(JitWidth regWidth, RegPtr dst, DYN_PTR_SIZE imm) = 0;

    void push16(RegPtr reg);
    void push32(RegPtr reg);
    RegPtr peek16(RegPtr resultReg = nullptr);
    RegPtr peek32(RegPtr resultReg = nullptr);
    RegPtr pop16(RegPtr resultReg = nullptr, U32 amount = 2);
    RegPtr pop32(RegPtr resultReg = nullptr, U32 amount = 4);

    virtual void storeLazyFlagType(LazyFlagType flags) = 0;
    virtual void storeLazyFlagsDest(RegPtr reg) = 0;
    virtual void storeLazyFlagsSrc(RegPtr reg) = 0;
    virtual void storeLazyFlagsSrc(U32 value) = 0;
    virtual void storeLazyFlagsResult(RegPtr reg) = 0;
    virtual void storeLazyFlagsOldCF(RegPtr reg) = 0;
    virtual void fillFlags(U32 flags = PF | SF | AF | CF | OF | ZF) = 0;
    virtual RegPtr getZF() = 0;
    virtual RegPtr getCF() = 0;
    virtual void orCPUFlags(RegPtr reg) = 0;
    virtual void xorCPUFlagsImmV2(U32 imm) = 0;
    virtual void andCPUFlagsImmV2(U32 imm) = 0;
    virtual void orCPUFlagsImmV2(U32 imm) = 0;    
    virtual RegPtr getReadOnlyFlags(RegPtr tmp = nullptr) = 0; // not guaranteed to return tmp
    virtual RegPtr getFlagsInTmp(RegPtr reg = nullptr) = 0;
    virtual void setFlags(RegPtr flags, U32 mask) = 0;
    virtual void writeFlags(RegPtr flags) = 0;
    virtual RegPtr getCondition(JitConditional condition, RegPtr resultReg = nullptr) = 0; // guaranteed to return 0 or 1

    virtual void If(JitWidth regWidth, RegPtr reg) = 0;
    virtual void IfTest(JitWidth regWidth, RegPtr reg, RegPtr mask) = 0;
    virtual void IfTest(JitWidth regWidth, RegPtr reg, U32 mask) = 0;
    virtual void IfNotTestBit(JitWidth regWidth, RegPtr reg, U32 bitPos) = 0;
    virtual void IfTestBit(JitWidth regWidth, RegPtr reg, U32 bitPos) = 0;
    virtual void IfEqual(JitWidth regWidth, RegPtr reg, DYN_PTR_SIZE value) = 0;
    virtual void IfEqual(JitWidth regWidth, RegPtr reg1, RegPtr reg2) = 0;
    virtual void IfNotEqual(JitWidth regWidth, RegPtr reg, DYN_PTR_SIZE value) = 0;
    virtual void IfNotEqual(JitWidth regWidth, RegPtr reg, RegPtr reg2) = 0;
    virtual void IfLessThan2(JitWidth regWidth, RegPtr reg, U32 value) = 0;
    virtual void IfLessThan2(JitWidth regWidth, RegPtr reg1, RegPtr reg2) = 0;
    virtual void IfGreaterThanOrEqual(JitWidth regWidth, RegPtr reg1, RegPtr reg2) = 0;
    virtual void IfGreaterThanOrEqual(JitWidth regWidth, RegPtr reg, U32 value) = 0;
    virtual void IfNot(JitWidth regWidth, RegPtr reg) = 0;
    virtual void IfNotCPU(JitWidth regWidth, RegPtr sib, U8 lsl, U32 offset) = 0;
    virtual void IfCondition(JitConditional condition) = 0;
    virtual void JumpIfCondition(JitConditional condition, U32 address) = 0;
    virtual U32 MarkJumpLocation() = 0;
    virtual void Goto(U32 location) = 0;
    virtual void IfDF() = 0;
    virtual void IfSmallStack() = 0;

    // these will check that the memory is valid and doesn't span a page then allow the caller to override what happens when everything is good by providing a callback, 
    // the callback will contain the host memory address
    virtual RegPtr readWriteMem(JitWidth width, RegPtr addressReg, std::function<void(RegPtr value)> prepareWrite, S8 hint = -1) = 0;
    virtual RegPtr read(JitWidth width, RegPtr addressReg, std::function<void(MemPtr address)> customMemoryOp = nullptr, std::function<void()> failedMemoryOp = nullptr, RegPtr tmp = nullptr, bool checkAlignment = true) = 0;
    virtual void write(JitWidth width, RegPtr addressReg, RegPtr src, std::function<void(MemPtr address)> customMemoryOp = nullptr, std::function<void()> failedMemoryOp = nullptr, bool checkAlignment = true) = 0;

    virtual RegPtr read(JitWidth width, MemPtr address, RegPtr result = nullptr) = 0;
    virtual void write(JitWidth width, MemPtr address, RegPtr src) = 0;
    virtual void write(JitWidth width, MemPtr address, U32 imm) = 0;

    void dynamic_RR(DecodedOp* op, JitWidth width, InstRegReg callback, LazyFlagType flags, bool writeback = true, bool addCF = false);
    void dynamic_RR_WriteBoth(DecodedOp* op, JitWidth width, InstRegReg callback, LazyFlagType flags);
    void dynamic_R_Cl(DecodedOp* op, JitWidth width, InstRegReg callback, LazyFlagType flags, InstRegRegCF callbackWithCF = nullptr);
    void dynamic_MR(DecodedOp* op, JitWidth width, InstRegReg callback, LazyFlagType flags, bool writeback = true, bool addCF = false);
    void dynamic_RM_WriteM(DecodedOp* op, JitWidth width, InstRegReg callback, LazyFlagType flags);
    void dynamic_M_Cl(DecodedOp* op, JitWidth width, InstRegReg callback, LazyFlagType flags, InstRegRegCF callbackWithCF = nullptr);
    void dynamic_RM(DecodedOp* op, JitWidth width, InstRegReg callback, LazyFlagType flags, bool writeback = true, bool addCF = false);
    void dynamic_RI(DecodedOp* op, JitWidth width, InstRegImm callback, LazyFlagType flags, bool writeback = true, bool addCF = false, InstRegReg cfCallback = nullptr, InstRegImmCF callbackWithCF = nullptr);
    void dynamic_MI(DecodedOp* op, JitWidth width, InstRegImm callback, LazyFlagType flags, bool writeback = true, bool addCF = false, InstRegReg cfCallback = nullptr, InstRegImmCF callbackWithCF = nullptr);
    void dynamic_R(DecodedOp* op, JitWidth width, InstReg callback, LazyFlagType flags, bool writeback = true);
    void dynamic_M(DecodedOp* op, JitWidth width, InstReg callback, LazyFlagType flags, bool writeback = true, RegPtr tmp = nullptr);

    using OpFunction = void(Jit::*)(DecodedOp* op);

    Jit(CPU* cpu) : cpu(cpu) {}
    CPU* cpu = nullptr;

    // don't perform logic based on currentLazyFlags, you must check at run time if the lazy flags is currently equal to currentLazyFlags then you can use it.  This is because
    // another instruction could have jumped to the instruction using currentLazyFlags, so it may not be the same lazy flags as the previous instruction that set it.
    LazyFlagType currentLazyFlags = FLAGS_NULL; 
    U32 currentEip = 0;
    DecodedOp* currentOp = nullptr;
    DecodedOp* skipToOp = nullptr;

    virtual bool canJumpInBlock(DecodedOp* op) = 0;
    virtual bool canJumpInBlock(U32 opEip, DecodedOp* op) = 0;

    virtual void blockNext1(U32 eip, DecodedOp* op) = 0;
    virtual void blockNext2(U32 eip, DecodedOp* op) = 0;
    virtual void blockExit() = 0;
    virtual void jumpEip(RegPtr reg) = 0;

    virtual void JumpInBlock(U32 address) = 0;
    virtual void StartElse() = 0;
    virtual void EndIf() = 0;

    void dynamic_sidt(DecodedOp* op);
    void dynamic_callback(DecodedOp* op);
    void dynamic_invalid_op(DecodedOp* op);    
    void dynamic_onTestEnd(DecodedOp* op);
    virtual void onTestEnd(DecodedOp* op) = 0;

    class DynParam {
    public:
        DynParam(JitCallParamType type) : type(type), value(0) {}
        DynParam(JitCallParamType type, U32 value) : type(type), value(value) {}
        DynParam(JitCallParamType type, RegPtr reg) : type(type), value(0), reg(reg) {}

        bool usesReg() const {
            return type == JitCallParamType::REG_8 || type == JitCallParamType::REG_16 || type == JitCallParamType::REG_32;
        }
        JitCallParamType type;
        DYN_PTR_SIZE value;
        const RegPtr reg;
    };
    virtual void callHostFunction(void* address, const std::vector<DynParam>& params, bool restoreCache = true, bool saveCache = true) = 0;
    virtual void callHostFunctionWithResult(RegPtr result, void* address, const std::vector<DynParam>& params) = 0;

    virtual void emulateSingleOp() = 0;

    using CallReturn = U32(*)(CPU* cpu);
    RegPtr callAndReturn(CallReturn address, RegPtr resultReg = nullptr);    

    using CallReturnOp = DecodedOp*(*)(CPU* cpu);
    RegPtr callAndReturnOp(CallReturnOp address, RegPtr resultReg = nullptr);

    using CallReturnPtr = DYN_PTR_SIZE(*)(CPU* cpu);
    RegPtr callAndReturnPtr(CallReturnPtr address, RegPtr resultReg = nullptr);

    using CallNoArgs = void(*)(CPU* cpu);
    void call(CallNoArgs address);

    using CallI = void(*)(CPU* cpu, U32 value);
    void call_I(CallI address, U32 value);

    using CallRI = void(*)(CPU* cpu, U32 value1, U32 value2);
    void call_RI(CallRI address, JitWidth width, RegPtr reg, U32 value);

    #define MAX_NUMBER_OF_CALL_PARAMS 2

#define INIT_CPU(e, f) virtual void dynamic_##f(DecodedOp* op);
#include "../common/cpu_init.h"
#include "../common/cpu_init_mmx.h"
#include "../common/cpu_init_sse.h"
#include "../common/cpu_init_sse2.h"
#include "../common/cpu_init_fpu.h"
#ifdef BOXEDWINE_MULTI_THREADED
#define INIT_CPU_LOCK(e, f) virtual void dynamic_##f##_lock(DecodedOp* op);
#include "../common/cpu_init_lock.h"
#undef INIT_CPU_LOCK
#endif
#undef INIT_CPU

    virtual void direct_cmp(JitWidth width, RegPtr left, RegPtr right) = 0;
    virtual void direct_cmp(JitWidth width, RegPtr left, U32 right) = 0;
    virtual void direct_test(JitWidth width, RegPtr left, RegPtr right) = 0;
    virtual void direct_test(JitWidth width, RegPtr left, U32 right) = 0;
    virtual void direct_jump(JitConditional condition, U32 address) = 0;
    virtual void direct_cmov(JitWidth width, JitConditional condition, RegPtr dst, RegPtr src) = 0;    
    virtual void direct_setcc(JitConditional condition, RegPtr dst) = 0;
    virtual void tryDirect(DecodedOp* op, std::function<void()> callback, std::function<void()> fallback) = 0;
    virtual void preCompile(DecodedOp* op, bool skippedOp = false) = 0;
    virtual void compile(DecodedOp* op) = 0;
    virtual void postCompile(DecodedOp* op) = 0;
    virtual bool directDoesAffectFlags(DecodedOp* op) = 0;

protected:
    RegPtr btMask(U32 bitMask, U32 reg);
    bool btStartFlags(DecodedOp* op);
    bool bsStartFlags(DecodedOp* op);
    void pushParam(std::vector<DynParam>& params, JitWidth width, RegPtr reg);
    void dshift(DecodedOp* op, JitWidth width, InstRegRegImm callback, LazyFlagType flags);
    void dshiftM(DecodedOp* op, JitWidth width, InstRegRegImm callback, LazyFlagType flags);
    void dshiftClM(DecodedOp* op, JitWidth width, InstRegRegCl callback, LazyFlagType flags);
    void dshiftCl(DecodedOp* op, JitWidth width, InstRegRegCl callback, LazyFlagType flags);
    void arithSetup(DecodedOp* op, U32& needsToSetFlags, LazyFlagType flags, RegPtr cf);
    void movs(U32 base, JitWidth valueWidth, U32 size, JitWidth regWidth);
    void movsr(JitWidth valueWidth, U32 size, JitWidth regWidth);
    void stos(JitWidth valueWidth, U32 size, JitWidth regWidth);
    void stosr(JitWidth valueWidth, U32 size, JitWidth regWidth);
    void lods(U32 base, JitWidth valueWidth, U32 size, JitWidth regWidth);
    void lodsr(JitWidth valueWidth, U32 size, JitWidth regWidth);
    void cmps(U32 base, JitWidth valueWidth, U32 size, JitWidth regWidth, LazyFlagType lazyFlags);
    void cmpsr(JitWidth valueWidth, U32 size, JitWidth regWidth, U32 rep_zero, LazyFlagType lazyFlags);
    void scas(JitWidth valueWidth, U32 size, JitWidth regWidth, LazyFlagType lazyFlags);
    void scasr(JitWidth valueWidth, U32 size, JitWidth regWidth, U32 rep_zero, LazyFlagType lazyFlags);
    void dynamic_jump(DecodedOp* op, JitConditional condition);
    void dynamic_cmov_R(JitWidth width, DecodedOp* op, JitConditional condition);
    void dynamic_cmov_M(JitWidth width, DecodedOp* op, JitConditional condition);
    void dynamic_set_R(DecodedOp* op, JitConditional condition);
    void dynamic_set_M(DecodedOp* op, JitConditional condition);
    using InstDiv = void(Jit::*)(JitWidth width, RegPtr destLow, RegPtr destHighAndRemainder, RegPtr src);
    void div8(DecodedOp* op, RegPtr src, bool isSigned, InstDiv callback);
    void div16(DecodedOp* op, RegPtr src, bool isSigned, InstDiv callback);
    void div32(DecodedOp* op, RegPtr src, bool isSigned, InstDiv callback);

    void nullReg(JitWidth regWidth, RegPtr reg, RegPtr rm) {}

    void writeRor32Flags(RegPtr reg);
    void writeRol32Flags(RegPtr reg);
    void writeRcl32Flags(RegPtr reg, RegPtr cf);
    void writeRcr32Flags(RegPtr reg, RegPtr cf);
};

class JitFlags {
private:
    void init(Jit* jit, CPU* cpu, DecodedOp* op, LazyFlagType flagsType, RegPtr cf);

public:
    JitFlags(Jit* jit, CPU* cpu, DecodedOp* op, LazyFlagType flagsType, RegPtr src, RegPtr cf, bool copySrc = false);
    JitFlags(Jit* jit, CPU* cpu, DecodedOp* op, LazyFlagType flagsType, U32 src, RegPtr cf);

    void commit(RegPtr result, RegPtr dst = nullptr);
    void copyDest(RegPtr dst);

    Jit* jit;
    RegPtr src;
    RegPtr dst;
    U32 srcValue = 0;
    RegPtr oldCF;
    bool shouldStoreOldCF;
    const LazyFlags* flags;
    U32 needsToSetFlags = 0;
    LazyFlagType flagsType;
    bool dstInCpuTmp = false;
};
#endif
