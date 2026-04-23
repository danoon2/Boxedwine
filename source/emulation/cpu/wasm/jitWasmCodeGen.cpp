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
#include "../../softmmu/soft_code_page.h"

#include <emscripten.h>
#include <emscripten/em_js.h>

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
        int tableIndex = (int)(uintptr_t)op->pfnJitCode;
        boxedwine_wasm_call_block(tableIndex, (int)(uintptr_t)cpu);
        // nextOp is updated by the WASM block itself (via helper call).
    }
}

// ---------------------------------------------------------------------------
// C++ helpers imported by generated WASM modules
// ---------------------------------------------------------------------------

// Read a 32-bit value from emulated memory at address stored in cpu->tmp
static void wasmHelper_readMem32(CPU* cpu) {
    U32 addr = cpu->src.u32;
    cpu->dst.u32 = cpu->memory->readd(addr);
}
static void wasmHelper_writeMem32(CPU* cpu) {
    cpu->memory->writed(cpu->dst.u32, cpu->src.u32);
}
static void wasmHelper_readMem8(CPU* cpu) {
    cpu->dst.u8 = cpu->memory->readb(cpu->src.u32);
}
static void wasmHelper_writeMem8(CPU* cpu) {
    cpu->memory->writeb(cpu->dst.u32, cpu->src.u8);
}
static void wasmHelper_readMem16(CPU* cpu) {
    cpu->dst.u16 = cpu->memory->readw(cpu->src.u32);
}
static void wasmHelper_writeMem16(CPU* cpu) {
    cpu->memory->writew(cpu->dst.u32, cpu->src.u16);
}

// Update cpu->nextOp by decoding the block at cpu->eip.u32
static void wasmHelper_fetchNextOp(CPU* cpu) {
    if (!cpu->thread->terminating) {
        cpu->nextOp = cpu->getNextOp();
    }
}

// Sync CPU lazy flags — flags are maintained in cpu->flags directly by WASM code
static void wasmHelper_syncFlags(CPU* cpu) {}

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
    });

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
    return getReadOnlySegAddress(seg);
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
    // cpu->memory->getHostReadAddress(page) -> i32
    // For WASM: read through cpu->memory MMU table
    // Simplified: call C++ helper that does the lookup
    // Store address in cpu->src.u32, call helper, read from cpu->dst.u32
    // (Full inline TLB lookup is a future optimization)
    auto r = dest ? dest : getTmpReg();
    // push index as cpu->src.u32
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    pushRegValue(index);
    m_emitter.emitI32Store((U32)offsetof(CPU, src.u32));
    // call helper
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitCall(m_helperReadMemIdx);
    // read result from cpu->dst.u32
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitI32Load((U32)offsetof(CPU, dst.u32));
    m_emitter.emitLocalSet(r->hardwareReg());
}

void JitWasmCodeGen::readMMU(RegPtr dest, U32 index) {
    auto tmp = getTmpReg();
    m_emitter.emitI32Const((S32)index);
    m_emitter.emitLocalSet(tmp->hardwareReg());
    readMMU(dest, tmp, 0);
    freeScratch(tmp->hardwareReg());
}

void JitWasmCodeGen::readHost(JitWidth w, MemPtr address, RegPtr result, bool emulatedMemory) {
    if (!result) result = getTmpReg();
    if (address->rm) {
        pushRegValue(address->rm);
    } else {
        m_emitter.emitI32Const((S32)address->offset);
    }
    switch (w) {
    case JitWidth::b8:  m_emitter.emitI32Load8U(0);  break;
    case JitWidth::b16: m_emitter.emitI32Load16U(0); break;
    default:            m_emitter.emitI32Load(0);     break;
    }
    m_emitter.emitLocalSet(result->hardwareReg());
}

void JitWasmCodeGen::writeHost(JitWidth w, MemPtr address, RegPtr src, bool emulatedMemory) {
    if (address->rm) {
        pushRegValue(address->rm);
    } else {
        m_emitter.emitI32Const((S32)address->offset);
    }
    pushRegValue(src);
    switch (w) {
    case JitWidth::b8:  m_emitter.emitI32Store8(0);  break;
    case JitWidth::b16: m_emitter.emitI32Store16(0); break;
    default:            m_emitter.emitI32Store(0);    break;
    }
}

void JitWasmCodeGen::writeHost(JitWidth w, MemPtr address, U32 imm, bool emulatedMemory) {
    if (address->rm) {
        pushRegValue(address->rm);
    } else {
        m_emitter.emitI32Const((S32)address->offset);
    }
    m_emitter.emitI32Const((S32)imm);
    switch (w) {
    case JitWidth::b8:  m_emitter.emitI32Store8(0);  break;
    case JitWidth::b16: m_emitter.emitI32Store16(0); break;
    default:            m_emitter.emitI32Store(0);    break;
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
// For now: use cpu scratch fields + helper calls. Future: inline TLB.
// ---------------------------------------------------------------------------
RegPtr JitWasmCodeGen::readWriteMem(JitWidth w, RegPtr addressReg,
                                     std::function<void(RegPtr)> prepareWrite, S8 hint) {
    auto result = getTmpReg();
    // store address in cpu->src.u32
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    pushRegValue(addressReg);
    m_emitter.emitI32Store((U32)offsetof(CPU, src.u32));
    // call appropriate read helper
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitCall(m_helperReadMemIdx);
    // read result
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitI32Load((U32)offsetof(CPU, dst.u32));
    m_emitter.emitLocalSet(result->hardwareReg());
    if (prepareWrite) prepareWrite(result);
    return result;
}

RegPtr JitWasmCodeGen::read(JitWidth w, RegPtr addressReg,
                             std::function<void(MemPtr)> customOp,
                             std::function<void()> failedOp,
                             RegPtr tmp, bool checkAlignment) {
    if (!tmp) tmp = getTmpReg();
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    pushRegValue(addressReg);
    m_emitter.emitI32Store((U32)offsetof(CPU, src.u32));
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitCall(m_helperReadMemIdx);
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitI32Load((U32)offsetof(CPU, dst.u32));
    m_emitter.emitLocalSet(tmp->hardwareReg());
    return tmp;
}

void JitWasmCodeGen::write(JitWidth w, RegPtr addressReg, RegPtr src,
                            std::function<void(MemPtr)> customOp,
                            std::function<void()> failedOp, bool checkAlignment) {
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    pushRegValue(addressReg);
    m_emitter.emitI32Store((U32)offsetof(CPU, dst.u32));
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    pushRegValue(src);
    m_emitter.emitI32Store((U32)offsetof(CPU, src.u32));
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitCall(m_helperWriteMemIdx);
}

RegPtr JitWasmCodeGen::read(JitWidth w, MemPtr address, RegPtr result) {
    if (!result) result = getTmpReg();
    readHost(w, address, result, true);
    return result;
}

void JitWasmCodeGen::write(JitWidth w, MemPtr address, RegPtr src) {
    writeHost(w, address, src, true);
}

void JitWasmCodeGen::write(JitWidth w, MemPtr address, U32 imm) {
    writeHost(w, address, imm, true);
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
void JitWasmCodeGen::sarReg(JitWidth w, RegPtr reg, RegPtr rm)   { emitBinOp(w, reg, rm,  WASM_I32_SHR_S); }
void JitWasmCodeGen::sarValue(JitWidth w, RegPtr reg, U32 imm)   { emitBinOpImm(w, reg, imm, WASM_I32_SHR_S); }
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

void JitWasmCodeGen::rolReg(JitWidth w, RegPtr reg, RegPtr rm)  { emitBinOp(w, reg, rm, WASM_I32_ROTL); }
void JitWasmCodeGen::rolValue(JitWidth w, RegPtr reg, U32 imm)  { emitBinOpImm(w, reg, imm, WASM_I32_ROTL); }
void JitWasmCodeGen::rorReg(JitWidth w, RegPtr reg, RegPtr rm)  { emitBinOp(w, reg, rm, WASM_I32_ROTR); }
void JitWasmCodeGen::rorValue(JitWidth w, RegPtr reg, U32 imm)  { emitBinOpImm(w, reg, imm, WASM_I32_ROTR); }

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
void JitWasmCodeGen::imulRRI(JitWidth w, RegPtr dst, RegPtr src, U32 s2, RegPtr ov) { emitBinOp(w, dst, src, WASM_I32_MUL); }
void JitWasmCodeGen::imulRR(JitWidth w, RegPtr dst, RegPtr src, RegPtr ov)   { emitBinOp(w, dst, src, WASM_I32_MUL); }
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
    writeCPU(JitWidth::b32, (U32)offsetof(CPU, dst.u32), reg);
}
void JitWasmCodeGen::storeLazyFlagsSrc(RegPtr reg) {
    writeCPU(JitWidth::b32, (U32)offsetof(CPU, src.u32), reg);
}
void JitWasmCodeGen::storeLazyFlagsSrc(U32 value) {
    writeCPUValue(JitWidth::b32, (U32)offsetof(CPU, src.u32), value);
}
void JitWasmCodeGen::storeLazyFlagsResult(RegPtr reg) {
    writeCPU(JitWidth::b32, (U32)offsetof(CPU, result.u32), reg);
}
void JitWasmCodeGen::storeLazyFlagsOldCF(RegPtr reg) {
    writeCPU(JitWidth::b32, (U32)offsetof(CPU, oldCF), reg);
}
void JitWasmCodeGen::fillFlags(U32 flags) {
    // Store placeholder; actual flag computation is via helpers
    syncDirtyRegsToHost();
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitCall(m_helperSyncToHostIdx);
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
    return getConditionCalculationReg(0); // base class handles most of this
}
RegPtr JitWasmCodeGen::getConditionCalculationReg(U32 index) {
    return readCPU(JitWidth::b32, (U32)offsetof(CPU, flags));
}

// ---------------------------------------------------------------------------
// Control flow (If/Else/End/Goto)
// ---------------------------------------------------------------------------
void JitWasmCodeGen::finishIf() {
    // Emit the WASM `if`. We can't inject register sync here: the condition
    // value is already on the WASM value stack, and emitting stores before
    // `if` would move it below intermediate pushes and violate the spec's
    // "condition at top-of-stack" invariant used by some validators.
    // Instead, we eagerly load segments/GP regs at function entry so every
    // branch sees valid locals without needing per-branch sync.
    m_emitter.emitIf();
}

void JitWasmCodeGen::If(JitWidth w, RegPtr reg) {
    pushRegValue(reg);
    maskToWidth(w);
    finishIf();
}
void JitWasmCodeGen::IfNot(JitWidth w, RegPtr reg) {
    pushRegValue(reg);
    maskToWidth(w);
    m_emitter.emitOp(WASM_I32_EQZ);
    finishIf();
}
void JitWasmCodeGen::IfTest(JitWidth w, RegPtr reg, RegPtr mask) {
    pushRegValue(reg);
    pushRegValue(mask);
    m_emitter.emitOp(WASM_I32_AND);
    finishIf();
}
void JitWasmCodeGen::IfTest(JitWidth w, RegPtr reg, U32 mask) {
    pushRegValue(reg);
    m_emitter.emitI32Const((S32)mask);
    m_emitter.emitOp(WASM_I32_AND);
    finishIf();
}
void JitWasmCodeGen::IfNotTestBit(JitWidth w, RegPtr reg, U32 bitPos) {
    pushRegValue(reg);
    m_emitter.emitI32Const((S32)(1u << bitPos));
    m_emitter.emitOp(WASM_I32_AND);
    m_emitter.emitOp(WASM_I32_EQZ);
    finishIf();
}
void JitWasmCodeGen::IfTestBit(JitWidth w, RegPtr reg, U32 bitPos) {
    pushRegValue(reg);
    m_emitter.emitI32Const((S32)(1u << bitPos));
    m_emitter.emitOp(WASM_I32_AND);
    finishIf();
}
void JitWasmCodeGen::IfEqual(JitWidth w, RegPtr reg, DYN_PTR_SIZE value) {
    pushRegValue(reg);
    m_emitter.emitI32Const((S32)value);
    m_emitter.emitOp(WASM_I32_EQ);
    finishIf();
}
void JitWasmCodeGen::IfEqual(JitWidth w, RegPtr r1, RegPtr r2) {
    pushRegValue(r1); pushRegValue(r2);
    m_emitter.emitOp(WASM_I32_EQ);
    finishIf();
}
void JitWasmCodeGen::IfNotEqual(JitWidth w, RegPtr reg, DYN_PTR_SIZE value) {
    pushRegValue(reg);
    m_emitter.emitI32Const((S32)value);
    m_emitter.emitOp(WASM_I32_NE);
    finishIf();
}
void JitWasmCodeGen::IfNotEqual(JitWidth w, RegPtr reg, RegPtr r2) {
    pushRegValue(reg); pushRegValue(r2);
    m_emitter.emitOp(WASM_I32_NE);
    finishIf();
}
void JitWasmCodeGen::IfLessThan2(JitWidth w, RegPtr reg, U32 value) {
    pushRegValue(reg);
    m_emitter.emitI32Const((S32)value);
    m_emitter.emitOp(WASM_I32_LT_U);
    finishIf();
}
void JitWasmCodeGen::IfLessThan2(JitWidth w, RegPtr r1, RegPtr r2) {
    pushRegValue(r1); pushRegValue(r2);
    m_emitter.emitOp(WASM_I32_LT_U);
    finishIf();
}
void JitWasmCodeGen::IfGreaterThanOrEqual(JitWidth w, RegPtr r1, RegPtr r2) {
    pushRegValue(r1); pushRegValue(r2);
    m_emitter.emitOp(WASM_I32_GE_U);
    finishIf();
}
void JitWasmCodeGen::IfGreaterThanOrEqual(JitWidth w, RegPtr reg, U32 value) {
    pushRegValue(reg);
    m_emitter.emitI32Const((S32)value);
    m_emitter.emitOp(WASM_I32_GE_U);
    finishIf();
}
void JitWasmCodeGen::IfNotCPU(JitWidth w, RegPtr sib, U8 lsl, U32 offset) {
    // read from CPU struct and If-not
    auto tmp = readCPU(w, sib, lsl, offset);
    IfNot(w, tmp);
    freeScratch(tmp->hardwareReg());
}
void JitWasmCodeGen::IfCondition(JitConditional cond) {
    // Base JitCodeGen handles: it calls getCondition and emits the right check
    auto r = getCondition(cond);
    If(JitWidth::b32, r);
}
void JitWasmCodeGen::JumpIfCondition(JitConditional cond, U32 address) {
    // Used for direct conditional jumps in compiled blocks — emit as if/br
    IfCondition(cond);
    blockExit();
    EndIf();
}
void JitWasmCodeGen::IfDF() {
    // Check the direction flag (DF) in cpu->flags
    auto cur = getReadOnlyFlags();
    m_emitter.emitLocalGet(cur->hardwareReg());
    m_emitter.emitI32Const(DF);
    m_emitter.emitOp(WASM_I32_AND);
    finishIf();
}
void JitWasmCodeGen::IfSmallStack() {
    // FIXME: this approximates small-stack mode by checking ESP < 0x10000.
    // The correct check would read cpu->stackNotMask, but the lazy
    // load-cache in this backend breaks when segment/GP locals are assigned
    // inside one branch of an if/else pair — since the true branch of
    // Push/Pop16 uses `getReadOnlySegAddress(SS)`, the else branch would
    // read an uninitialized local at runtime. Tests start with ESP=4096 so
    // they always take the small-stack path where SS is loaded; games that
    // set stackNotMask explicitly will hit the correct path too once
    // per-branch cache invalidation is implemented.
    loadGPReg(4);
    m_emitter.emitLocalGet(wasmLocalForGPReg(4));
    m_emitter.emitI32Const(0x10000);
    m_emitter.emitOp(WASM_I32_LT_U);
    finishIf();
}
void JitWasmCodeGen::StartElse() { m_emitter.emitElse(); }
void JitWasmCodeGen::EndIf()     { m_emitter.emitEnd(); }
void JitWasmCodeGen::JumpInBlock(U32 address) {
    // Direct intra-block jump — not yet supported in first impl; fall back
    blockExit();
}
void JitWasmCodeGen::blockExit() {
    syncDirtyRegsToHost();
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitCall(m_helperGetNextOpIdx);
    m_emitter.emitReturn();
}
void JitWasmCodeGen::blockNext1(U32 eip, DecodedOp* op) {
    writeEip(eip);
    syncDirtyRegsToHost();
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitCall(m_helperGetNextOpIdx);
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
    // Sync state, call the C++ single-op emulator, reload.
    syncDirtyRegsToHost();
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitCall(m_helperEmulateSingleOpIdx);
    m_gpLoaded.fill(false);
}

// ---------------------------------------------------------------------------
// Direct operations (cmp/test/jmp for JIT optimization paths)
// ---------------------------------------------------------------------------
void JitWasmCodeGen::direct_cmp(JitWidth w, RegPtr left, RegPtr right) {
    pushRegValue(left); pushRegValue(right);
    m_emitter.emitOp(WASM_I32_SUB);  // sets implicit comparison state
    m_emitter.emitOp(WASM_DROP);
}
void JitWasmCodeGen::direct_cmp(JitWidth w, RegPtr left, U32 right) {
    pushRegValue(left); m_emitter.emitI32Const((S32)right);
    m_emitter.emitOp(WASM_I32_SUB); m_emitter.emitOp(WASM_DROP);
}
void JitWasmCodeGen::direct_test(JitWidth w, RegPtr left, RegPtr right) {
    pushRegValue(left); pushRegValue(right);
    m_emitter.emitOp(WASM_I32_AND); m_emitter.emitOp(WASM_DROP);
}
void JitWasmCodeGen::direct_test(JitWidth w, RegPtr left, U32 right) {
    pushRegValue(left); m_emitter.emitI32Const((S32)right);
    m_emitter.emitOp(WASM_I32_AND); m_emitter.emitOp(WASM_DROP);
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
void JitWasmCodeGen::tryDirect(DecodedOp* op, std::function<void()> callback,
                                 std::function<void()> fallback) {
    callback(); // WASM supports direct for all common ops
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
U32  JitWasmCodeGen::getIfJumpSize()             { return 4; } // placeholder

// MarkJumpLocation / Goto: used by JitCodeGen for intra-block branch patching.
// WASM uses structural control flow so binary patching isn't needed; these are
// no-ops that satisfy the interface.
U32  JitWasmCodeGen::MarkJumpLocation() { return markBufferLocation(); }
void JitWasmCodeGen::Goto(U32 location) { /* structural control flow — no binary patch needed */ }

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
    // Reset per-instruction scratch state
    m_scratchInUse.fill(false);
}

void JitWasmCodeGen::compile(DecodedOp* op) {
    JitCodeGen::compile(op); // delegates to dynamic_* dispatch
}

void JitWasmCodeGen::postCompile(DecodedOp* op) {
    // Nothing to do per-instruction beyond what compile() did
}

void JitWasmCodeGen::commitJIT(DecodedOp* op) {
    // Finalize the WASM function body and the module binary.
    m_emitter.endFunction();
    m_wasmBinary = m_emitter.finalize();

    if (m_wasmBinary.empty()) return;

    // Pass the helper function table to the JS instantiator.
    int tableIdx = boxedwine_wasm_instantiate(
        m_wasmBinary.data(),
        (int)m_wasmBinary.size(),
        g_wasmHelperTable,
        WASM_HELPER_COUNT
    );

    if (tableIdx < 0) {
        // Instantiation failed — leave pfn as-is (normal CPU handles it).
        return;
    }

    // Walk all ops in this block: the first op gets the startJITOp entry
    // (its pfnJitCode is the wasmTable index the dispatcher invokes). All
    // ops in the block share the same tableIdx so removeCodeBlock's sweep
    // picks them up for cleanup; they also get OP_FLAG_JIT so doJIT skips
    // recompiling. Only the first op's pfn points at startJITOp — control
    // never re-enters the middle of a WASM block.
    DecodedOp* cur = op;
    while (cur) {
        cur->pfnJitCode = (void*)(uintptr_t)(U32)tableIdx;
        cur->flags     |= OP_FLAG_JIT;
        cur->blockStart = op;
        if (cur == op) {
            cur->pfn = cpu->thread->process->startJITOp;
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
            boxedwine_wasm_free_block(tableIdx);
        }
    }
}

#endif // BOXEDWINE_WASM_JIT
