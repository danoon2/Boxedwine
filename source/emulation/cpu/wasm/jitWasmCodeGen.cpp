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

#ifdef BOXEDWINE_WASM_JIT

#include "jitWasmCodeGen.h"
#include "../jit/jitCodeGen.h"
#include "../common/cpu.h"
#include "../normal/normalCPU.h"
#include "../../softmmu/soft_code_page.h"
#include "../../softmmu/kmemory_soft.h"

#include <emscripten.h>
#include <emscripten/em_js.h>

#ifdef BOXEDWINE_MULTI_THREADED
// Serializes addFunction()/removeFunction() calls across worker threads.
// Under Emscripten pthreads the wasmTable is shared; wasmTable.grow() (called
// internally by addFunction) is not thread-safe across workers. This mutex is
// backed by Atomics on SharedArrayBuffer so it synchronizes cross-worker.
static BOXEDWINE_MUTEX g_wasmJitTableMutex;
#endif

// ---------------------------------------------------------------------------
// JS interop: compile and instantiate a WASM module synchronously.
//
// The generated module imports:
//   - "env"."memory"          : Emscripten linear memory
//   - "helpers"."fn_0" etc.   : C++ helper functions (by table index)
//
// Returns the wasmTable index of the compiled "execute(cpu_ptr: i32)" function,
// or -1 on failure.
// ---------------------------------------------------------------------------
EM_JS(int, boxedwine_wasm_instantiate,
      (const void* bytes, int size, const void** importFns, int importCount),
{
    try {
        var wasmBytes = new Uint8Array(HEAPU8.buffer, bytes, size);
        var mod = new WebAssembly.Module(wasmBytes);

        // Build the helper imports object.  Each entry in importFns is a C++
        // function pointer that already lives in wasmTable (since all Emscripten
        // functions are in the table).
        var helpers = {};
        var view = new Int32Array(HEAP32.buffer, importFns, importCount);
        for (var i = 0; i < importCount; i++) {
            helpers['fn_' + i] = wasmTable.get(view[i]);
        }

        var inst = new WebAssembly.Instance(mod, {
            'env':     { 'memory': wasmMemory },
            'helpers': helpers
        });

        // Add the "execute" export to wasmTable and return its index.
        var fn = inst.exports['execute'];
        if (!fn) return -1;
        var idx = addFunction(fn, 'vi');
        return idx;
    } catch(e) {
        console.error('boxedwine_wasm_instantiate failed:', e);
        return -1;
    }
});

EM_JS(void, boxedwine_wasm_call_block, (int tableIndex, int cpuPtr),
{
    wasmTable.get(tableIndex)(cpuPtr);
});

EM_JS(void, boxedwine_wasm_free_block, (int tableIndex),
{
    removeFunction(tableIndex);
});

// ---------------------------------------------------------------------------
// The OpCallback installed as process->startJITOp for WASM-compiled blocks.
// DecodedOp::pfnJitCode stores the wasmTable index (cast to void*).
// ---------------------------------------------------------------------------
void OPCALL wasmStartJITOp(CPU* cpu, DecodedOp* op) {
    if (op->pfnJitCode) {
        boxedwine_wasm_call_block((int)(uintptr_t)op->pfnJitCode, (int)(uintptr_t)cpu);
        // nextOp is updated by the WASM block itself (via helper call).
    }
}

// ---------------------------------------------------------------------------
// C++ helpers imported by generated WASM modules
// ---------------------------------------------------------------------------

// Memory helpers read their address from cpu->memHelperAddr and their
// value to/from cpu->memHelperValue. Using dedicated scratch fields
// (rather than cpu->src.u32 / cpu->dst.u32) keeps lazy-flag state intact
// across an RMW op's callback.
static void wasmHelper_readMem32(CPU* cpu) {
    cpu->memHelperValue = cpu->memory->readd(cpu->memHelperAddr);
}
static void wasmHelper_writeMem32(CPU* cpu) {
    cpu->memory->writed(cpu->memHelperAddr, cpu->memHelperValue);
}
static void wasmHelper_readMem8(CPU* cpu) {
    cpu->memHelperValue = cpu->memory->readb(cpu->memHelperAddr);
}
static void wasmHelper_writeMem8(CPU* cpu) {
    cpu->memory->writeb(cpu->memHelperAddr, (U8)cpu->memHelperValue);
}
static void wasmHelper_readMem16(CPU* cpu) {
    cpu->memHelperValue = cpu->memory->readw(cpu->memHelperAddr);
}
static void wasmHelper_writeMem16(CPU* cpu) {
    cpu->memory->writew(cpu->memHelperAddr, (U16)cpu->memHelperValue);
}

// Same as the regular write helpers but additionally check whether the
// active JIT block's pfnJitCode got cleared by removeCodeBlock — meaning
// the write landed on the bytes we're currently executing. Used by
// `write(address, src)` in the inline JIT path; the JIT then emits a
// post-call check that exits the block on bailout. (We don't put this
// check on every write because push/pop go through `write` in
// `push16`/`push32` which is wrapped in IfSmallStack — adding the if/end
// check inside that nested control flow broke Xchg tests.)
static inline void checkActiveBlockAfterWrite(CPU* cpu) {
    DecodedOp* active = cpu->wasmJitActiveBlock;
    if (active && active->pfnJitCode == nullptr) {
        cpu->wasmJitBailout = 1;
    }
}
static void wasmHelper_writeMem32_check(CPU* cpu) {
    cpu->memory->writed(cpu->memHelperAddr, cpu->memHelperValue);
    checkActiveBlockAfterWrite(cpu);
}
static void wasmHelper_writeMem16_check(CPU* cpu) {
    cpu->memory->writew(cpu->memHelperAddr, (U16)cpu->memHelperValue);
    checkActiveBlockAfterWrite(cpu);
}
static void wasmHelper_writeMem8_check(CPU* cpu) {
    cpu->memory->writeb(cpu->memHelperAddr, (U8)cpu->memHelperValue);
    checkActiveBlockAfterWrite(cpu);
}

// Update cpu->nextOp by decoding the block at cpu->eip.u32
static void wasmHelper_fetchNextOp(CPU* cpu) {
    if (!cpu->thread->terminating) {
        cpu->nextOp = cpu->getNextOp();
    }
}

// Sync CPU lazy flags — flags are maintained in cpu->flags directly by WASM code
static void wasmHelper_syncFlags(CPU* cpu) {}

// Called at the top of every JIT block: capture which block is running so
// write helpers can detect when a write hit our own bytes (cleared
// pfnJitCode by removeCodeBlock), clear the leftover bailout flag, and
// refresh the inline-TLB array pointers so the JIT codegen can walk
// `wasmReadPageBase[addr>>12]` for direct host-pointer access without
// re-resolving cpu->memory->data each block.
static void wasmHelper_blockEnter(CPU* cpu) {
    cpu->wasmJitActiveBlock = cpu->nextOp;
    cpu->wasmJitBailout = 0;
    KMemoryData* d = getMemData(cpu->memory);
    cpu->wasmReadPageBaseArray  = (U32)(uintptr_t)d->wasmReadPageBase;
    cpu->wasmWritePageBaseArray = (U32)(uintptr_t)d->wasmWritePageBase;
}

// Fall back to the normal CPU interpreter for one instruction (used for
// operations the WASM JIT doesn't inline, e.g. FPU/SSE/complex shifts).
void jitRunSingleOp(CPU* cpu);   // defined in jitCodeGen.cpp
static void wasmHelper_emulateSingleOp(CPU* cpu) {
    jitRunSingleOp(cpu);
}

// Compute CF using the C++ lazy-flag machinery and stash the result in
// cpu->tmpReg. Used by the WASM backend's getCF() override since it can't
// do a nakedCall to the per-flag-type computation functions.
static void wasmHelper_computeCF(CPU* cpu) {
    cpu->tmpReg = cpu->getCF() ? 1 : 0;
}
static void wasmHelper_computeZF(CPU* cpu) {
    cpu->tmpReg = cpu->getZF() ? 1 : 0;
}

// Materialize all lazy flags into cpu->flags; reset lazyFlagType to FLAGS_NONE.
void common_fillFlags(CPU* cpu);
static void wasmHelper_fillFlags(CPU* cpu) {
    common_fillFlags(cpu);
}

// One helper per JitConditional: evaluates the condition against cpu state
// (honoring lazy flags via cpu->getXF()) and stashes 0/1 in cpu->tmpReg.
// Separate helpers avoid any need for an input parameter — using
// cpu->src.u32 would clobber live lazy-flag state.
#define WASM_COND_HELPER(NAME, EXPR)                                           \
    static void wasmHelper_cond_##NAME(CPU* cpu) { cpu->tmpReg = (EXPR) ? 1 : 0; }
WASM_COND_HELPER(O,   cpu->getOF())
WASM_COND_HELPER(NO,  !cpu->getOF())
WASM_COND_HELPER(B,   cpu->getCF())
WASM_COND_HELPER(NB,  !cpu->getCF())
WASM_COND_HELPER(Z,   cpu->getZF())
WASM_COND_HELPER(NZ,  !cpu->getZF())
WASM_COND_HELPER(BE,  cpu->getCF() || cpu->getZF())
WASM_COND_HELPER(NBE, !cpu->getCF() && !cpu->getZF())
WASM_COND_HELPER(S,   cpu->getSF())
WASM_COND_HELPER(NS,  !cpu->getSF())
WASM_COND_HELPER(P,   cpu->getPF())
WASM_COND_HELPER(NP,  !cpu->getPF())
WASM_COND_HELPER(L,   (cpu->getSF() ? 1 : 0) != (cpu->getOF() ? 1 : 0))
WASM_COND_HELPER(NL,  (cpu->getSF() ? 1 : 0) == (cpu->getOF() ? 1 : 0))
WASM_COND_HELPER(LE,  cpu->getZF() || ((cpu->getSF() ? 1 : 0) != (cpu->getOF() ? 1 : 0)))
WASM_COND_HELPER(NLE, !cpu->getZF() && ((cpu->getSF() ? 1 : 0) == (cpu->getOF() ? 1 : 0)))
#undef WASM_COND_HELPER

// ---------------------------------------------------------------------------
// Helper table: array of C++ function pointers exported to WASM modules.
// Order must match WasmHelperIdx below.
// ---------------------------------------------------------------------------
static const void* g_wasmHelperTable[] = {
    (void*)wasmHelper_readMem8,
    (void*)wasmHelper_writeMem8,
    (void*)wasmHelper_readMem16,
    (void*)wasmHelper_writeMem16,
    (void*)wasmHelper_readMem32,
    (void*)wasmHelper_writeMem32,
    (void*)wasmHelper_fetchNextOp,
    (void*)wasmHelper_syncFlags,
    (void*)wasmHelper_emulateSingleOp,
    (void*)wasmHelper_computeCF,
    (void*)wasmHelper_computeZF,
    (void*)wasmHelper_fillFlags,
    // 16 condition helpers — index order MUST match JitConditional enum.
    (void*)wasmHelper_cond_O,
    (void*)wasmHelper_cond_NO,
    (void*)wasmHelper_cond_B,
    (void*)wasmHelper_cond_NB,
    (void*)wasmHelper_cond_Z,
    (void*)wasmHelper_cond_NZ,
    (void*)wasmHelper_cond_BE,
    (void*)wasmHelper_cond_NBE,
    (void*)wasmHelper_cond_S,
    (void*)wasmHelper_cond_NS,
    (void*)wasmHelper_cond_P,
    (void*)wasmHelper_cond_NP,
    (void*)wasmHelper_cond_L,
    (void*)wasmHelper_cond_NL,
    (void*)wasmHelper_cond_LE,
    (void*)wasmHelper_cond_NLE,
    (void*)wasmHelper_blockEnter,
    (void*)wasmHelper_writeMem8_check,
    (void*)wasmHelper_writeMem16_check,
    (void*)wasmHelper_writeMem32_check,
};
static constexpr int WASM_HELPER_COUNT = (int)(sizeof(g_wasmHelperTable) / sizeof(g_wasmHelperTable[0]));

enum WasmHelperIdx {
    HELPER_READ_MEM8         = 0,
    HELPER_WRITE_MEM8        = 1,
    HELPER_READ_MEM16        = 2,
    HELPER_WRITE_MEM16       = 3,
    HELPER_READ_MEM32        = 4,
    HELPER_WRITE_MEM32       = 5,
    HELPER_FETCH_NEXT        = 6,
    HELPER_SYNC_FLAGS        = 7,
    HELPER_EMULATE_SINGLE_OP = 8,
    HELPER_COMPUTE_CF        = 9,
    HELPER_COMPUTE_ZF        = 10,
    HELPER_FILL_FLAGS        = 11,
    HELPER_COND_BASE         = 12, // add JitConditional index to this
    HELPER_BLOCK_ENTER       = 28, // 12 + 16 condition helpers
    HELPER_WRITE_MEM8_CHECK  = 29,
    HELPER_WRITE_MEM16_CHECK = 30,
    HELPER_WRITE_MEM32_CHECK = 31,
};

// ---------------------------------------------------------------------------
// JitWasmCodeGen constructor
// ---------------------------------------------------------------------------
JitWasmCodeGen::JitWasmCodeGen(CPU* cpu) : JitCodeGen(cpu) {
    // Register common function types for the module being built.
    m_typeVoidVoid   = m_emitter.addFuncType({}, {});
    m_typeVoidI32    = m_emitter.addFuncType({ WasmType::I32 }, {});
    m_typeI32Void    = m_emitter.addFuncType({}, { WasmType::I32 });
    m_typeI32I32     = m_emitter.addFuncType({ WasmType::I32 }, { WasmType::I32 });
    m_typeVoidI32I32 = m_emitter.addFuncType({ WasmType::I32, WasmType::I32 }, {});

    // Import linear memory.
    m_emitter.addMemoryImport("env", "memory");

    // Import the C++ helpers (all have signature (i32) -> () i.e. cpu_ptr).
    for (int i = 0; i < WASM_HELPER_COUNT; i++) {
        char name[16];
        snprintf(name, sizeof(name), "fn_%d", i);
        m_emitter.addFunctionImport("helpers", name, m_typeVoidI32);
    }
    // Map well-known helpers to their import indices.
    m_helperReadMemIdx          = HELPER_READ_MEM32;
    m_helperWriteMemIdx         = HELPER_WRITE_MEM32;
    m_helperGetNextOpIdx        = HELPER_FETCH_NEXT;
    m_helperSyncToHostIdx       = HELPER_SYNC_FLAGS;
    m_helperEmulateSingleOpIdx  = HELPER_EMULATE_SINGLE_OP;

    // Declare the single local function and export it.
    U32 execFuncIdx = m_emitter.addFunction(m_typeVoidI32);
    m_emitter.addExport("execute", execFuncIdx);

    // Begin the function body. Locals layout:
    //   local 0 (param): cpu_ptr i32
    //   locals 1-8 (i32 x8): GP registers eax-edi
    //   locals 9-12 (i32 x4): segment addresses cs, ds, ss, es
    //   locals 13-15 (i32 x3): scratch
    m_emitter.beginFunction({
        { 8, WasmType::I32 },  // GP registers (locals 1-8)
        { 4, WasmType::I32 },  // segment addresses (locals 9-12)
        { 8, WasmType::I32 },  // scratch temporaries (locals 13-20)
        { 1, WasmType::I64 },  // i64 scratch for 64-bit multiply (local 21)
    });

    // Self-modifying-code arming: capture cpu->wasmJitActiveBlock so the
    // checking write helpers can detect when removeCodeBlock cleared our
    // own pfnJitCode mid-flight. Also clears the leftover bailout flag.
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitCall(HELPER_BLOCK_ENTER);

    m_gpLoaded.fill(false);
    m_gpDirty.fill(false);
    m_segLoaded.fill(false);
    m_scratchInUse.fill(false);
}

JitWasmCodeGen::~JitWasmCodeGen() {}

// ---------------------------------------------------------------------------
// Internal helpers
// ---------------------------------------------------------------------------
void JitWasmCodeGen::pushCpuPtr() {
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
}

static U32 cpuRegOffset32(U8 emulatedReg) {
    return CPU::offsetofReg32(emulatedReg);
}

void JitWasmCodeGen::loadGPReg(U8 emulatedReg) {
    if (m_gpLoaded[emulatedReg]) return;
    U32 local = wasmLocalForGPReg(emulatedReg);
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitI32Load(cpuRegOffset32(emulatedReg));
    m_emitter.emitLocalSet(local);
    m_gpLoaded[emulatedReg] = true;
}

void JitWasmCodeGen::storeGPReg(U8 emulatedReg) {
    U32 local = wasmLocalForGPReg(emulatedReg);
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitLocalGet(local);
    m_emitter.emitI32Store(cpuRegOffset32(emulatedReg));
    m_gpDirty[emulatedReg] = false;
}

void JitWasmCodeGen::syncDirtyRegsToHost() {
    for (U8 i = 0; i < WASM_GP_LOCAL_COUNT; i++) {
        if (m_gpDirty[i]) storeGPReg(i);
    }
}

void JitWasmCodeGen::pushRegValue(RegPtr reg) {
    if (!reg) return;
    U8 emReg = reg->emulatedReg;
    if (emReg < WASM_GP_LOCAL_COUNT) {
        loadGPReg(emReg);
        m_emitter.emitLocalGet(wasmLocalForGPReg(emReg));
    } else {
        // temp register — just use local directly
        m_emitter.emitLocalGet(reg->hardwareReg());
    }
    // High-byte register reference (AH/CH/DH/BH): shift low byte of high pair
    // into position so callers masking with 0xff get the right byte.
    if (reg->isHigh) {
        m_emitter.emitI32Const(8);
        m_emitter.emitOp(WASM_I32_SHR_U);
    }
}

void JitWasmCodeGen::popToReg(JitWidth w, RegPtr reg) {
    if (!reg) { m_emitter.emitOp(WASM_DROP); return; }
    U8 emReg = reg->emulatedReg;
    bool isGP = emReg < WASM_GP_LOCAL_COUNT;
    U32 local = isGP ? wasmLocalForGPReg(emReg) : reg->hardwareReg();

    // Scratch temporaries are always full-width; any masking needed was done
    // by the caller. GP register writes at sub-word widths must preserve the
    // upper bits of the emulated register (x86 semantics for mov al, imm8
    // etc.), so we merge the new value with the existing local.
    if (!isGP || w == JitWidth::b32 || w == JitWidth::b64) {
        m_emitter.emitLocalSet(local);
    } else if (w == JitWidth::b16) {
        // stack: new16
        m_emitter.emitI32Const(0xFFFF);
        m_emitter.emitOp(WASM_I32_AND);
        m_emitter.emitLocalGet(local);
        m_emitter.emitI32Const((S32)0xFFFF0000);
        m_emitter.emitOp(WASM_I32_AND);
        m_emitter.emitOp(WASM_I32_OR);
        m_emitter.emitLocalSet(local);
    } else { // b8
        if (reg->isHigh) {
            // (existing & 0xFFFF00FF) | ((new8 & 0xFF) << 8)
            m_emitter.emitI32Const(0xFF);
            m_emitter.emitOp(WASM_I32_AND);
            m_emitter.emitI32Const(8);
            m_emitter.emitOp(WASM_I32_SHL);
            m_emitter.emitLocalGet(local);
            m_emitter.emitI32Const((S32)0xFFFF00FF);
            m_emitter.emitOp(WASM_I32_AND);
            m_emitter.emitOp(WASM_I32_OR);
            m_emitter.emitLocalSet(local);
        } else {
            // (existing & 0xFFFFFF00) | (new8 & 0xFF)
            m_emitter.emitI32Const(0xFF);
            m_emitter.emitOp(WASM_I32_AND);
            m_emitter.emitLocalGet(local);
            m_emitter.emitI32Const((S32)0xFFFFFF00);
            m_emitter.emitOp(WASM_I32_AND);
            m_emitter.emitOp(WASM_I32_OR);
            m_emitter.emitLocalSet(local);
        }
    }
    if (isGP) {
        m_gpLoaded[emReg] = true;
        m_gpDirty[emReg]  = true;
    }
}

U32 JitWasmCodeGen::allocScratch() {
    for (U32 i = 0; i < WASM_TMP_LOCAL_COUNT; i++) {
        if (!m_scratchInUse[i]) {
            m_scratchInUse[i] = true;
            return WASM_TMP_LOCAL_BASE + i;
        }
    }
    return WASM_TMP_LOCAL_BASE; // fallback: reuse 0 (shouldn't happen in practice)
}

void JitWasmCodeGen::freeScratch(U32 local) {
    if (local >= WASM_TMP_LOCAL_BASE && local < WASM_TMP_LOCAL_BASE + WASM_TMP_LOCAL_COUNT) {
        m_scratchInUse[local - WASM_TMP_LOCAL_BASE] = false;
    }
}

static RegPtr makeWasmReg(U8 hwLocal, U8 emulatedReg) {
    return std::make_shared<JitReg>(hwLocal, emulatedReg);
}

static RegPtr makeWasmReg(U8 hwLocal, U8 emulatedReg, bool isHigh) {
    return std::make_shared<JitReg>(hwLocal, emulatedReg, isHigh);
}

// Emit a masked value on the WASM stack.
void JitWasmCodeGen::maskToWidth(JitWidth w) {
    switch (w) {
    case JitWidth::b8:
        m_emitter.emitI32Const(0xff);
        m_emitter.emitOp(WASM_I32_AND);
        break;
    case JitWidth::b16:
        m_emitter.emitI32Const(0xffff);
        m_emitter.emitOp(WASM_I32_AND);
        break;
    case JitWidth::b32:
    case JitWidth::b64:
    default:
        break; // no masking needed for 32/64-bit
    }
}

// Helper: emit a binary operation that reads dst, applies op with src, writes dst.
void JitWasmCodeGen::emitBinOp(JitWidth w, RegPtr dst, RegPtr src, U8 wasmOp32, U8 wasmOp64) {
    // Stack: push dst, push src, op, (mask), pop dst
    pushRegValue(dst);
    if (w == JitWidth::b8 || w == JitWidth::b16) {
        maskToWidth(w);
    }
    pushRegValue(src);
    if (w == JitWidth::b8 || w == JitWidth::b16) {
        maskToWidth(w);
    }
    m_emitter.emitOp(wasmOp32);
    maskToWidth(w);
    popToReg(w, dst);
}

void JitWasmCodeGen::emitBinOpImm(JitWidth w, RegPtr dst, U32 imm, U8 wasmOp32) {
    pushRegValue(dst);
    if (w == JitWidth::b8 || w == JitWidth::b16) maskToWidth(w);
    m_emitter.emitI32Const((S32)imm);
    m_emitter.emitOp(wasmOp32);
    maskToWidth(w);
    popToReg(w, dst);
}

// ---------------------------------------------------------------------------
// Register management
// ---------------------------------------------------------------------------
RegPtr JitWasmCodeGen::getReg(U8 reg, S8 hint, bool load) {
    if (load) loadGPReg(reg);
    auto r = makeWasmReg((U8)wasmLocalForGPReg(reg), reg);
    m_gpDirty[reg] = true;
    return r;
}

RegPtr JitWasmCodeGen::getReg8(U8 reg, bool load) {
    // x86 encoding: 0-3 = AL/CL/DL/BL (low byte of reg[0-3]),
    //               4-7 = AH/CH/DH/BH (high byte of reg[0-3]).
    bool isHigh = (reg >= 4);
    U8 emReg    = isHigh ? (U8)(reg - 4) : reg;
    if (load) loadGPReg(emReg);
    auto r = makeWasmReg((U8)wasmLocalForGPReg(emReg), emReg, isHigh);
    m_gpDirty[emReg] = true;
    return r;
}

RegPtr JitWasmCodeGen::getReadOnlyReg(U8 reg, bool delayed, S8 hint) {
    if (!delayed) loadGPReg(reg);
    return makeWasmReg((U8)wasmLocalForGPReg(reg), reg);
}

RegPtr JitWasmCodeGen::getReadOnlyReg8(U8 reg, bool delayed, S8 hint) {
    bool isHigh = (reg >= 4);
    U8 emReg    = isHigh ? (U8)(reg - 4) : reg;
    if (!delayed) loadGPReg(emReg);
    return makeWasmReg((U8)wasmLocalForGPReg(emReg), emReg, isHigh);
}

RegPtr JitWasmCodeGen::getTmpReg() {
    U32 local = allocScratch();
    return makeWasmReg((U8)local, 0xff);
}

RegPtr JitWasmCodeGen::getTmpReg8()                  { return getTmpReg(); }
RegPtr JitWasmCodeGen::getTmpRegWithHint(S8 hint)    { return getTmpReg(); }
RegPtr JitWasmCodeGen::getTmpRegForCallResult()      { return getTmpReg(); }
RegPtr JitWasmCodeGen::getTmpReg(U8 reg, bool delayed, S8 hint) {
    auto r = getTmpReg();
    if (!delayed) {
        loadGPReg(reg);
        // copy: push local, pop to tmp
        m_emitter.emitLocalGet(wasmLocalForGPReg(reg));
        m_emitter.emitLocalSet(r->hardwareReg());
    }
    return r;
}
RegPtr JitWasmCodeGen::getTmpReg8(U8 reg, bool delayed, S8 hint) {
    auto r = getTmpReg();
    if (!delayed) {
        bool isHigh = (reg >= 4);
        U8 emReg = isHigh ? (U8)(reg - 4) : reg;
        loadGPReg(emReg);
        m_emitter.emitLocalGet(wasmLocalForGPReg(emReg));
        if (isHigh) {
            m_emitter.emitI32Const(8);
            m_emitter.emitOp(WASM_I32_SHR_U);
        }
        m_emitter.emitI32Const(0xff);
        m_emitter.emitOp(WASM_I32_AND);
        m_emitter.emitLocalSet(r->hardwareReg());
    }
    return r;
}

RegPtr JitWasmCodeGen::getReadOnlySegAddress(U8 seg) {
    U32 local = WASM_SEG_LOCAL_BASE + (seg & 3);
    if (!m_segLoaded[seg & 3]) {
        m_emitter.emitLocalGet(WASM_CPU_LOCAL);
        m_emitter.emitI32Load(CPU::offsetofSegAddress(seg));
        m_emitter.emitLocalSet(local);
        m_segLoaded[seg & 3] = true;
    }
    return makeWasmReg((U8)local, 0xfe);
}

RegPtr JitWasmCodeGen::getTmpSegAddress(U8 seg) {
    auto r = getTmpReg();
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitI32Load(CPU::offsetofSegAddress(seg));
    m_emitter.emitLocalSet(r->hardwareReg());
    return r;
}

RegPtr JitWasmCodeGen::getReadOnlySegValue(U8 seg) {
    auto r = getTmpReg();
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitI32Load(CPU::offsetofSegValue(seg));
    m_emitter.emitLocalSet(r->hardwareReg());
    return r;
}

RegPtr JitWasmCodeGen::getStringRegEcx() { return getReg(1); }
RegPtr JitWasmCodeGen::getStringRegEsi() { return getReg(6); }
RegPtr JitWasmCodeGen::getStringRegEdi() { return getReg(7); }

bool JitWasmCodeGen::isTmpRegAvailable() {
    for (U32 i = 0; i < WASM_TMP_LOCAL_COUNT; i++)
        if (!m_scratchInUse[i]) return true;
    return false;
}

void JitWasmCodeGen::forceSyncBackIfNotCached(RegPtr reg) {
    if (!reg) return;
    U8 emReg = reg->emulatedReg;
    if (emReg < WASM_GP_LOCAL_COUNT && m_gpDirty[emReg]) {
        storeGPReg(emReg);
        m_gpDirty[emReg] = false;
    }
}

// ---------------------------------------------------------------------------
// EIP
// ---------------------------------------------------------------------------
RegPtr JitWasmCodeGen::readEip() {
    auto r = getTmpReg();
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitI32Load((U32)offsetof(CPU, eip.u32));
    m_emitter.emitLocalSet(r->hardwareReg());
    return r;
}

void JitWasmCodeGen::writeEip(RegPtr eip) {
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    pushRegValue(eip);
    m_emitter.emitI32Store((U32)offsetof(CPU, eip.u32));
}

void JitWasmCodeGen::writeEip(U32 eip) {
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitI32Const((S32)eip);
    m_emitter.emitI32Store((U32)offsetof(CPU, eip.u32));
}

// ---------------------------------------------------------------------------
// CPU state read/write (JitCodeGen pure virtuals)
// ---------------------------------------------------------------------------
RegPtr JitWasmCodeGen::readCPU(JitWidth w, U32 offset, RegPtr res) {
    if (!res) res = getTmpReg();
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    switch (w) {
    case JitWidth::b8:  m_emitter.emitI32Load8U(offset);  break;
    case JitWidth::b16: m_emitter.emitI32Load16U(offset); break;
    default:            m_emitter.emitI32Load(offset);     break;
    }
    m_emitter.emitLocalSet(res->hardwareReg());
    return res;
}

RegPtr JitWasmCodeGen::readCPU(JitWidth w, RegPtr sib, U8 lsl, U32 offset, RegPtr res) {
    // Compute address: sib << lsl + offset, add to cpu_ptr
    if (!res) res = getTmpReg();
    U32 scratch = allocScratch();
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    pushRegValue(sib);
    if (lsl) {
        m_emitter.emitI32Const(lsl);
        m_emitter.emitOp(WASM_I32_SHL);
    }
    m_emitter.emitI32Const((S32)offset);
    m_emitter.emitOp(WASM_I32_ADD);
    m_emitter.emitOp(WASM_I32_ADD);
    m_emitter.emitI32Load(0);
    m_emitter.emitLocalSet(res->hardwareReg());
    freeScratch(scratch);
    return res;
}

void JitWasmCodeGen::writeCPU(JitWidth w, U32 offset, RegPtr src) {
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    pushRegValue(src);
    switch (w) {
    case JitWidth::b8:  m_emitter.emitI32Store8(offset);  break;
    case JitWidth::b16: m_emitter.emitI32Store16(offset); break;
    default:            m_emitter.emitI32Store(offset);    break;
    }
}

void JitWasmCodeGen::writeCPU(JitWidth w, RegPtr sib, U8 lsl, U32 offset, RegPtr src) {
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    pushRegValue(sib);
    if (lsl) {
        m_emitter.emitI32Const(lsl);
        m_emitter.emitOp(WASM_I32_SHL);
    }
    m_emitter.emitI32Const((S32)offset);
    m_emitter.emitOp(WASM_I32_ADD);
    m_emitter.emitOp(WASM_I32_ADD);
    pushRegValue(src);
    switch (w) {
    case JitWidth::b8:  m_emitter.emitI32Store8(0);  break;
    case JitWidth::b16: m_emitter.emitI32Store16(0); break;
    default:            m_emitter.emitI32Store(0);    break;
    }
}

void JitWasmCodeGen::writeCPUValue(JitWidth w, U32 offset, DYN_PTR_SIZE src) {
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitI32Const((S32)src);
    switch (w) {
    case JitWidth::b8:  m_emitter.emitI32Store8(offset);  break;
    case JitWidth::b16: m_emitter.emitI32Store16(offset); break;
    default:            m_emitter.emitI32Store(offset);    break;
    }
}

void JitWasmCodeGen::writeCPUValue(JitWidth w, RegPtr sib, U8 lsl, U32 offset, DYN_PTR_SIZE src) {
    auto tmp = getTmpReg();
    m_emitter.emitI32Const((S32)src);
    m_emitter.emitLocalSet(tmp->hardwareReg());
    writeCPU(w, sib, lsl, offset, tmp);
    freeScratch(tmp->hardwareReg());
}

// MMU read: get host page base pointer for emulated memory page index
void JitWasmCodeGen::readMMU(RegPtr dest, RegPtr index, U32 offset) {
    // Reads a 4-byte value at `index + offset` in emulated memory via the
    // dedicated memHelperAddr/memHelperValue scratch fields so that lazy
    // flag state survives the call.
    auto r = dest ? dest : getTmpReg();
    storeMemHelperField((U32)offsetof(CPU, memHelperAddr), index);
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitCall(HELPER_READ_MEM32);
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitI32Load((U32)offsetof(CPU, memHelperValue));
    m_emitter.emitLocalSet(r->hardwareReg());
}

void JitWasmCodeGen::readMMU(RegPtr dest, U32 index) {
    auto tmp = getTmpReg();
    m_emitter.emitI32Const((S32)index);
    m_emitter.emitLocalSet(tmp->hardwareReg());
    readMMU(dest, tmp, 0);
    freeScratch(tmp->hardwareReg());
}

// readHost/writeHost access WASM linear memory directly (host-side C++
// pointers live there). The WASM memarg offset is ADDED to the pushed
// address, so both the dynamic base (`rm`) and the static displacement
// (`offset`) must flow through correctly. Previously we passed 0 as the
// load/store offset, which silently discarded the MemPtr's offset field
// for rm-based addressing (e.g. readHost(op + offsetof(DecodedOp, next))).
void JitWasmCodeGen::readHost(JitWidth w, MemPtr address, RegPtr result, bool emulatedMemory) {
    if (!result) result = getTmpReg();
    U32 memOffset = 0;
    if (address->rm) {
        pushRegValue(address->rm);
        if (address->sib) {
            pushRegValue(address->sib);
            if (address->lsl) {
                m_emitter.emitI32Const((S32)address->lsl);
                m_emitter.emitOp(WASM_I32_SHL);
            }
            m_emitter.emitOp(WASM_I32_ADD);
        }
        memOffset = address->offset;
    } else {
        m_emitter.emitI32Const((S32)address->offset);
    }
    switch (w) {
    case JitWidth::b8:  m_emitter.emitI32Load8U(memOffset);  break;
    case JitWidth::b16: m_emitter.emitI32Load16U(memOffset); break;
    default:            m_emitter.emitI32Load(memOffset);     break;
    }
    m_emitter.emitLocalSet(result->hardwareReg());
}

void JitWasmCodeGen::writeHost(JitWidth w, MemPtr address, RegPtr src, bool emulatedMemory) {
    U32 memOffset = 0;
    if (address->rm) {
        pushRegValue(address->rm);
        if (address->sib) {
            pushRegValue(address->sib);
            if (address->lsl) {
                m_emitter.emitI32Const((S32)address->lsl);
                m_emitter.emitOp(WASM_I32_SHL);
            }
            m_emitter.emitOp(WASM_I32_ADD);
        }
        memOffset = address->offset;
    } else {
        m_emitter.emitI32Const((S32)address->offset);
    }
    pushRegValue(src);
    switch (w) {
    case JitWidth::b8:  m_emitter.emitI32Store8(memOffset);  break;
    case JitWidth::b16: m_emitter.emitI32Store16(memOffset); break;
    default:            m_emitter.emitI32Store(memOffset);    break;
    }
}

void JitWasmCodeGen::writeHost(JitWidth w, MemPtr address, U32 imm, bool emulatedMemory) {
    U32 memOffset = 0;
    if (address->rm) {
        pushRegValue(address->rm);
        if (address->sib) {
            pushRegValue(address->sib);
            if (address->lsl) {
                m_emitter.emitI32Const((S32)address->lsl);
                m_emitter.emitOp(WASM_I32_SHL);
            }
            m_emitter.emitOp(WASM_I32_ADD);
        }
        memOffset = address->offset;
    } else {
        m_emitter.emitI32Const((S32)address->offset);
    }
    m_emitter.emitI32Const((S32)imm);
    switch (w) {
    case JitWidth::b8:  m_emitter.emitI32Store8(memOffset);  break;
    case JitWidth::b16: m_emitter.emitI32Store16(memOffset); break;
    default:            m_emitter.emitI32Store(memOffset);    break;
    }
}

void JitWasmCodeGen::clearMMUPermissionIfSpansPage(JitWidth w, RegPtr offset, RegPtr reg) {
    // No-op for WASM; memory safety is handled by the WASM runtime itself.
}
void JitWasmCodeGen::clearIfSpansPage(JitWidth w, RegPtr offset, RegPtr reg) {
    // No-op for WASM.
}

// ---------------------------------------------------------------------------
// Emulated memory read/write (with softMMU)
//
// Plain reads and writes use an inline TLB fast path: load
// `wasmReadPageBase[addr>>12]` (or wasmWritePageBase for writes),
// branch on zero or width-aware boundary cross to the existing helper
// call, otherwise direct i32.load/store from `entry + (addr & MASK)`.
// CodePages have wasmWritePageBase==0 so writes that could hit JIT'd
// code always slow-path through the bailout-checking helper.
//
// The RMW path (readWriteMem, used by add/and/or/xchg/bts/btr/btc and
// shift-of-memory ops) still routes through the helpers — the
// scratch-pool budget for callbacks like btMask + nested IfTest is
// too tight to add an inline if/else without spilling into
// allocScratch's silent slot-0 fallback.
// ---------------------------------------------------------------------------
// Helper-import index for emulated-memory read/write of the given width.
// Picking the wrong width corrupts neighboring bytes: writeMem32 on a
// 16-bit push would clobber the two bytes above the stack slot.
static U32 readHelperForWidth(JitWidth w) {
    switch (w) {
    case JitWidth::b8:  return HELPER_READ_MEM8;
    case JitWidth::b16: return HELPER_READ_MEM16;
    default:            return HELPER_READ_MEM32;
    }
}
static U32 writeHelperForWidth(JitWidth w) {
    switch (w) {
    case JitWidth::b8:  return HELPER_WRITE_MEM8;
    case JitWidth::b16: return HELPER_WRITE_MEM16;
    default:            return HELPER_WRITE_MEM32;
    }
}

// Index of the bailout-checking write helper for a given width. The
// inline-RMW path (readWriteMem) and direct-write path (write) end with a
// helper call followed by an `if (cpu->wasmJitBailout) blockExit()` check
// — only when the write potentially lands on the active block's code page.
static U32 writeCheckHelperForWidth(JitWidth w) {
    switch (w) {
    case JitWidth::b8:  return HELPER_WRITE_MEM8_CHECK;
    case JitWidth::b16: return HELPER_WRITE_MEM16_CHECK;
    default:            return HELPER_WRITE_MEM32_CHECK;
    }
}

// Self-modifying-code bailout check: emitted after every JIT-inline mem
// write. If the write hit the active block's bytes, the helper set
// cpu->wasmJitBailout. We exit by writing the *next* op's offset to
// cpu->eip and calling blockExit so the dispatcher resumes there with a
// fresh decode.
//
// The if-body's blockExit() runs syncDirtyRegsToHost which compile-time-
// clears m_gpDirty[]; that's wrong for the no-bailout path where the
// body didn't run. Save/restore the tracker around the emit.
void JitWasmCodeGen::emitBailoutCheck() {
    auto savedGpDirty   = m_gpDirty;
    auto savedGpLoaded  = m_gpLoaded;
    auto savedSegLoaded = m_segLoaded;
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitI32Load((U32)offsetof(CPU, wasmJitBailout));
    m_emitter.emitIf();
    // currentEip is the *current* op's start (postCompile bumps it after
    // compile returns), so add op->len = lastCompiledOpLen to land at the
    // next op so the dispatcher resumes there.
    writeEip(this->currentEip + this->lastCompiledOpLen
             - cpu->seg[CS].address);
    blockExit();
    m_emitter.emitEnd();
    m_gpDirty   = savedGpDirty;
    m_gpLoaded  = savedGpLoaded;
    m_segLoaded = savedSegLoaded;
}

// Store `reg` into cpu->memHelperAddr (our dedicated mem-helper scratch).
// Also used for memHelperValue by varying the offset.
void JitWasmCodeGen::storeMemHelperField(U32 offset, RegPtr reg) {
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    pushRegValue(reg);
    m_emitter.emitI32Store(offset);
}

RegPtr JitWasmCodeGen::readWriteMem(JitWidth w, RegPtr addressReg,
                                     std::function<void(RegPtr)> prepareWrite, S8 hint) {
    auto result = getTmpReg();
    storeMemHelperField((U32)offsetof(CPU, memHelperAddr), addressReg);
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitCall(readHelperForWidth(w));
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitI32Load((U32)offsetof(CPU, memHelperValue));
    m_emitter.emitLocalSet(result->hardwareReg());
    if (prepareWrite) {
        prepareWrite(result);
        // The callback may have mutated addressReg's local or rewritten
        // memHelperAddr indirectly — re-stage it before dispatching write.
        storeMemHelperField((U32)offsetof(CPU, memHelperAddr), addressReg);
        storeMemHelperField((U32)offsetof(CPU, memHelperValue), result);
        m_emitter.emitLocalGet(WASM_CPU_LOCAL);
        // Use the bailout-checking write so self-modifying code that hits
        // the active block's bytes can be detected; emit the check.
        m_emitter.emitCall(writeCheckHelperForWidth(w));
        emitBailoutCheck();
    }
    return result;
}

// Inline TLB fast path for emulated reads. Compiles down to (in pseudo-WASM):
//
//   entry = wasmReadPageBase[addr >> 12]
//   if (entry == 0 || (addr & 0xfff) > 0x1000 - width) {
//       // slow: existing helper
//       memHelperAddr = addr; call read_helper(cpu); tmp = memHelperValue;
//   } else {
//       tmp = i32.load{8u,16u,_}(entry + addr)
//   }
//
// The boundary check is omitted for b8 (every byte is in some page).
// Save/restore the dirty/loaded trackers around the if since only one
// arm runs and `if`/`end` is treated as a branch boundary structurally
// even though neither branch invalidates the JIT register file.
RegPtr JitWasmCodeGen::read(JitWidth w, RegPtr addressReg,
                             std::function<void(MemPtr)> customOp,
                             std::function<void()> failedOp,
                             RegPtr tmp, bool checkAlignment) {
    if (!tmp) tmp = getTmpReg();
    if (customOp) {
        // Legacy path: customOp expects a MemPtr it can manipulate; the
        // fast path can't supply one, so always slow-path here.
        storeMemHelperField((U32)offsetof(CPU, memHelperAddr), addressReg);
        m_emitter.emitLocalGet(WASM_CPU_LOCAL);
        m_emitter.emitCall(readHelperForWidth(w));
        m_emitter.emitLocalGet(WASM_CPU_LOCAL);
        m_emitter.emitI32Load((U32)offsetof(CPU, memHelperValue));
        m_emitter.emitLocalSet(tmp->hardwareReg());
        return tmp;
    }

    U32 entryLocal = allocScratch();
    auto savedGpDirty   = m_gpDirty;
    auto savedGpLoaded  = m_gpLoaded;
    auto savedSegLoaded = m_segLoaded;

    // Push: missCondition (entry == 0 [|| boundary])
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitI32Load((U32)offsetof(CPU, wasmReadPageBaseArray));
    pushRegValue(addressReg);
    m_emitter.emitI32Const(K_PAGE_SHIFT);
    m_emitter.emitOp(WASM_I32_SHR_U);
    m_emitter.emitI32Const(2);                  // sizeof(U32)
    m_emitter.emitOp(WASM_I32_SHL);
    m_emitter.emitOp(WASM_I32_ADD);
    m_emitter.emitI32Load(0);                   // entry = wasmReadPageBase[page]
    m_emitter.emitLocalTee(entryLocal);
    m_emitter.emitOp(WASM_I32_EQZ);

    if (w != JitWidth::b8) {
        U32 widthBytes = (w == JitWidth::b16) ? 2 : 4;
        pushRegValue(addressReg);
        m_emitter.emitI32Const(K_PAGE_MASK);
        m_emitter.emitOp(WASM_I32_AND);
        m_emitter.emitI32Const((S32)(K_PAGE_SIZE - widthBytes));
        m_emitter.emitOp(WASM_I32_GT_U);
        m_emitter.emitOp(WASM_I32_OR);
    }

    m_emitter.emitIf();
    {
        // Slow path: existing helper-based load.
        storeMemHelperField((U32)offsetof(CPU, memHelperAddr), addressReg);
        m_emitter.emitLocalGet(WASM_CPU_LOCAL);
        m_emitter.emitCall(readHelperForWidth(w));
        m_emitter.emitLocalGet(WASM_CPU_LOCAL);
        m_emitter.emitI32Load((U32)offsetof(CPU, memHelperValue));
        m_emitter.emitLocalSet(tmp->hardwareReg());
    }
    m_emitter.emitElse();
    {
        // Fast path: tmp = i32.load{8u,16u,_}(entry + (addr & K_PAGE_MASK))
        // entry holds the raw host base of the page; mask the address to
        // get the in-page offset before adding (we don't use the
        // (host_base - virt_page_base) trick the host-exception readCache
        // does because under WASM the no-access sentinel can't be a low
        // address that traps).
        m_emitter.emitLocalGet(entryLocal);
        pushRegValue(addressReg);
        m_emitter.emitI32Const(K_PAGE_MASK);
        m_emitter.emitOp(WASM_I32_AND);
        m_emitter.emitOp(WASM_I32_ADD);
        switch (w) {
        case JitWidth::b8:  m_emitter.emitI32Load8U(0);  break;
        case JitWidth::b16: m_emitter.emitI32Load16U(0); break;
        default:            m_emitter.emitI32Load(0, 0); break; // align=0 (unaligned-safe)
        }
        m_emitter.emitLocalSet(tmp->hardwareReg());
    }
    m_emitter.emitEnd();

    m_gpDirty   = savedGpDirty;
    m_gpLoaded  = savedGpLoaded;
    m_segLoaded = savedSegLoaded;
    freeScratch(entryLocal);
    return tmp;
}

// Inline TLB fast path for emulated writes. Mirrors read():
//
//   entry = wasmWritePageBase[addr >> 12]
//   if (entry == 0 || (addr & 0xfff) > 0x1000 - width) {
//       slow: bailout-checking helper + emitBailoutCheck()
//   } else {
//       i32.store{8,16,}(entry + (addr & K_PAGE_MASK), src)
//   }
//
// Crucially, CodePage::canWriteRam returns false, so
// wasmWritePageBase[CodePage] == 0 → all writes to JIT'd code take the
// slow path, which preserves the SMC bailout. Plain RWPage writes can
// take the fast path because they can't be on an active block.
void JitWasmCodeGen::write(JitWidth w, RegPtr addressReg, RegPtr src,
                            std::function<void(MemPtr)> customOp,
                            std::function<void()> failedOp, bool checkAlignment) {
    if (customOp) {
        // Legacy path: customOp expects a MemPtr; slow-path only.
        storeMemHelperField((U32)offsetof(CPU, memHelperAddr), addressReg);
        storeMemHelperField((U32)offsetof(CPU, memHelperValue), src);
        m_emitter.emitLocalGet(WASM_CPU_LOCAL);
        m_emitter.emitCall(writeCheckHelperForWidth(w));
        emitBailoutCheck();
        return;
    }

    U32 entryLocal = allocScratch();
    auto savedGpDirty   = m_gpDirty;
    auto savedGpLoaded  = m_gpLoaded;
    auto savedSegLoaded = m_segLoaded;

    // missCondition = (entry == 0) [|| boundary]
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitI32Load((U32)offsetof(CPU, wasmWritePageBaseArray));
    pushRegValue(addressReg);
    m_emitter.emitI32Const(K_PAGE_SHIFT);
    m_emitter.emitOp(WASM_I32_SHR_U);
    m_emitter.emitI32Const(2);
    m_emitter.emitOp(WASM_I32_SHL);
    m_emitter.emitOp(WASM_I32_ADD);
    m_emitter.emitI32Load(0);
    m_emitter.emitLocalTee(entryLocal);
    m_emitter.emitOp(WASM_I32_EQZ);

    if (w != JitWidth::b8) {
        U32 widthBytes = (w == JitWidth::b16) ? 2 : 4;
        pushRegValue(addressReg);
        m_emitter.emitI32Const(K_PAGE_MASK);
        m_emitter.emitOp(WASM_I32_AND);
        m_emitter.emitI32Const((S32)(K_PAGE_SIZE - widthBytes));
        m_emitter.emitOp(WASM_I32_GT_U);
        m_emitter.emitOp(WASM_I32_OR);
    }

    m_emitter.emitIf();
    {
        // Slow path: bailout-checking helper. Writes to CodePages, RO
        // pages, on-demand pages, and cross-page accesses all land here.
        storeMemHelperField((U32)offsetof(CPU, memHelperAddr), addressReg);
        storeMemHelperField((U32)offsetof(CPU, memHelperValue), src);
        m_emitter.emitLocalGet(WASM_CPU_LOCAL);
        m_emitter.emitCall(writeCheckHelperForWidth(w));
        emitBailoutCheck();
    }
    m_emitter.emitElse();
    {
        // Fast path: i32.store{8,16,}(entry + (addr & K_PAGE_MASK), src)
        // No bailout check needed: by construction, fast-pathed writes
        // can't land on the active block (which lives on a CodePage,
        // and CodePage::canWriteRam() == false → wasmWritePageBase == 0).
        m_emitter.emitLocalGet(entryLocal);
        pushRegValue(addressReg);
        m_emitter.emitI32Const(K_PAGE_MASK);
        m_emitter.emitOp(WASM_I32_AND);
        m_emitter.emitOp(WASM_I32_ADD);
        pushRegValue(src);
        switch (w) {
        case JitWidth::b8:  m_emitter.emitI32Store8(0);  break;
        case JitWidth::b16: m_emitter.emitI32Store16(0); break;
        default:            m_emitter.emitI32Store(0, 0); break;
        }
    }
    m_emitter.emitEnd();

    m_gpDirty   = savedGpDirty;
    m_gpLoaded  = savedGpLoaded;
    m_segLoaded = savedSegLoaded;
    freeScratch(entryLocal);
}

// Materialize a MemPtr (base + index*scale + disp) into a scratch reg as a
// 32-bit virtual address. Only used for emulated-memory ops; DYN_PTR reads
// go through readHost which loads directly from WASM linear memory.
RegPtr JitWasmCodeGen::memPtrToAddressReg(MemPtr address) {
    auto addr = getTmpReg();
    if (address->rm) {
        pushRegValue(address->rm);
        if (address->sib) {
            pushRegValue(address->sib);
            if (address->lsl) {
                m_emitter.emitI32Const((S32)address->lsl);
                m_emitter.emitOp(WASM_I32_SHL);
            }
            m_emitter.emitOp(WASM_I32_ADD);
        }
        if (address->offset) {
            m_emitter.emitI32Const((S32)address->offset);
            m_emitter.emitOp(WASM_I32_ADD);
        }
    } else {
        m_emitter.emitI32Const((S32)address->offset);
    }
    m_emitter.emitLocalSet(addr->hardwareReg());
    return addr;
}

RegPtr JitWasmCodeGen::read(JitWidth w, MemPtr address, RegPtr result) {
    if (!address->emulatedAddress) {
        if (!result) result = getTmpReg();
        readHost(w, address, result, true);
        return result;
    }
    auto addr = memPtrToAddressReg(address);
    auto r = read(w, addr, nullptr, nullptr, result);
    freeScratch(addr->hardwareReg());
    return r;
}

void JitWasmCodeGen::write(JitWidth w, MemPtr address, RegPtr src) {
    if (!address->emulatedAddress) {
        writeHost(w, address, src, true);
        return;
    }
    auto addr = memPtrToAddressReg(address);
    write(w, addr, src);
    freeScratch(addr->hardwareReg());
}

void JitWasmCodeGen::write(JitWidth w, MemPtr address, U32 imm) {
    if (!address->emulatedAddress) {
        writeHost(w, address, imm, true);
        return;
    }
    auto addr = memPtrToAddressReg(address);
    auto val = getTmpReg();
    m_emitter.emitI32Const((S32)imm);
    m_emitter.emitLocalSet(val->hardwareReg());
    write(w, addr, val);
    freeScratch(val->hardwareReg());
    freeScratch(addr->hardwareReg());
}

// ---------------------------------------------------------------------------
// Arithmetic operations
// ---------------------------------------------------------------------------
void JitWasmCodeGen::addReg(JitWidth w, RegPtr reg, RegPtr rm)   { emitBinOp(w, reg, rm,  WASM_I32_ADD); }
void JitWasmCodeGen::addValue(JitWidth w, RegPtr reg, U32 imm)   { emitBinOpImm(w, reg, imm, WASM_I32_ADD); }
void JitWasmCodeGen::subReg(JitWidth w, RegPtr reg, RegPtr rm)   { emitBinOp(w, reg, rm,  WASM_I32_SUB); }
void JitWasmCodeGen::subValue(JitWidth w, RegPtr reg, U32 imm)   { emitBinOpImm(w, reg, imm, WASM_I32_SUB); }
void JitWasmCodeGen::orReg(JitWidth w, RegPtr reg, RegPtr rm)    { emitBinOp(w, reg, rm,  WASM_I32_OR); }
void JitWasmCodeGen::orValue(JitWidth w, RegPtr reg, U32 imm)    { emitBinOpImm(w, reg, imm, WASM_I32_OR); }
void JitWasmCodeGen::andReg(JitWidth w, RegPtr reg, RegPtr rm)   { emitBinOp(w, reg, rm,  WASM_I32_AND); }
void JitWasmCodeGen::andValue(JitWidth w, RegPtr reg, U32 imm)   { emitBinOpImm(w, reg, imm, WASM_I32_AND); }
void JitWasmCodeGen::xorReg(JitWidth w, RegPtr reg, RegPtr rm)   { emitBinOp(w, reg, rm,  WASM_I32_XOR); }
void JitWasmCodeGen::xorValue(JitWidth w, RegPtr reg, U32 imm)   { emitBinOpImm(w, reg, imm, WASM_I32_XOR); }
void JitWasmCodeGen::shlReg(JitWidth w, RegPtr reg, RegPtr rm)   { emitBinOp(w, reg, rm,  WASM_I32_SHL); }
void JitWasmCodeGen::shlValue(JitWidth w, RegPtr reg, U32 imm)   { emitBinOpImm(w, reg, imm, WASM_I32_SHL); }
void JitWasmCodeGen::shrReg(JitWidth w, RegPtr reg, RegPtr rm)   { emitBinOp(w, reg, rm,  WASM_I32_SHR_U); }
void JitWasmCodeGen::shrValue(JitWidth w, RegPtr reg, U32 imm)   { emitBinOpImm(w, reg, imm, WASM_I32_SHR_U); }
// Signed shift needs the value sign-extended from its width before SHR_S.
// emitBinOp masks the low byte/word with AND, which would zero the sign bit
// and turn a negative 8/16-bit into a positive 32-bit, making SHR_S wrong.
static void emitSignExtendTo32(WasmEmitter& e, JitWidth w) {
    if (w == JitWidth::b8) {
        e.emitI32Const(24); e.emitOp(WASM_I32_SHL);
        e.emitI32Const(24); e.emitOp(WASM_I32_SHR_S);
    } else if (w == JitWidth::b16) {
        e.emitI32Const(16); e.emitOp(WASM_I32_SHL);
        e.emitI32Const(16); e.emitOp(WASM_I32_SHR_S);
    }
}
void JitWasmCodeGen::sarReg(JitWidth w, RegPtr reg, RegPtr rm) {
    pushRegValue(reg);
    emitSignExtendTo32(m_emitter, w);
    pushRegValue(rm);
    if (w == JitWidth::b8 || w == JitWidth::b16) maskToWidth(w);
    m_emitter.emitOp(WASM_I32_SHR_S);
    maskToWidth(w);
    popToReg(w, reg);
}
void JitWasmCodeGen::sarValue(JitWidth w, RegPtr reg, U32 imm) {
    pushRegValue(reg);
    emitSignExtendTo32(m_emitter, w);
    m_emitter.emitI32Const((S32)imm);
    m_emitter.emitOp(WASM_I32_SHR_S);
    maskToWidth(w);
    popToReg(w, reg);
}
#ifdef BOXEDWINE_64
void JitWasmCodeGen::andValue64(RegPtr reg, U64 imm) { andValue(JitWidth::b32, reg, (U32)imm); }
#endif

void JitWasmCodeGen::notReg2(JitWidth w, RegPtr reg) {
    pushRegValue(reg);
    m_emitter.emitI32Const(-1);
    m_emitter.emitOp(WASM_I32_XOR);
    maskToWidth(w);
    popToReg(w, reg);
}

void JitWasmCodeGen::negReg2(JitWidth w, RegPtr reg) {
    m_emitter.emitI32Const(0);
    pushRegValue(reg);
    m_emitter.emitOp(WASM_I32_SUB);
    maskToWidth(w);
    popToReg(w, reg);
}

void JitWasmCodeGen::clzReg(JitWidth w, RegPtr result, RegPtr reg) {
    pushRegValue(reg);
    m_emitter.emitOp(WASM_I32_CLZ);
    m_emitter.emitLocalSet(result->hardwareReg());
}

// WASM i32.rotl/i32.rotr wrap at 32 bits; x86 8/16-bit rotates wrap at 8/16.
// For b32 the WASM semantics match exactly; fall back for narrower widths.
void JitWasmCodeGen::rolReg(JitWidth w, RegPtr reg, RegPtr rm) {
    if (w == JitWidth::b32) emitBinOp(w, reg, rm, WASM_I32_ROTL);
    else emulateSingleOp();
}
void JitWasmCodeGen::rolValue(JitWidth w, RegPtr reg, U32 imm) {
    if (w == JitWidth::b32) emitBinOpImm(w, reg, imm & 31, WASM_I32_ROTL);
    else emulateSingleOp();
}
void JitWasmCodeGen::rorReg(JitWidth w, RegPtr reg, RegPtr rm) {
    if (w == JitWidth::b32) emitBinOp(w, reg, rm, WASM_I32_ROTR);
    else emulateSingleOp();
}
void JitWasmCodeGen::rorValue(JitWidth w, RegPtr reg, U32 imm) {
    if (w == JitWidth::b32) emitBinOpImm(w, reg, imm & 31, WASM_I32_ROTR);
    else emulateSingleOp();
}

// Complex rotate-through-carry, shift double, mul/div: fall back to single-op emulation
void JitWasmCodeGen::rclReg(JitWidth w, RegPtr reg, RegPtr rm, RegPtr cf)    { emulateSingleOp(); }
void JitWasmCodeGen::rclValue(JitWidth w, RegPtr reg, U32 imm, RegPtr cf)    { emulateSingleOp(); }
void JitWasmCodeGen::rcrReg(JitWidth w, RegPtr reg, RegPtr rm, RegPtr cf)    { emulateSingleOp(); }
void JitWasmCodeGen::rcrValue(JitWidth w, RegPtr reg, U32 imm, RegPtr cf)    { emulateSingleOp(); }
void JitWasmCodeGen::shrdReg(JitWidth w, RegPtr reg, RegPtr rm, RegPtr cl)   { emulateSingleOp(); }
void JitWasmCodeGen::shrdValue(JitWidth w, RegPtr reg, RegPtr rm, U32 imm)   { emulateSingleOp(); }
void JitWasmCodeGen::shldReg(JitWidth w, RegPtr reg, RegPtr rm, RegPtr cl)   { emulateSingleOp(); }
void JitWasmCodeGen::shldValue(JitWidth w, RegPtr reg, RegPtr rm, U32 imm)   { emulateSingleOp(); }
void JitWasmCodeGen::mulReg(JitWidth w, RegPtr reg)                          { emulateSingleOp(); }
void JitWasmCodeGen::imulReg(JitWidth w, RegPtr reg)                         { emulateSingleOp(); }
void JitWasmCodeGen::imulRRI(JitWidth w, RegPtr dst, RegPtr src, U32 s2, RegPtr ov) {
    if (!ov) {
        // No overflow tracking: 32-bit multiply is sufficient.
        pushRegValue(src);
        m_emitter.emitI32Const((S32)s2);
        m_emitter.emitOp(WASM_I32_MUL);
        maskToWidth(w);
        popToReg(w, dst);
        return;
    }
    // With overflow (b32 only): sign-extend operands to i64, multiply, extract halves.
    pushRegValue(src);
    m_emitter.emitOp(WASM_I64_EXTEND_I32_S);
    m_emitter.emitI64Const((S64)(S32)s2);
    m_emitter.emitOp(WASM_I64_MUL);
    m_emitter.emitLocalTee(WASM_I64_SCRATCH);
    m_emitter.emitOp(WASM_I32_WRAP_I64);       // low 32 bits
    popToReg(w, dst);
    m_emitter.emitLocalGet(WASM_I64_SCRATCH);
    m_emitter.emitI64Const(32);
    m_emitter.emitOp(WASM_I64_SHR_S);
    m_emitter.emitOp(WASM_I32_WRAP_I64);       // high 32 bits
    popToReg(JitWidth::b32, ov);
}
void JitWasmCodeGen::imulRR(JitWidth w, RegPtr dst, RegPtr src, RegPtr ov) {
    if (!ov) {
        emitBinOp(w, dst, src, WASM_I32_MUL);
        return;
    }
    // With overflow: sign-extend operands to i64, multiply, extract halves.
    pushRegValue(dst);
    m_emitter.emitOp(WASM_I64_EXTEND_I32_S);
    pushRegValue(src);
    m_emitter.emitOp(WASM_I64_EXTEND_I32_S);
    m_emitter.emitOp(WASM_I64_MUL);
    m_emitter.emitLocalTee(WASM_I64_SCRATCH);
    m_emitter.emitOp(WASM_I32_WRAP_I64);       // low 32 bits
    popToReg(w, dst);
    m_emitter.emitLocalGet(WASM_I64_SCRATCH);
    m_emitter.emitI64Const(32);
    m_emitter.emitOp(WASM_I64_SHR_S);
    m_emitter.emitOp(WASM_I32_WRAP_I64);       // high 32 bits
    popToReg(JitWidth::b32, ov);
}
void JitWasmCodeGen::divRegRegWithRemainder(JitWidth w, RegPtr d, RegPtr dh, RegPtr s) { emulateSingleOp(); }
void JitWasmCodeGen::idivRegRegWithRemainder(JitWidth w, RegPtr d, RegPtr dh, RegPtr s) { emulateSingleOp(); }
void JitWasmCodeGen::bsfReg(JitWidth w, RegPtr reg, RegPtr rm)               { emulateSingleOp(); }
void JitWasmCodeGen::bsrReg(JitWidth w, RegPtr reg, RegPtr rm)               { emulateSingleOp(); }
void JitWasmCodeGen::absReg(JitWidth w, RegPtr reg)                          { emulateSingleOp(); }

void JitWasmCodeGen::byteSwapReg32(RegPtr reg) {
    // bswap: ((v & 0xff)<<24) | ((v & 0xff00)<<8) | ((v>>8)&0xff00) | (v>>24)
    // Fall back to single-op emulation for correctness.
    emulateSingleOp();
}

void JitWasmCodeGen::xchgReg(JitWidth w, RegPtr dest, RegPtr src) {
    U32 tmp = allocScratch();
    pushRegValue(dest);
    if (w == JitWidth::b8 || w == JitWidth::b16) maskToWidth(w);
    m_emitter.emitLocalSet(tmp);
    pushRegValue(src);
    if (w == JitWidth::b8 || w == JitWidth::b16) maskToWidth(w);
    popToReg(w, dest);
    m_emitter.emitLocalGet(tmp);
    popToReg(w, src);
    freeScratch(tmp);
}

void JitWasmCodeGen::xaddReg(JitWidth w, RegPtr reg, RegPtr rm) {
    // tmp = reg; reg = reg+rm; rm = tmp
    U32 tmp = allocScratch();
    pushRegValue(reg);
    if (w == JitWidth::b8 || w == JitWidth::b16) maskToWidth(w);
    m_emitter.emitLocalSet(tmp);
    addReg(w, reg, rm);
    m_emitter.emitLocalGet(tmp);
    popToReg(w, rm);
    freeScratch(tmp);
}

// ---------------------------------------------------------------------------
// Compare / test
// ---------------------------------------------------------------------------
RegPtr JitWasmCodeGen::compareReg(JitWidth w, RegPtr r1, RegPtr r2, JitEvaluate cond, RegPtr res) {
    if (!res) res = getTmpReg();
    pushRegValue(r1);
    if (w == JitWidth::b8 || w == JitWidth::b16) maskToWidth(w);
    pushRegValue(r2);
    if (w == JitWidth::b8 || w == JitWidth::b16) maskToWidth(w);
    switch (cond) {
    case JitEvaluate::EQUALS:                   m_emitter.emitOp(WASM_I32_EQ);   break;
    case JitEvaluate::NOT_EQUALS:               m_emitter.emitOp(WASM_I32_NE);   break;
    case JitEvaluate::LESS_THAN_UNSIGNED:       m_emitter.emitOp(WASM_I32_LT_U); break;
    case JitEvaluate::LESS_THAN_EQUAL_UNSIGNED: m_emitter.emitOp(WASM_I32_LE_U); break;
    case JitEvaluate::GREATER_THAN_UNSIGNED:    m_emitter.emitOp(WASM_I32_GT_U); break;
    case JitEvaluate::GREATER_THAN_EQUAL_UNSIGNED: m_emitter.emitOp(WASM_I32_GE_U); break;
    case JitEvaluate::LESS_THAN_SIGNED:         m_emitter.emitOp(WASM_I32_LT_S); break;
    case JitEvaluate::LESS_THAN_EQUAL_SIGNED:   m_emitter.emitOp(WASM_I32_LE_S); break;
    case JitEvaluate::GREATER_THAN_EQUAL_SIGNED:m_emitter.emitOp(WASM_I32_GE_S); break;
    case JitEvaluate::GREATER_THAN_SIGNED:      m_emitter.emitOp(WASM_I32_GT_S); break;
    default: m_emitter.emitOp(WASM_I32_EQ); break;
    }
    m_emitter.emitLocalSet(res->hardwareReg());
    return res;
}

RegPtr JitWasmCodeGen::compareValue(JitWidth w, RegPtr reg, U32 val, JitEvaluate cond, RegPtr res) {
    auto tmp = getTmpReg();
    m_emitter.emitI32Const((S32)val);
    m_emitter.emitLocalSet(tmp->hardwareReg());
    auto result = compareReg(w, reg, tmp, cond, res);
    freeScratch(tmp->hardwareReg());
    return result;
}

RegPtr JitWasmCodeGen::testZeroReg(JitWidth w, RegPtr reg, RegPtr res) {
    if (!res) res = getTmpReg();
    pushRegValue(reg);
    maskToWidth(w);
    m_emitter.emitOp(WASM_I32_EQZ);
    m_emitter.emitLocalSet(res->hardwareReg());
    return res;
}

// ---------------------------------------------------------------------------
// Move operations
// ---------------------------------------------------------------------------
void JitWasmCodeGen::mov(JitWidth w, RegPtr dest, RegPtr src) {
    pushRegValue(src);
    maskToWidth(w);
    popToReg(w, dest);
}

void JitWasmCodeGen::movzx(JitWidth dw, RegPtr dest, JitWidth sw, RegPtr src) {
    pushRegValue(src);
    maskToWidth(sw);
    // result is zero-extended to dw (source bits at low, upper dw bits are 0)
    popToReg(dw, dest);
}

void JitWasmCodeGen::movsx(JitWidth dw, RegPtr dest, JitWidth sw, RegPtr src) {
    pushRegValue(src);
    switch (sw) {
    case JitWidth::b8:  m_emitter.emitOp(WASM_I32_EXTEND8_S);  break;
    case JitWidth::b16: m_emitter.emitOp(WASM_I32_EXTEND16_S); break;
    default: break;
    }
    if (dw == JitWidth::b16) {
        // Mask to 16-bit for the merge in popToReg
        m_emitter.emitI32Const(0xFFFF);
        m_emitter.emitOp(WASM_I32_AND);
    }
    popToReg(dw, dest);
}

void JitWasmCodeGen::movValue(JitWidth w, RegPtr dst, DYN_PTR_SIZE imm) {
    m_emitter.emitI32Const((S32)imm);
    popToReg(w, dst);
}

// ---------------------------------------------------------------------------
// Flags – delegated to JitCodeGen base which stores to CPU struct fields
// (the generated WASM reads/writes these via readCPU/writeCPU which delegate
//  to emitI32Load/emitI32Store with the appropriate offsets)
// ---------------------------------------------------------------------------
void JitWasmCodeGen::storeLazyFlagType(LazyFlagType flags) {
    writeCPUValue(JitWidth::b8, (U32)offsetof(CPU, lazyFlagType), (DYN_PTR_SIZE)flags);
}
void JitWasmCodeGen::storeLazyFlagsDest(RegPtr reg) {
    // isHigh regs point at the high byte of reg[0..3]; pushRegValue already
    // shifts right 8 for them, so a b8 store at dst.u32's low byte is
    // correct (the lazy-flag code reads dst.u8). Non-high regs go in as b32.
    writeCPU(reg->isHigh ? JitWidth::b8 : JitWidth::b32,
             (U32)offsetof(CPU, dst.u32), reg);
}
void JitWasmCodeGen::storeLazyFlagsSrc(RegPtr reg) {
    writeCPU(reg->isHigh ? JitWidth::b8 : JitWidth::b32,
             (U32)offsetof(CPU, src.u32), reg);
}
void JitWasmCodeGen::storeLazyFlagsSrc(U32 value) {
    writeCPUValue(JitWidth::b32, (U32)offsetof(CPU, src.u32), value);
}
void JitWasmCodeGen::storeLazyFlagsResult(RegPtr reg) {
    writeCPU(reg->isHigh ? JitWidth::b8 : JitWidth::b32,
             (U32)offsetof(CPU, result.u32), reg);
}
void JitWasmCodeGen::storeLazyFlagsOldCF(RegPtr reg) {
    writeCPU(JitWidth::b32, (U32)offsetof(CPU, oldCF), reg);
}
void JitWasmCodeGen::fillFlags(U32 flags) {
    // Materialize lazy flags: call common_fillFlags(cpu).
    syncDirtyRegsToHost();
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitCall(HELPER_FILL_FLAGS);
    m_gpLoaded.fill(false);
    currentLazyFlags = FLAGS_NONE;
}
RegPtr JitWasmCodeGen::getZF() {
    // Sync dirty GP regs so helper sees correct CPU state, then call
    // wasmHelper_computeZF(cpu) which stores ZF in cpu->tmpReg.
    syncDirtyRegsToHost();
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitCall(HELPER_COMPUTE_ZF);
    auto r = getTmpReg();
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitI32Load((U32)offsetof(CPU, tmpReg));
    m_emitter.emitLocalSet(r->hardwareReg());
    // Any register values we had cached may have been invalidated if helper
    // touched state; invalidate and let callers re-load as needed.
    m_gpLoaded.fill(false);
    return r;
}
RegPtr JitWasmCodeGen::getCF() {
    syncDirtyRegsToHost();
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitCall(HELPER_COMPUTE_CF);
    auto r = getTmpReg();
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitI32Load((U32)offsetof(CPU, tmpReg));
    m_emitter.emitLocalSet(r->hardwareReg());
    m_gpLoaded.fill(false);
    return r;
}
void JitWasmCodeGen::orCPUFlags(RegPtr reg) {
    auto cur = readCPU(JitWidth::b32, (U32)offsetof(CPU, flags));
    orReg(JitWidth::b32, cur, reg);
    writeCPU(JitWidth::b32, (U32)offsetof(CPU, flags), cur);
}
void JitWasmCodeGen::xorCPUFlagsImmV2(U32 imm) {
    auto cur = readCPU(JitWidth::b32, (U32)offsetof(CPU, flags));
    xorValue(JitWidth::b32, cur, imm);
    writeCPU(JitWidth::b32, (U32)offsetof(CPU, flags), cur);
}
void JitWasmCodeGen::andCPUFlagsImmV2(U32 imm) {
    auto cur = readCPU(JitWidth::b32, (U32)offsetof(CPU, flags));
    andValue(JitWidth::b32, cur, imm);
    writeCPU(JitWidth::b32, (U32)offsetof(CPU, flags), cur);
}
void JitWasmCodeGen::orCPUFlagsImmV2(U32 imm) {
    auto cur = readCPU(JitWidth::b32, (U32)offsetof(CPU, flags));
    orValue(JitWidth::b32, cur, imm);
    writeCPU(JitWidth::b32, (U32)offsetof(CPU, flags), cur);
}
RegPtr JitWasmCodeGen::getReadOnlyFlags(RegPtr tmp) {
    return readCPU(JitWidth::b32, (U32)offsetof(CPU, flags), tmp);
}
RegPtr JitWasmCodeGen::getFlagsInTmp(RegPtr reg) {
    return readCPU(JitWidth::b32, (U32)offsetof(CPU, flags), reg);
}
void JitWasmCodeGen::setFlags(RegPtr flags, U32 mask) {
    // flags = (existing & ~mask) | (flags & mask)
    auto cur = readCPU(JitWidth::b32, (U32)offsetof(CPU, flags));
    andValue(JitWidth::b32, cur, ~mask);
    auto masked = getTmpReg();
    mov(JitWidth::b32, masked, flags);
    andValue(JitWidth::b32, masked, mask);
    orReg(JitWidth::b32, cur, masked);
    writeCPU(JitWidth::b32, (U32)offsetof(CPU, flags), cur);
    freeScratch(masked->hardwareReg());
}
void JitWasmCodeGen::writeFlags(RegPtr flags) {
    writeCPU(JitWidth::b32, (U32)offsetof(CPU, flags), flags);
}
RegPtr JitWasmCodeGen::getCondition(JitConditional cond, RegPtr res) {
    // Per-condition helper stashes 0/1 in cpu->tmpReg. Done this way (rather
    // than one helper taking a condition parameter) because our helper
    // import signature is (i32)->() — a condition arg would have to go
    // through a CPU scratch field, and every candidate (src.u32, dst.u32,
    // etc.) is already reserved for lazy-flag state.
    syncDirtyRegsToHost();
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitCall(HELPER_COND_BASE + (U32)cond);

    // Always load the helper result into a scratch first. `res` may be a
    // GP register; writing its local directly via emitLocalSet would
    // clobber the upper bytes. If the caller gave us a `res`, copy via
    // mov(b8) which merges correctly for GP regs and scratch alike.
    auto tmp = getTmpReg();
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitI32Load((U32)offsetof(CPU, tmpReg));
    m_emitter.emitLocalSet(tmp->hardwareReg());
    m_gpLoaded.fill(false);
    if (res && res != tmp) {
        mov(JitWidth::b8, res, tmp);
        freeScratch(tmp->hardwareReg());
        return res;
    }
    return tmp;
}
RegPtr JitWasmCodeGen::getConditionCalculationReg(U32 index) {
    return readCPU(JitWidth::b32, (U32)offsetof(CPU, flags));
}

// ---------------------------------------------------------------------------
// Control flow (If/Else/End/Goto)
// ---------------------------------------------------------------------------
void JitWasmCodeGen::finishIf() {
    m_emitter.emitIf();
}

void JitWasmCodeGen::branchBoundary() {
    syncDirtyRegsToHost();
    m_gpLoaded.fill(false);
    m_segLoaded.fill(false);
}

void JitWasmCodeGen::If(JitWidth w, RegPtr reg) {
    branchBoundary();
    pushRegValue(reg);
    maskToWidth(w);
    finishIf();
}
void JitWasmCodeGen::IfNot(JitWidth w, RegPtr reg) {
    branchBoundary();
    pushRegValue(reg);
    maskToWidth(w);
    m_emitter.emitOp(WASM_I32_EQZ);
    finishIf();
}
void JitWasmCodeGen::IfTest(JitWidth w, RegPtr reg, RegPtr mask) {
    branchBoundary();
    pushRegValue(reg);
    pushRegValue(mask);
    m_emitter.emitOp(WASM_I32_AND);
    finishIf();
}
void JitWasmCodeGen::IfTest(JitWidth w, RegPtr reg, U32 mask) {
    branchBoundary();
    pushRegValue(reg);
    m_emitter.emitI32Const((S32)mask);
    m_emitter.emitOp(WASM_I32_AND);
    finishIf();
}
void JitWasmCodeGen::IfNotTestBit(JitWidth w, RegPtr reg, U32 bitPos) {
    branchBoundary();
    pushRegValue(reg);
    m_emitter.emitI32Const((S32)(1u << bitPos));
    m_emitter.emitOp(WASM_I32_AND);
    m_emitter.emitOp(WASM_I32_EQZ);
    finishIf();
}
void JitWasmCodeGen::IfTestBit(JitWidth w, RegPtr reg, U32 bitPos) {
    branchBoundary();
    pushRegValue(reg);
    m_emitter.emitI32Const((S32)(1u << bitPos));
    m_emitter.emitOp(WASM_I32_AND);
    finishIf();
}
void JitWasmCodeGen::IfEqual(JitWidth w, RegPtr reg, DYN_PTR_SIZE value) {
    branchBoundary();
    pushRegValue(reg);
    m_emitter.emitI32Const((S32)value);
    m_emitter.emitOp(WASM_I32_EQ);
    finishIf();
}
void JitWasmCodeGen::IfEqual(JitWidth w, RegPtr r1, RegPtr r2) {
    branchBoundary();
    pushRegValue(r1); pushRegValue(r2);
    m_emitter.emitOp(WASM_I32_EQ);
    finishIf();
}
void JitWasmCodeGen::IfNotEqual(JitWidth w, RegPtr reg, DYN_PTR_SIZE value) {
    branchBoundary();
    pushRegValue(reg);
    m_emitter.emitI32Const((S32)value);
    m_emitter.emitOp(WASM_I32_NE);
    finishIf();
}
void JitWasmCodeGen::IfNotEqual(JitWidth w, RegPtr reg, RegPtr r2) {
    branchBoundary();
    pushRegValue(reg); pushRegValue(r2);
    m_emitter.emitOp(WASM_I32_NE);
    finishIf();
}
void JitWasmCodeGen::IfLessThan2(JitWidth w, RegPtr reg, U32 value) {
    branchBoundary();
    pushRegValue(reg);
    m_emitter.emitI32Const((S32)value);
    m_emitter.emitOp(WASM_I32_LT_U);
    finishIf();
}
void JitWasmCodeGen::IfLessThan2(JitWidth w, RegPtr r1, RegPtr r2) {
    branchBoundary();
    pushRegValue(r1); pushRegValue(r2);
    m_emitter.emitOp(WASM_I32_LT_U);
    finishIf();
}
void JitWasmCodeGen::IfGreaterThanOrEqual(JitWidth w, RegPtr r1, RegPtr r2) {
    branchBoundary();
    pushRegValue(r1); pushRegValue(r2);
    m_emitter.emitOp(WASM_I32_GE_U);
    finishIf();
}
void JitWasmCodeGen::IfGreaterThanOrEqual(JitWidth w, RegPtr reg, U32 value) {
    branchBoundary();
    pushRegValue(reg);
    m_emitter.emitI32Const((S32)value);
    m_emitter.emitOp(WASM_I32_GE_U);
    finishIf();
}
void JitWasmCodeGen::IfNotCPU(JitWidth w, RegPtr sib, U8 lsl, U32 offset) {
    // read from CPU struct and If-not
    branchBoundary();
    auto tmp = readCPU(w, sib, lsl, offset);
    pushRegValue(tmp);
    maskToWidth(w);
    m_emitter.emitOp(WASM_I32_EQZ);
    finishIf();
    freeScratch(tmp->hardwareReg());
}
void JitWasmCodeGen::IfCondition(JitConditional cond) {
    // Base JitCodeGen handles: it calls getCondition and emits the right check
    auto r = getCondition(cond);
    If(JitWidth::b32, r);
    // getCondition allocates a scratch local; If() consumes its value on the
    // WASM stack, so we can release the scratch now. Without this, nested
    // codegen (dynamic_loopnz / dynamic_loopz) exhausts the 8-slot pool.
    freeScratch(r->hardwareReg());
}
void JitWasmCodeGen::JumpIfCondition(JitConditional cond, U32 address) {
    // Called when canJumpInBlock() is true. Native backends do a direct
    // branch inside the generated code; we don't have structural
    // intra-block jumps, so treat it as a block exit to the target. The
    // `address` parameter is the *linear* EIP (CS.address + offset), but
    // cpu->eip.u32 stores only the offset relative to CS — subtract here.
    IfCondition(cond);
    writeEip(address - cpu->seg[CS].address);
    blockExit();
    EndIf();
}
void JitWasmCodeGen::IfDF() {
    branchBoundary();
    auto cur = getReadOnlyFlags();
    m_emitter.emitLocalGet(cur->hardwareReg());
    m_emitter.emitI32Const(DF);
    m_emitter.emitOp(WASM_I32_AND);
    finishIf();
}
void JitWasmCodeGen::IfSmallStack() {
    // Small-stack mode is indicated by a non-zero cpu->stackNotMask. Loaded
    // inline rather than via readCPU()+If(reg) so branchBoundary runs once.
    branchBoundary();
    U32 scratch = allocScratch();
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitI32Load((U32)offsetof(CPU, stackNotMask));
    m_emitter.emitLocalSet(scratch);
    m_emitter.emitLocalGet(scratch);
    finishIf();
    freeScratch(scratch);
}
void JitWasmCodeGen::StartElse() {
    branchBoundary();
    m_emitter.emitElse();
}
void JitWasmCodeGen::EndIf() {
    branchBoundary();
    m_emitter.emitEnd();
}
void JitWasmCodeGen::JumpInBlock(U32 address) {
    // No native intra-block branching: set EIP (offset within CS, so strip
    // the CS base) and exit. fetchNextOp picks back up at the target op
    // in the next dispatcher round.
    writeEip(address - cpu->seg[CS].address);
    syncDirtyRegsToHost();
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitCall(m_helperGetNextOpIdx);
    m_emitter.emitReturn();
}
void JitWasmCodeGen::blockExit() {
    syncDirtyRegsToHost();
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitCall(m_helperGetNextOpIdx);
    m_emitter.emitReturn();
}
// blockNext1/2 are block-terminating branch destinations (taken vs. fall-
// through). Mirror the base JitCodeGen::blockNext{1,2}: write the CS-offset
// EIP and emit a blockExit so the dispatcher picks up the next op.
void JitWasmCodeGen::blockNext1(U32 eip, DecodedOp* op) {
    writeCPUValue(DYN_PTR, (U32)offsetof(CPU, nextOp), 0);
    writeEip(eip - cpu->seg[CS].address);
    blockExit();
}
void JitWasmCodeGen::blockNext2(U32 eip, DecodedOp* op) {
    blockNext1(eip, op);
}
void JitWasmCodeGen::jumpEip(RegPtr reg) {
    writeEip(reg);
    blockExit();
}
bool JitWasmCodeGen::canJumpInBlock(DecodedOp* op) {
    return JitCodeGen::canJumpInBlock(op);
}
bool JitWasmCodeGen::canJumpInBlock(U32 opEip, DecodedOp* op) {
    return JitCodeGen::canJumpInBlock(opEip, op);
}
void JitWasmCodeGen::onTestEnd(DecodedOp* op) {
    syncDirtyRegsToHost();
    // Set cpu->nextOp = op (the TestEnd op)
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitI32Const((S32)(uintptr_t)op);
    m_emitter.emitI32Store((U32)offsetof(CPU, nextOp));
}
void JitWasmCodeGen::jmpHost(RegPtr reg)          { /* no native jumps in WASM */ }
void JitWasmCodeGen::jmpHost(DYN_PTR_SIZE address) { /* no native jumps in WASM */ }
void JitWasmCodeGen::nakedCall(RegPtr reg)         { /* no bare calls in WASM */ }
void JitWasmCodeGen::nakedReturn()                 { m_emitter.emitReturn(); }

// ---------------------------------------------------------------------------
// Function calls to C++ helpers
// ---------------------------------------------------------------------------
void JitWasmCodeGen::callHostFunction(void* address, const std::vector<DynParam>& params,
                                       bool restoreCache, bool saveCache) {
    // For WASM: call via function table index.
    // The function address IS a wasmTable index for C++ functions in Emscripten.
    syncDirtyRegsToHost();
    // Pass cpu as first argument (all our helpers take CPU*)
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitCallIndirect(m_typeVoidI32, 0);
    // Re-load GP regs if needed after call
    m_gpLoaded.fill(false);
}

void JitWasmCodeGen::callHostFunctionWithResult(RegPtr result, void* address,
                                                  const std::vector<DynParam>& params) {
    callHostFunction(address, params, true, true);
    // read result from cpu->dst.u32
    if (result) {
        m_emitter.emitLocalGet(WASM_CPU_LOCAL);
        m_emitter.emitI32Load((U32)offsetof(CPU, dst.u32));
        m_emitter.emitLocalSet(result->hardwareReg());
    }
}


void JitWasmCodeGen::emulateSingleOp() {
    // Sync state, point cpu->eip.u32 at this op's offset (only when we may
    // be in a mixed JIT/emulate block — for pure-emulate blocks each
    // helper's NEXT() advances eip naturally and no writeEip is needed),
    // call runNextSingleOp, reload.
    //
    // Mixed blocks (an inline JIT op followed by emulateSingleOp) need this:
    // the JIT op doesn't update cpu->eip, so the helper would re-decode at
    // the previous emulated op's eip and run the wrong instruction.
    //
    // We can't easily know at compile time whether an inline op preceded
    // this one (the previous op may have used emulateSingleOp), so emit
    // writeEip when the current op is *not* the first op in the block.
    // When it's the first op, cpu->eip was set by the dispatcher.
    if (this->currentEip != this->startingEip) {
        writeEip(this->currentEip - cpu->seg[CS].address);
    }
    syncDirtyRegsToHost();
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitCall(m_helperEmulateSingleOpIdx);
    m_gpLoaded.fill(false);
    m_segLoaded.fill(false);
}

// ---------------------------------------------------------------------------
// Direct operations (cmp/test/jmp for JIT optimization paths)
// ---------------------------------------------------------------------------
// Stage a JitWidth::bN store of dst/src/result into cpu->{dst,src,result}.u32
// plus cpu->lazyFlagType. The helper-based condition accessors read these to
// compute the flag, so direct-mode needs to populate them even though it
// normally would skip lazy-flag storage.
static LazyFlagType lazyTypeForSub(JitWidth w) {
    switch (w) {
    case JitWidth::b8:  return FLAGS_SUB8;
    case JitWidth::b16: return FLAGS_SUB16;
    default:            return FLAGS_SUB32;
    }
}
static LazyFlagType lazyTypeForTest(JitWidth w) {
    switch (w) {
    case JitWidth::b8:  return FLAGS_TEST8;
    case JitWidth::b16: return FLAGS_TEST16;
    default:            return FLAGS_TEST32;
    }
}
void JitWasmCodeGen::direct_cmp(JitWidth w, RegPtr left, RegPtr right) {
    storeLazyFlagsDest(left);
    storeLazyFlagsSrc(right);
    auto result = getTmpReg();
    pushRegValue(left);
    pushRegValue(right);
    m_emitter.emitOp(WASM_I32_SUB);
    m_emitter.emitLocalSet(result->hardwareReg());
    storeLazyFlagsResult(result);
    storeLazyFlagType(lazyTypeForSub(w));
    freeScratch(result->hardwareReg());
    currentLazyFlags = lazyTypeForSub(w);
}
void JitWasmCodeGen::direct_cmp(JitWidth w, RegPtr left, U32 right) {
    storeLazyFlagsDest(left);
    storeLazyFlagsSrc(right);
    auto result = getTmpReg();
    pushRegValue(left);
    m_emitter.emitI32Const((S32)right);
    m_emitter.emitOp(WASM_I32_SUB);
    m_emitter.emitLocalSet(result->hardwareReg());
    storeLazyFlagsResult(result);
    storeLazyFlagType(lazyTypeForSub(w));
    freeScratch(result->hardwareReg());
    currentLazyFlags = lazyTypeForSub(w);
}
void JitWasmCodeGen::direct_test(JitWidth w, RegPtr left, RegPtr right) {
    auto result = getTmpReg();
    pushRegValue(left);
    pushRegValue(right);
    m_emitter.emitOp(WASM_I32_AND);
    m_emitter.emitLocalSet(result->hardwareReg());
    storeLazyFlagsResult(result);
    storeLazyFlagType(lazyTypeForTest(w));
    freeScratch(result->hardwareReg());
    currentLazyFlags = lazyTypeForTest(w);
}
void JitWasmCodeGen::direct_test(JitWidth w, RegPtr left, U32 right) {
    auto result = getTmpReg();
    pushRegValue(left);
    m_emitter.emitI32Const((S32)right);
    m_emitter.emitOp(WASM_I32_AND);
    m_emitter.emitLocalSet(result->hardwareReg());
    storeLazyFlagsResult(result);
    storeLazyFlagType(lazyTypeForTest(w));
    freeScratch(result->hardwareReg());
    currentLazyFlags = lazyTypeForTest(w);
}
void JitWasmCodeGen::direct_jump(JitConditional cond, U32 address) {
    JumpIfCondition(cond, address);
}
void JitWasmCodeGen::direct_cmov(JitWidth w, JitConditional cond, RegPtr dst, RegPtr src) {
    IfCondition(cond);
    mov(w, dst, src);
    EndIf();
}
void JitWasmCodeGen::direct_setcc(JitConditional cond, RegPtr dst) {
    auto r = getCondition(cond, dst);
    if (r != dst) mov(JitWidth::b8, dst, r);
}
bool JitWasmCodeGen::directDoesAffectFlags(DecodedOp* op) { return false; }


// ---------------------------------------------------------------------------
// Code management (buffer tracking for branch patching)
// For WASM: we don't do binary patching; use structural control flow.
// These are stubs required by the JitCodeGen interface.
// ---------------------------------------------------------------------------
U32  JitWasmCodeGen::getBufferSize()             { return (U32)m_patchBuffer.size(); }
U32  JitWasmCodeGen::markBufferLocation()        { U32 id = (U32)m_patchLocations.size(); m_patchLocations.push_back((U32)m_patchBuffer.size()); return id; }
U32  JitWasmCodeGen::getBufferLocation(U32 id)   { return id < m_patchLocations.size() ? m_patchLocations[id] : 0; }
void JitWasmCodeGen::copyBuffer(U8* dst, U32 sz) { /* WASM binary managed separately */ }
// Base postCompile checks getIfJumpSize() == 0 to verify all `If`s emitted
// during the op had matching `EndIf`s (in native JITs the size of pending
// branch fixups is non-zero while an If is open). WASM uses structural
// control flow (`if … end`) where the validator catches mismatches itself,
// so we always report 0 here.
U32  JitWasmCodeGen::getIfJumpSize()             { return 0; }

// MarkJumpLocation / Goto: used by JitCodeGen for intra-block branch patching
// in the x86/ARM backends. WASM uses structural control flow, so for
// MarkJumpLocation/Goto specifically there is no patching to do — the loops
// that actually need backward branches use LoopBegin/Goto/LoopEnd instead
// (see the override of those below). Goto remains generic: it dispatches on
// whether `location` is a real loop token (encoded by LoopBegin) or a
// MarkJumpLocation buffer offset (in which case nothing to do).
//
// To distinguish, LoopBegin returns a token tagged with the high bit set
// (kLoopTokenTag); MarkJumpLocation's buffer-offset return value never sets
// it because compiled bodies are far smaller than 2 GiB.
static constexpr U32 kLoopTokenTag = 0x80000000u;

U32 JitWasmCodeGen::MarkJumpLocation() { return markBufferLocation(); }

void JitWasmCodeGen::Goto(U32 location) {
    if ((location & kLoopTokenTag) == 0) {
        // MarkJumpLocation token — x86/ARM patching path; nothing to do.
        return;
    }
    U32 loopDepth = location & ~kLoopTokenTag;
    U32 currentDepth = m_emitter.currentCtrlDepth();
    // The loop frame sits `currentDepth - loopDepth` levels above the
    // current innermost frame. `br N` targets that frame's label, which
    // for a `loop` is the *top* of the loop — i.e., a backward branch.
    branchBoundary();
    m_emitter.emitBr(currentDepth - loopDepth);
}

U32 JitWasmCodeGen::LoopBegin() {
    // Emit `loop` and capture the depth right after it's pushed. The
    // returned token encodes the loop frame's depth so Goto can compute
    // a relative `br` argument from wherever it's called inside the body.
    branchBoundary();
    m_emitter.emitLoop();
    U32 depth = m_emitter.currentCtrlDepth();
    return kLoopTokenTag | depth;
}

void JitWasmCodeGen::LoopEnd() {
    branchBoundary();
    m_emitter.emitEnd();
}

// ---------------------------------------------------------------------------
// Module instantiation hooks
// ---------------------------------------------------------------------------
U8* JitWasmCodeGen::createSyncToHost()    { return nullptr; } // handled inline
U8* JitWasmCodeGen::createSyncFromHost()  { return nullptr; }
U8* JitWasmCodeGen::createBlockExit()     { return nullptr; }
// doJIT panics if calculateCF[0] == nullptr. createCalculationCF (non-virtual)
// calls createDynamicExecutableMemory, so returning a non-null stub here
// satisfies the check. The pointer is never actually called from WASM since
// nakedCall() is a no-op in this backend.
static void wasmDummyCode(CPU*) {}
U8* JitWasmCodeGen::createDynamicExecutableMemory(U32* pSize) {
    return (U8*)(void*)wasmDummyCode;
}

U8* JitWasmCodeGen::createStartJITCode() {
    // Return the address of the static wasmStartJITOp function.
    // NormalCPU::run() uses this as process->startJITOp.
    return (U8*)wasmStartJITOp;
}

// ---------------------------------------------------------------------------
// Compilation lifecycle
// ---------------------------------------------------------------------------
void JitWasmCodeGen::preCompile(DecodedOp* op, bool skippedOp) {
    // Reset per-instruction scratch state, then run base bookkeeping (block
    // op count, eip→buffer-pos map, preOp hook). Stash op->len for
    // emitBailoutCheck — it needs the post-write next-op EIP.
    m_scratchInUse.fill(false);
    lastCompiledOpLen = op->len;
    JitCodeGen::preCompile(op, skippedOp);
}

void JitWasmCodeGen::compile(DecodedOp* op) {
    JitCodeGen::compile(op); // delegates to dynamic_* dispatch
}

void JitWasmCodeGen::postCompile(DecodedOp* op) {
    // Critical: base class advances currentEip by op->len. Without delegating,
    // every branch op compiled later in the block computes its target from a
    // stale block-start currentEip, not the op's own position. That made
    // CallJw with imm=0x123 land at start+0x123 instead of start+0x126.
    JitCodeGen::postCompile(op);
}

void JitWasmCodeGen::commitJIT(DecodedOp* op) {
    // Finalize the WASM function body and the module binary.
    m_emitter.endFunction();
    m_wasmBinary = m_emitter.finalize();

    if (m_wasmBinary.empty()) return;

    // Pass the helper function table to the JS instantiator.
    // Under multiThreadedJit, serialize wasmTable mutations across workers.
#ifdef BOXEDWINE_MULTI_THREADED
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(g_wasmJitTableMutex);
#endif
    int tableIdx = boxedwine_wasm_instantiate(
        m_wasmBinary.data(),
        (int)m_wasmBinary.size(),
        g_wasmHelperTable,
        WASM_HELPER_COUNT
    );

    if (tableIdx < 0) {
        // Instantiation failed — fall back to the normal CPU interpreter
        // for every op in the block. Without this, op->pfn stays as
        // firstDynamicOp, which would recompile-then-call-self every time
        // the op runs → infinite loop.
        DecodedOp* cur = op;
        while (cur) {
            cur->pfn = NormalCPU::getFunctionForOp(cur);
            cur->pfnJitCode = nullptr;
            if (cur->next == nullptr) break;
            cur = cur->next;
        }
        return;
    }

    // Walk all ops in this block: the first op gets the startJITOp entry
    // (its pfnJitCode is the wasmTable index the dispatcher invokes). All
    // ops in the block share the same tableIdx so removeCodeBlock's sweep
    // picks them up for cleanup; they also get OP_FLAG_JIT so doJIT skips
    // recompiling. Only the first op's pfn points at startJITOp — control
    // never re-enters the middle of a WASM block.
    op->blockLen     = this->emulatedLen;
    op->blockOpCount = this->blockOpCount;
    DecodedOp* cur = op;
    while (cur) {
        cur->pfnJitCode = (void*)(uintptr_t)(U32)tableIdx;
        cur->flags     |= OP_FLAG_JIT;
        cur->blockStart = op;
        if (cur == op) {
            cur->pfn = cpu->thread->process->startJITOp;
        } else if (cur->pfn == cpu->thread->process->startJITOp) {
            // This interior op was previously the first op of another compiled
            // block. Its pfnJitCode has now been overwritten to this block's
            // tableIdx, so calling startJITOp here would run the wrong WASM
            // block from its beginning with incorrect CPU state.  Reset to the
            // interpreter function so direct entry at this op falls back to
            // the normal interpreter path.
            cur->pfn = NormalCPU::getFunctionForOp(cur);
        }
        if (cur->next == nullptr) break;
        cur = cur->next;
    }
}

// ---------------------------------------------------------------------------
// Factory function: create a new WASM JIT backend instance.
// This is called by the shared JIT infrastructure in jitCodeGen.cpp.
// ---------------------------------------------------------------------------
JitCodeGen* startNewJIT(CPU* cpu) {
    return new JitWasmCodeGen(cpu);
}

void startNewJIT(CPU* cpu, U32 address, DecodedOp* op) {
    JitCodeGen* jit = startNewJIT(cpu);
    jit->doJIT(address, op);
    delete jit;
}

// ---------------------------------------------------------------------------
// clearJitBlock: called by kmemory when evicting JIT blocks.
// For WASM, release the compiled function from wasmTable.
// ---------------------------------------------------------------------------
void clearJitBlock(const std::vector<void*>& jitOps) {
    // All ops in a WASM-compiled block share the same wasmTable index
    // (we don't sub-compile individual ops), so dedupe before freeing.
    std::set<int> seen;
    for (void* p : jitOps) {
        if (!p) continue;
        int tableIdx = (int)(uintptr_t)p;
        if (seen.insert(tableIdx).second) {
#ifdef BOXEDWINE_MULTI_THREADED
            BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(g_wasmJitTableMutex);
#endif
            boxedwine_wasm_free_block(tableIdx);
        }
    }
}

#endif // BOXEDWINE_WASM_JIT
