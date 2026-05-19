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

/*
 * JitWasmCodeGen: Boxedwine JIT backend that emits WebAssembly bytecode.
 *
 * Design overview
 * ---------------
 * Each compiled basic block becomes a standalone WASM module containing one
 * function: `execute(cpu_ptr: i32)`.  On first call (runCount == JIT_RUN_COUNT)
 * the WASM binary is compiled and instantiated via WebAssembly.Module +
 * WebAssembly.Instance (synchronous, as in the QEMU wasm64 backend).  The
 * resulting function is added to Emscripten's wasmTable and the table index is
 * stored in DecodedOp::pfnJitCode.  Subsequent calls go through
 * wasmStartJITOp() which reads that index and calls the compiled function.
 *
 * Register model
 * --------------
 * WASM has no native register file.  We use WASM *local variables* (i32) as
 * the JIT register file:
 *
 *   local 0   : function parameter — cpu_ptr (i32, pointer into linear memory)
 *   locals 1-8 : GP registers eax–edi (loaded from CPU struct on first use,
 *                written back to CPU struct on block exit / sync)
 *   locals 9-12: segment base addresses cs, ds, ss, es
 *   locals 13-44: scratch temporaries
 *
 * The JitReg::hardwareReg() field stores the WASM local variable index.
 * Emulated register n maps to local n+1 (so eax=1, ecx=2, ..., edi=8).
 *
 * CPU state access
 * ----------------
 * The CPU struct lives in Emscripten linear memory.  All reads/writes use
 * i32.load / i32.store with the cpu_ptr as base and the field's offsetof()
 * as the static offset.  Helper C++ functions are imported via the function
 * table for operations that are impractical to inline (memory read/write,
 * complex flags, etc.).
 *
 * Inline TLB (emulated memory)
 * ----------------------------
 * For plain emulated reads and writes, JitWasmCodeGen::read and ::write
 * inline a TLB fast path before the helper call:
 *
 *   entry = wasm{Read,Write}PageBase[addr >> 12]
 *   if (entry == 0 || (addr & 0xfff) > 0x1000 - width) {
 *       slow: existing helper-based load/store
 *   } else {
 *       i32.load{8u,16u,_}  (entry + (addr & K_PAGE_MASK))   // read
 *       i32.store{8,16,_}   (entry + (addr & K_PAGE_MASK), value)  // write
 *   }
 *
 * The boundary check is omitted for b8 (every byte is in some page).
 * The arrays live in KMemoryData (BOXEDWINE_WASM_JIT-gated, ~8 MB total);
 * each entry is the 32-bit linear-memory offset of the page's RAM start,
 * or 0 if the page can't be directly accessed (CodePage, on-demand,
 * permission-denied). They're populated in onPageChanged alongside the
 * existing readCache/writeCache.
 *
 * SMC bailout is preserved by construction: CodePage::canWriteRam is
 * false, so wasmWritePageBase[CodePage] == 0, so writes that could land
 * on the active block always slow-path through the bailout-checking
 * helper.
 *
 * The RMW path (readWriteMem) is NOT TLB-fast-pathed — see the comment
 * in jitWasmCodeGen.cpp above readWriteMem for why.
 *
 * Ops still routed through emulateSingleOp
 * ----------------------------------------
 * The dynamic_* overrides further down hand certain instructions to the
 * normal CPU interpreter via emulateSingleOp() instead of inlining them.
 * Each fallback exists for a specific reason; if you remove one,
 * understand which constraint it papered over before doing so.
 *
 *   rol/ror b8 + b16            i32.rotl/rotr rotate within the full
 *                               32-bit word; narrowing produces wrong
 *                               results (e.g. ROL b8 0x80,1 → 0x00
 *                               instead of 0x01).
 *
 *   rcl/rcr (all widths)        Need lazy CF chaining across the
 *   shrd/shld                   carry-fold; no native WASM equivalent
 *                               and the helper-based codegen the base
 *                               class uses doesn't synthesize it.
 *
 *   mul/imul (R8/E8/...)        Lazy CF/OF for "did the result fit in
 *   div/idiv                    the low half"; div also needs the #DE
 *                               trap on zero. Routed through emulator
 *                               which already has the precise
 *                               semantics.
 *   bsf/bsr
 *
 *   xadd, cmpxchg, cmpxchg8b    Atomic + lock-prefix semantics; we'd
 *                               need to coordinate with the lock
 *                               runtime in common_lock.cpp.
 *
 *   pushA / popA                Eight sequential pushes/pops; no
 *                               base-class codegen and the manual
 *                               version would need stack-overflow
 *                               handling for each one.
 */

#ifndef __JIT_WASM_CODE_GEN_H__
#define __JIT_WASM_CODE_GEN_H__

#ifdef BOXEDWINE_WASM_JIT

#include "../jit/jitCodeGen.h"
#include "wasmEmitter.h"
#include <array>

// Number of WASM locals used as GP register slots (eax-edi = 8).
static constexpr U32 WASM_GP_LOCAL_COUNT  = 8;
// Index of the cpu_ptr parameter local (always 0 — the function parameter).
static constexpr U32 WASM_CPU_LOCAL       = 0;
// First GP register local (eax).
static constexpr U32 WASM_GP_LOCAL_BASE   = 1;
// Segment address locals (cs, ds, ss, es).
static constexpr U32 WASM_SEG_LOCAL_BASE  = 9;
// Scratch temporaries.
static constexpr U32 WASM_TMP_LOCAL_BASE  = 13;
static constexpr U32 WASM_TMP_LOCAL_COUNT = 32;
// i64 scratch local for 64-bit multiply (imulRRI/imulRR overflow tracking).
static constexpr U32 WASM_I64_SCRATCH     = 45;  // WASM_TMP_LOCAL_BASE + WASM_TMP_LOCAL_COUNT
// Total locals beyond the parameter.
static constexpr U32 WASM_LOCAL_COUNT     = 46;

// ---------------------------------------------------------------------------
// Mapping from emulated register index to WASM local index.
// emulatedReg 0-7 → WASM_GP_LOCAL_BASE + emulatedReg
// ---------------------------------------------------------------------------
static inline U32 wasmLocalForGPReg(U8 emulatedReg) {
    return WASM_GP_LOCAL_BASE + emulatedReg;
}

// ---------------------------------------------------------------------------
// JitWasmCodeGen
// ---------------------------------------------------------------------------
class JitWasmCodeGen : public JitCodeGen {
public:
    explicit JitWasmCodeGen(CPU* cpu);
    ~JitWasmCodeGen() override;

    // -----------------------------------------------------------------------
    // JitCodeGen / Jit pure virtual interface
    // (all methods must be implemented to satisfy the abstract base classes)
    // -----------------------------------------------------------------------

    // --- Register management ---
    RegPtr getStringRegEcx() override;
    RegPtr getStringRegEsi() override;
    RegPtr getStringRegEdi() override;
    RegPtr getReg(U8 reg, S8 hint = -1, bool load = true) override;
    RegPtr getReg8(U8 reg, bool load = true) override;
    RegPtr getReadOnlyReg(U8 reg, bool delayed = false, S8 hint = -1) override;
    RegPtr getReadOnlyReg8(U8 reg, bool delayed = false, S8 hint = -1) override;
    RegPtr getTmpReg() override;
    RegPtr getTmpReg8() override;
    RegPtr getTmpRegWithHint(S8 hint) override;
    RegPtr getTmpRegForCallResult() override;
    RegPtr getTmpReg(U8 reg, bool delayed = false, S8 hint = -1) override;
    RegPtr getTmpReg8(U8 reg, bool delayed = false, S8 hint = -1) override;
    RegPtr getReadOnlySegAddress(U8 seg) override;
    RegPtr getTmpSegAddress(U8 seg) override;
    RegPtr getReadOnlySegValue(U8 seg) override;
    bool   isTmpRegAvailable() override;
    void   forceSyncBackIfNotCached(RegPtr reg) override;

    // --- EIP ---
    RegPtr readEip() override;
    void   writeEip(RegPtr eip) override;
    void   writeEip(U32 eip) override;

    // --- Jumps ---
    void jmpHost(RegPtr reg) override;
    void jmpHost(DYN_PTR_SIZE address) override;

    // --- Arithmetic ---
    void addReg(JitWidth w, RegPtr reg, RegPtr rm) override;
    void addValue(JitWidth w, RegPtr reg, U32 imm) override;
    void orReg(JitWidth w, RegPtr reg, RegPtr rm) override;
    void orValue(JitWidth w, RegPtr reg, U32 imm) override;
    void subReg(JitWidth w, RegPtr reg, RegPtr rm) override;
    void subValue(JitWidth w, RegPtr reg, U32 imm) override;
    void andReg(JitWidth w, RegPtr reg, RegPtr rm) override;
    void andValue(JitWidth w, RegPtr reg, U32 imm) override;
#ifdef BOXEDWINE_64
    void andValue64(RegPtr reg, U64 imm) override;
#endif
    void xorReg(JitWidth w, RegPtr reg, RegPtr rm) override;
    void xorValue(JitWidth w, RegPtr reg, U32 imm) override;
    void shrReg(JitWidth w, RegPtr reg, RegPtr rm) override;
    void shrValue(JitWidth w, RegPtr reg, U32 imm) override;
    void shlReg(JitWidth w, RegPtr reg, RegPtr rm) override;
    void shlValue(JitWidth w, RegPtr reg, U32 imm) override;
    void sarReg(JitWidth w, RegPtr reg, RegPtr rm) override;
    void sarValue(JitWidth w, RegPtr reg, U32 imm) override;
    void notReg2(JitWidth w, RegPtr reg) override;
    void negReg2(JitWidth w, RegPtr reg) override;
    void bsfReg(JitWidth w, RegPtr reg, RegPtr rm) override;
    void bsrReg(JitWidth w, RegPtr reg, RegPtr rm) override;
    void rolReg(JitWidth w, RegPtr reg, RegPtr rm) override;
    void rolValue(JitWidth w, RegPtr reg, U32 imm) override;
    void rorReg(JitWidth w, RegPtr reg, RegPtr rm) override;
    void rorValue(JitWidth w, RegPtr reg, U32 imm) override;
    void rclReg(JitWidth w, RegPtr reg, RegPtr rm, RegPtr cf) override;
    void rclValue(JitWidth w, RegPtr reg, U32 imm, RegPtr cf) override;
    void rcrReg(JitWidth w, RegPtr reg, RegPtr rm, RegPtr cf) override;
    void rcrValue(JitWidth w, RegPtr reg, U32 imm, RegPtr cf) override;
    void shrdReg(JitWidth w, RegPtr reg, RegPtr rm, RegPtr cl) override;
    void shrdValue(JitWidth w, RegPtr reg, RegPtr rm, U32 imm) override;
    void shldReg(JitWidth w, RegPtr reg, RegPtr rm, RegPtr cl) override;
    void shldValue(JitWidth w, RegPtr reg, RegPtr rm, U32 imm) override;
    void xchgReg(JitWidth w, RegPtr dest, RegPtr src) override;
    void xaddReg(JitWidth w, RegPtr reg, RegPtr rm) override;
    void mulReg(JitWidth w, RegPtr reg) override;
    void imulReg(JitWidth w, RegPtr reg) override;
    void imulRRI(JitWidth w, RegPtr dst, RegPtr src, U32 src2, RegPtr overflow = nullptr) override;
    void imulRR(JitWidth w, RegPtr dst, RegPtr src, RegPtr overflow = nullptr) override;
    void divRegRegWithRemainder(JitWidth w, RegPtr dest, RegPtr destHigh, RegPtr src) override;
    void idivRegRegWithRemainder(JitWidth w, RegPtr dest, RegPtr destHigh, RegPtr src) override;
    void byteSwapReg32(RegPtr reg) override;
    RegPtr compareReg(JitWidth w, RegPtr r1, RegPtr r2, JitEvaluate cond, RegPtr res = nullptr) override;
    RegPtr compareValue(JitWidth w, RegPtr r, U32 val, JitEvaluate cond, RegPtr res = nullptr) override;
    RegPtr testZeroReg(JitWidth w, RegPtr reg, RegPtr res = nullptr) override;
    void absReg(JitWidth w, RegPtr reg) override;
    void clzReg(JitWidth w, RegPtr result, RegPtr reg) override;

    // --- Move ---
    void mov(JitWidth w, RegPtr dest, RegPtr src) override;
    void movzx(JitWidth dw, RegPtr dest, JitWidth sw, RegPtr src) override;
    void movsx(JitWidth dw, RegPtr dest, JitWidth sw, RegPtr src) override;
    void movValue(JitWidth w, RegPtr dst, DYN_PTR_SIZE imm) override;

    // --- Flags ---
    void storeLazyFlagType(LazyFlagType flags) override;
    void storeLazyFlagsDest(RegPtr reg) override;
    void storeLazyFlagsSrc(RegPtr reg) override;
    void storeLazyFlagsSrc(U32 value) override;
    void storeLazyFlagsResult(RegPtr reg) override;
    void storeLazyFlagsOldCF(RegPtr reg) override;
    void fillFlags(U32 flags = PF | SF | AF | CF | OF | ZF) override;
    RegPtr getZF() override;
    RegPtr getCF() override;
    void orCPUFlags(RegPtr reg) override;
    void xorCPUFlagsImmV2(U32 imm) override;
    void andCPUFlagsImmV2(U32 imm) override;
    void orCPUFlagsImmV2(U32 imm) override;
    RegPtr getReadOnlyFlags(RegPtr tmp = nullptr) override;
    RegPtr getFlagsInTmp(RegPtr reg = nullptr) override;
    void setFlags(RegPtr flags, U32 mask) override;
    void writeFlags(RegPtr flags) override;
    RegPtr getCondition(JitConditional cond, RegPtr res = nullptr) override;
    RegPtr getConditionCalculationReg(U32 index = 0) override;

    // --- Control flow ---
    void If(JitWidth w, RegPtr reg) override;
    void IfTest(JitWidth w, RegPtr reg, RegPtr mask) override;
    void IfTest(JitWidth w, RegPtr reg, U32 mask) override;
    void IfNotTestBit(JitWidth w, RegPtr reg, U32 bitPos) override;
    void IfTestBit(JitWidth w, RegPtr reg, U32 bitPos) override;
    void IfEqual(JitWidth w, RegPtr reg, DYN_PTR_SIZE value) override;
    void IfEqual(JitWidth w, RegPtr r1, RegPtr r2) override;
    void IfNotEqual(JitWidth w, RegPtr reg, DYN_PTR_SIZE value) override;
    void IfNotEqual(JitWidth w, RegPtr reg, RegPtr r2) override;
    void IfLessThan(JitWidth w, ComparisonType type, RegPtr reg, U32 value) override;
    void IfLessThan(JitWidth w, ComparisonType type, RegPtr r1, RegPtr r2) override;
    void IfGreaterThanOrEqual(JitWidth w, ComparisonType type, RegPtr r1, RegPtr r2) override;
    void IfGreaterThanOrEqual(JitWidth w, ComparisonType type, RegPtr reg, U32 value) override;
    void IfGreaterThan(JitWidth w, ComparisonType type, RegPtr r1, RegPtr r2) override;
    void IfGreaterThan(JitWidth w, ComparisonType type, RegPtr reg, U32 value) override;
    void IfNot(JitWidth w, RegPtr reg) override;
    void IfNotCPU(JitWidth w, RegPtr sib, U8 lsl, U32 offset) override;
    void IfCondition(JitConditional cond) override;
    void JumpIfCondition(JitConditional cond, U32 address) override;
    U32  MarkJumpLocation() override;
    void Goto(U32 location) override;
    U32  LoopBegin() override;
    void LoopEnd() override;
    void IfDF() override;
    void IfSmallStack() override;
    void StartElse() override;
    void EndIf() override;
    void JumpInBlock(U32 address) override;
    void blockExit() override;
    void blockNext1(U32 eip, DecodedOp* op) override;
    void blockNext2(U32 eip, DecodedOp* op) override;
    void jumpEip(RegPtr reg) override;
    bool canJumpInBlock(DecodedOp* op) override;
    bool canJumpInBlock(U32 opEip, DecodedOp* op) override;
    void onTestEnd(DecodedOp* op) override;

    // --- Memory access (JitCodeGen pure virtuals) ---
    void readMMU(RegPtr dest, RegPtr index, U32 offset = 0) override;
    void readMMU(RegPtr dest, U32 index) override;
    RegPtr readCPU(JitWidth w, U32 offset, RegPtr res = nullptr) override;
    RegPtr readCPU(JitWidth w, RegPtr sib, U8 lsl, U32 offset, RegPtr res = nullptr) override;
    void writeCPU(JitWidth w, RegPtr sib, U8 lsl, U32 offset, RegPtr src) override;
    void writeCPU(JitWidth w, U32 offset, RegPtr src) override;
    void writeCPUValue(JitWidth w, RegPtr sib, U8 lsl, U32 offset, DYN_PTR_SIZE src) override;
    void writeCPUValue(JitWidth w, U32 offset, DYN_PTR_SIZE src) override;
    void readHost(JitWidth w, MemPtr address, RegPtr result, bool emulatedMemory = true) override;
    void writeHost(JitWidth w, MemPtr address, RegPtr src, bool emulatedMemory = true) override;
    void writeHost(JitWidth w, MemPtr address, U32 imm, bool emulatedMemory = true) override;
    void clearMMUPermissionIfSpansPage(JitWidth w, RegPtr offset, RegPtr reg) override;
    void clearIfSpansPage(JitWidth w, RegPtr offset, RegPtr reg) override;

    // --- Emulated memory read/write (with MMU) ---
    RegPtr readWriteMem(JitWidth w, RegPtr addressReg,
                        std::function<void(RegPtr)> prepareWrite,
                        S8 hint = -1) override;
    RegPtr read(JitWidth w, RegPtr addressReg,
                std::function<void(MemPtr)> customOp = nullptr,
                std::function<void()> failedOp = nullptr,
                RegPtr tmp = nullptr, bool checkAlignment = true) override;
    void write(JitWidth w, RegPtr addressReg, RegPtr src,
               std::function<void(MemPtr)> customOp = nullptr,
               std::function<void()> failedOp = nullptr,
               bool checkAlignment = true) override;
    RegPtr read(JitWidth w, MemPtr address, RegPtr result = nullptr) override;
    void   write(JitWidth w, MemPtr address, RegPtr src) override;
    void   write(JitWidth w, MemPtr address, U32 imm) override;

    // --- Function calls ---
    void callHostFunction(void* address, const std::vector<DynParam>& params,
                          bool restoreCache = true, bool saveCache = true) override;
    void callHostFunctionWithResult(RegPtr result, void* address,
                                    const std::vector<DynParam>& params) override;
    void emulateSingleOp() override;
    void nakedCall(RegPtr reg) override;
    void nakedReturn() override;

    // --- Direct operations ---
    void direct_cmp(JitWidth w, RegPtr left, RegPtr right) override;
    void direct_cmp(JitWidth w, RegPtr left, U32 right) override;
    void direct_test(JitWidth w, RegPtr left, RegPtr right) override;
    void direct_test(JitWidth w, RegPtr left, U32 right) override;
    void direct_jump(JitConditional cond, U32 address) override;
    void direct_cmov(JitWidth w, JitConditional cond, RegPtr dst, RegPtr src) override;
    void direct_setcc(JitConditional cond, RegPtr dst) override;
    bool directDoesAffectFlags(DecodedOp* op) override;

    // Shift/rotate family: WASM's i32.shl/shr/rotl truncate behavior doesn't
    // match x86 byte/word semantics for CF/OF, and the base dynamic_RI lazy-
    // flag path here is fragile. Route every shift/rotate through the normal
    // CPU via emulateSingleOp. Perf hit is acceptable for the WASM target.
    void dynamic_rol8_reg_op(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_rol8_mem_op(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_rol8cl_reg_op(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_rol8cl_mem_op(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_rol16_reg_op(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_rol16_mem_op(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_rol16cl_reg_op(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_rol16cl_mem_op(DecodedOp* op) override { emulateSingleOp(); }
    // rol32/ror32 — base class emits native i32.rotl/rotr (32-bit rotate
    // is correct on full-width values). b8/b16 forms still emulate
    // because i32.rotl rotates within 32 bits and produces wrong results
    // when narrowed.
    void dynamic_ror8_reg_op(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_ror8_mem_op(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_ror8cl_reg_op(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_ror8cl_mem_op(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_ror16_reg_op(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_ror16_mem_op(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_ror16cl_reg_op(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_ror16cl_mem_op(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_rcl8_reg_op(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_rcl8_mem_op(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_rcl8cl_reg_op(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_rcl8cl_mem_op(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_rcl16_reg_op(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_rcl16_mem_op(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_rcl16cl_reg_op(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_rcl16cl_mem_op(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_rcl32_reg_op(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_rcl32_mem_op(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_rcl32cl_reg_op(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_rcl32cl_mem_op(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_rcr8_reg_op(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_rcr8_mem_op(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_rcr8cl_reg_op(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_rcr8cl_mem_op(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_rcr16_reg_op(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_rcr16_mem_op(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_rcr16cl_reg_op(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_rcr16cl_mem_op(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_rcr32_reg_op(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_rcr32_mem_op(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_rcr32cl_reg_op(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_rcr32cl_mem_op(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_shl8_reg_op(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_shl8_mem_op(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_shl8cl_reg_op(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_shl8cl_mem_op(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_shl16_reg_op(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_shl16_mem_op(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_shl16cl_reg_op(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_shl16cl_mem_op(DecodedOp* op) override { emulateSingleOp(); }
    // shl/shr/sar (all widths, imm + cl) — base-class codegen emits
    // native i32.shl/shr_u/shr_s with lazy-flag plumbing. WASM's shifts
    // implicitly mask count to 5 bits (matches x86 b32); b8/b16 forms
    // rely on emitBinOp's post-op maskToWidth, and sar additionally
    // sign-extends to 32 before i32.shr_s.
    void dynamic_dshlr16r16(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_dshle16r16(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_dshlr32r32(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_dshle32r32(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_dshlclr16r16(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_dshlcle16r16(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_dshlclr32r32(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_dshlcle32r32(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_dshrr16r16(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_dshre16r16(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_dshrr32r32(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_dshre32r32(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_dshrclr16r16(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_dshrcle16r16(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_dshrclr32r32(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_dshrcle32r32(DecodedOp* op) override { emulateSingleOp(); }

    // IMul/PopA: the multi-precision arithmetic (signed mul + overflow flag)
    // and the multi-word pop sequences don't have simple WASM primitives.
    // Route through the normal CPU.
    void dynamic_imulR8(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_imulE8(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_imulR16(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_imulE16(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_imulR32(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_imulE32(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_dimulcr16r16(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_dimulcr16e16(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_dimulcr32r32(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_dimulcr32e32(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_dimulr16r16(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_dimulr16e16(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_dimulr32r32(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_dimulr32e32(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_popA16(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_popA32(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_pushA16(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_pushA32(DecodedOp* op) override { emulateSingleOp(); }

    // MUL/DIV/IDIV/BSF/BSR/RCL/RCR/SHLD/SHRD: the JIT helpers (`mulReg`,
    // `divRegRegWithRemainder`, `bsfReg`, `rclReg`, `shldReg`, `absReg`, …)
    // are stubbed as `emulateSingleOp` here, which only works at the *op*
    // level. Inline use of those helpers — e.g. `Jit::div8` calls absReg+
    // absReg+callback within one op — would dispatch the entire instruction
    // multiple times via runNextSingleOp, corrupting state. Bypass the
    // helper-based codegen by overriding the whole op to emulateSingleOp.
    void dynamic_mulR8(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_mulE8(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_mulR16(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_mulE16(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_mulR32(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_mulE32(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_divR8(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_divE8(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_idivR8(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_idivE8(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_divR16(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_divE16(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_idivR16(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_idivE16(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_divR32(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_divE32(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_idivR32(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_idivE32(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_bsfr16r16(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_bsfr16e16(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_bsfr32r32(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_bsfr32e32(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_bsrr16r16(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_bsrr16e16(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_bsrr32r32(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_bsrr32e32(DecodedOp* op) override { emulateSingleOp(); }
    // (RCL/RCR/SHLD/SHRD ops already overridden in the shift/rotate block above.)

    // XADD / CMPXCHG: lazy-flag plumbing in dynamic_RM_WriteM and friends
    // doesn't round-trip cleanly to WASM for these. Defer to normal CPU.
    void dynamic_xaddr8r8(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_xaddr8e8(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_xaddr16r16(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_xaddr16e16(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_xaddr32r32(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_xaddr32e32(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_cmpxchgr8r8(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_cmpxchge8r8(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_cmpxchgr16r16(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_cmpxchge16r16(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_cmpxchgr32r32(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_cmpxchge32r32(DecodedOp* op) override { emulateSingleOp(); }
    void dynamic_cmpxchgg8b(DecodedOp* op) override { emulateSingleOp(); }

    // String ops (movs/cmps/stos/lods/scas, all widths) now use the
    // base-class native codegen; the rep'd loops route through
    // LoopBegin/LoopEnd which emit a structural WASM `loop`/`end` pair.







    // --- Code management ---
    U32  getBufferSize() override;
    U32  markBufferLocation() override;
    U32  getBufferLocation(U32 id) override;
    void copyBuffer(U8* dst, U32 size) override;
    U32  getIfJumpSize() override;
    U8*  createSyncToHost() override;
    U8*  createSyncFromHost() override;
    U8*  createBlockExit() override;
    U8*  createStartJITCode() override;
    // Returns a non-null stub so doJIT's calculateCF[0] null-check passes.
    // The stub is never actually called from WASM (nakedCall is a no-op).
    U8*  createDynamicExecutableMemory(U32* pSize = nullptr) override;

    // --- Compilation lifecycle ---
    void preCompile(DecodedOp* op, bool skippedOp = false) override;
    void compile(DecodedOp* op) override;
    void postCompile(DecodedOp* op) override;

    // virtual commitJIT is inherited from JitCodeGen and calls createStartJITCode
    void commitJIT(DecodedOp* op) override;

protected:
    // Helpers used internally during code generation
    void loadGPReg(U8 emulatedReg);   // emit: local.get cpu; i32.load; local.set localN
    void storeGPReg(U8 emulatedReg);  // emit: local.get cpu; local.get localN; i32.store

    // Emit cpu_ptr onto WASM stack (local.get 0)
    void pushCpuPtr();

    // Emit load of a register value onto the WASM stack (does NOT set a local).
    // Honors JitReg::isHigh (shifts right 8) so the value at stack top
    // represents the register *as referenced* (e.g. AH value).
    void pushRegValue(RegPtr reg);
    // Emit store from WASM stack top to register's local.
    // For GP registers at 8/16-bit width, merges with the existing local so
    // that unreferenced upper bits (or other half of a byte pair) are preserved.
    void popToReg(JitWidth w, RegPtr reg);
    // Back-compat: default to 32-bit writes.
    void popToReg(RegPtr reg) { popToReg(JitWidth::b32, reg); }

    // Mask a value to the given width and leave on WASM stack
    void maskToWidth(JitWidth w);

    // Helper: emit an i32 binary operation between two registers
    void emitBinOp(JitWidth w, RegPtr dst, RegPtr src, U8 wasmOp32, U8 wasmOp64 = 0);
    void emitBinOpImm(JitWidth w, RegPtr dst, U32 imm, U8 wasmOp32);

    // Emit a C++ helper call via function table index
    // The helper has signature: void helper(CPU* cpu)
    void callCppHelper(void* helperFn);

    // Allocate a scratch local (from WASM_TMP_LOCAL_BASE range)
    U32 allocScratch();
    void freeScratch(U32 local);

    // Common tail of every IfXxx: emit the WASM `if`. The condition is
    // already on the value stack.
    void finishIf();

    // Store a GP/scratch RegPtr into a CPU struct field (used to stage
    // mem-helper args — address/value — without touching lazy-flag state).
    void storeMemHelperField(U32 offset, RegPtr reg);

    // Flatten a MemPtr (base + index*scale + disp) into a single 32-bit
    // virtual-address scratch reg. Used by the emulated-memory read/write
    // overloads; the result feeds the MMU helper call.
    RegPtr memPtrToAddressReg(MemPtr address);

    // Called at the start of each IfXxx and at StartElse/EndIf: flush any
    // dirty GP regs to the CPU struct and invalidate the compile-time
    // load-cache. This keeps the two branches in sync: neither can assume a
    // local was populated by a load emitted in the other.
    void branchBoundary();

    // Self-modifying-code bailout check, emitted after a JIT-inline memory
    // write. If the write invalidated the active block (cpu->wasmJitBailout
    // set by the checking write helper), set cpu->eip to the next op and
    // exit so the dispatcher re-decodes. Uses lastCompiledOpLen captured
    // by preCompile.
    void emitBailoutCheck();
    U32 lastCompiledOpLen = 0;

    WasmEmitter m_emitter;

    // Type indices pre-registered for common helper signatures
    U32 m_typeVoidVoid  = 0;  // () -> ()
    U32 m_typeVoidI32   = 0;  // (i32) -> ()
    U32 m_typeI32Void   = 0;  // () -> i32
    U32 m_typeI32I32    = 0;  // (i32) -> i32
    U32 m_typeVoidI32I32= 0;  // (i32, i32) -> ()

    // Imported function indices
    U32 m_helperReadMemIdx  = 0;
    U32 m_helperWriteMemIdx = 0;
    U32 m_helperEmulateSingleOpIdx = 0;
    U32 m_helperGetNextOpIdx = 0;
    U32 m_helperSyncToHostIdx = 0;

    // Tracks which GP locals are loaded and which are dirty (need writeback)
    std::array<bool, WASM_GP_LOCAL_COUNT> m_gpLoaded{};
    std::array<bool, WASM_GP_LOCAL_COUNT> m_gpDirty{};
    std::array<bool, 4> m_segLoaded{};

    // Buffer used by getBufferSize/markBufferLocation for branch patching
    std::vector<U8>  m_patchBuffer;
    std::vector<U32> m_patchLocations;

    // Scratch allocation
    std::array<bool, WASM_TMP_LOCAL_COUNT> m_scratchInUse{};

    // The current block's WASM binary (set in commitJIT)
    std::vector<U8> m_wasmBinary;

    // Helper: emit code to sync all dirty GP registers back to CPU struct
    void syncDirtyRegsToHost();
    // Helper: emit code to load all GP registers from CPU struct
    void loadAllGPRegs();
};

// ---------------------------------------------------------------------------
// Emscripten JS interop for WASM module instantiation and calling.
// Declared here, implemented in jitWasmCodeGen.cpp as EM_JS functions.
// ---------------------------------------------------------------------------

// Compile and instantiate a WASM binary. Returns the wasmTable index of the
// compiled "execute" function, or -1 on failure.
// importFns: array of C++ function pointers that the module imports.
extern "C" int  boxedwine_wasm_instantiate(const void* bytes, int size,
                                            const void** importFns, int importCount);

// Release a compiled block (remove from wasmTable).
extern "C" void boxedwine_wasm_free_block(int tableIndex);

// The static OpCallback used as startJITOp for WASM-compiled blocks.
void OPCALL wasmStartJITOp(CPU* cpu, DecodedOp* op);

#endif // BOXEDWINE_WASM_JIT
#endif // __JIT_WASM_CODE_GEN_H__
