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
 * WasmEmitter: Builds a valid WebAssembly binary module at runtime.
 *
 * Architecture (inspired by the QEMU wasm64 TCG backend):
 *  - One WASM module per compiled JIT block
 *  - Module imports Emscripten linear memory + C++ helper functions
 *  - Exports a single "execute" function: (param cpu_ptr i32)
 *  - Generated via WebAssembly.Module + WebAssembly.Instance (synchronous)
 *  - Compiled function stored by Emscripten wasmTable index in pfnJitCode
 *
 * WASM binary format reference: https://webassembly.github.io/spec/core/binary/
 */

#ifndef __WASM_EMITTER_H__
#define __WASM_EMITTER_H__

#ifdef BOXEDWINE_WASM_JIT

#include "boxedwine.h"
#include <vector>
#include <utility>

// ---------------------------------------------------------------------------
// WASM section IDs
// ---------------------------------------------------------------------------
enum class WasmSection : U8 {
    Custom   = 0x00,
    Type     = 0x01,
    Import   = 0x02,
    Function = 0x03,
    Export   = 0x07,
    Code     = 0x0a,
};

// ---------------------------------------------------------------------------
// WASM value types used in signatures and locals
// ---------------------------------------------------------------------------
enum class WasmType : U8 {
    I32  = 0x7f,
    I64  = 0x7e,
    Void = 0x40,  // for block result types that return nothing
};

enum class WasmBranchHint : U8 {
    Unlikely = 0,
    Likely   = 1,
};

// ---------------------------------------------------------------------------
// WASM opcodes
// ---------------------------------------------------------------------------
enum WasmOp : U8 {
    WASM_UNREACHABLE    = 0x00,
    WASM_NOP            = 0x01,
    WASM_BLOCK          = 0x02,
    WASM_LOOP           = 0x03,
    WASM_IF             = 0x04,
    WASM_ELSE           = 0x05,
    WASM_END            = 0x0b,
    WASM_BR             = 0x0c,
    WASM_BR_IF          = 0x0d,
    WASM_RETURN         = 0x0f,
    WASM_CALL           = 0x10,
    WASM_CALL_INDIRECT  = 0x11,
    WASM_DROP           = 0x1a,
    WASM_SELECT         = 0x1b,
    WASM_LOCAL_GET      = 0x20,
    WASM_LOCAL_SET      = 0x21,
    WASM_LOCAL_TEE      = 0x22,
    WASM_GLOBAL_GET     = 0x23,
    WASM_I32_LOAD       = 0x28,
    WASM_I64_LOAD       = 0x29,
    WASM_F64_LOAD       = 0x2b,
    WASM_I32_LOAD8_S    = 0x2c,
    WASM_I32_LOAD8_U    = 0x2d,
    WASM_I32_LOAD16_S   = 0x2e,
    WASM_I32_LOAD16_U   = 0x2f,
    WASM_I32_STORE      = 0x36,
    WASM_I64_STORE      = 0x37,
    WASM_F64_STORE      = 0x39,
    WASM_I32_STORE8     = 0x3a,
    WASM_I32_STORE16    = 0x3b,
    WASM_I32_CONST      = 0x41,
    WASM_I64_CONST      = 0x42,
    WASM_I32_EQZ        = 0x45,
    WASM_I32_EQ         = 0x46,
    WASM_I32_NE         = 0x47,
    WASM_I32_LT_S       = 0x48,
    WASM_I32_LT_U       = 0x49,
    WASM_I32_GT_S       = 0x4a,
    WASM_I32_GT_U       = 0x4b,
    WASM_I32_LE_S       = 0x4c,
    WASM_I32_LE_U       = 0x4d,
    WASM_I32_GE_S       = 0x4e,
    WASM_I32_GE_U       = 0x4f,
    WASM_I64_EQ         = 0x51,
    WASM_I64_NE         = 0x52,
    WASM_I32_CLZ        = 0x67,
    WASM_I32_CTZ        = 0x68,
    WASM_I32_POPCNT     = 0x69,
    WASM_I32_ADD        = 0x6a,
    WASM_I32_SUB        = 0x6b,
    WASM_I32_MUL        = 0x6c,
    WASM_I32_DIV_S      = 0x6d,
    WASM_I32_DIV_U      = 0x6e,
    WASM_I32_REM_S      = 0x6f,
    WASM_I32_REM_U      = 0x70,
    WASM_I32_AND        = 0x71,
    WASM_I32_OR         = 0x72,
    WASM_I32_XOR        = 0x73,
    WASM_I32_SHL        = 0x74,
    WASM_I32_SHR_S      = 0x75,
    WASM_I32_SHR_U      = 0x76,
    WASM_I32_ROTL       = 0x77,
    WASM_I32_ROTR       = 0x78,
    WASM_I64_ADD        = 0x7c,
    WASM_I64_SUB        = 0x7d,
    WASM_I64_MUL        = 0x7e,
    WASM_I64_DIV_S      = 0x7f,
    WASM_I64_DIV_U      = 0x80,
    WASM_I64_REM_S      = 0x81,
    WASM_I64_REM_U      = 0x82,
    WASM_I64_AND        = 0x83,
    WASM_I64_OR         = 0x84,
    WASM_I64_XOR        = 0x85,
    WASM_I64_SHL        = 0x86,
    WASM_I64_SHR_S      = 0x87,
    WASM_I64_SHR_U      = 0x88,
    WASM_F64_DIV        = 0xa3,
    WASM_I32_WRAP_I64   = 0xa7,
    WASM_I64_EXTEND_I32_S = 0xac,
    WASM_I64_EXTEND_I32_U = 0xad,
    WASM_F64_PROMOTE_F32      = 0xbb,
    WASM_I64_REINTERPRET_F64  = 0xbd,
    WASM_F32_REINTERPRET_I32  = 0xbe,
    WASM_I32_EXTEND8_S  = 0xc0,
    WASM_I32_EXTEND16_S = 0xc1,
};

// ---------------------------------------------------------------------------
// WasmEmitter
//
// Builds a WASM binary suitable for `new WebAssembly.Module(bytes)`.
// Sections are accumulated in order; call finalize() to get the complete
// binary including the magic header.
// ---------------------------------------------------------------------------
class WasmEmitter {
public:
    WasmEmitter();

    // --- Type section -------------------------------------------------------
    // Add a function type (signature) and return its index.
    U32 addFuncType(const std::vector<WasmType>& params,
                    const std::vector<WasmType>& results);

    // --- Import section -----------------------------------------------------
    // Import the Emscripten linear memory. Must be called before function imports.
    void addMemoryImport(const char* module, const char* field);
    // Import a function; returns the function index (starting at 0).
    U32  addFunctionImport(const char* module, const char* field, U32 typeIdx);
    // Import an immutable i32 global; returns the global index (starting at 0).
    // Globals have their own index space, so this can be added at any point
    // before finalize() without disturbing function import indices.
    U32  addGlobalImport(const char* module, const char* field);

    U32 numImportedFunctions() const { return m_numImportedFunctions; }

    // --- Function + Export sections -----------------------------------------
    // Declare a local function (should be called after all imports).
    U32 addFunction(U32 typeIdx);
    void addExport(const char* name, U32 funcIdx);

    // --- Code section -------------------------------------------------------
    // Begin writing the body for a local function.
    // locals: list of (count, type) pairs for WASM local declarations.
    void beginFunction(const std::vector<std::pair<U32, WasmType>>& locals);

    // Core instruction emitters. These append bytes to m_currentBody.
    void emitByte(U8 b);
    void emitULEB(U32 val);
    void emitSLEB(S32 val);

    void emitLocalGet(U32 idx);
    void emitLocalSet(U32 idx);
    void emitLocalTee(U32 idx);
    void emitGlobalGet(U32 idx);
    void emitI32Const(S32 val);
    void emitI64Const(S64 val);

    void emitI32Load(U32 offset, U32 align = 2);
    void emitI32Load8S(U32 offset);
    void emitI32Load8U(U32 offset);
    void emitI32Load16S(U32 offset);
    void emitI32Load16U(U32 offset);
    void emitI32Store(U32 offset, U32 align = 2);
    void emitI32Store8(U32 offset);
    void emitI32Store16(U32 offset);
    void emitI64Load(U32 offset, U32 align = 3);
    void emitI64Store(U32 offset, U32 align = 3);
    void emitF64Load(U32 offset, U32 align = 3);
    void emitF64Store(U32 offset, U32 align = 3);

    void emitOp(U8 op);   // emit a standalone opcode
    void emitCall(U32 funcIdx);
    void emitCallIndirect(U32 typeIdx, U32 tableIdx = 0);

    void emitIf(WasmType blockType = WasmType::Void);
    void emitElse();
    void emitBlock(WasmType blockType = WasmType::Void);
    void emitLoop(WasmType blockType = WasmType::Void);
    void emitEnd();
    void emitBr(U32 depth);
    void emitBrIf(U32 depth);

    // Applies to the next emitted if/br_if only. Hint is consumed and cleared.
    void setNextBranchHint(WasmBranchHint hint);
    void clearNextBranchHint();

    // Number of currently-open structural blocks (if/block/loop). Used by
    // the JIT codegen to compute the relative depth argument for `br` in
    // LoopBegin/Goto. Counts each emitIf/emitBlock/emitLoop as +1 and
    // each emitEnd as -1 (emitElse does not change depth — `else` stays
    // inside the same `if` frame).
    U32 currentCtrlDepth() const { return m_ctrlDepth; }
    void emitDrop();
    void emitReturn();
    void emitUnreachable();

    // End the current function body and append it to the code section.
    void endFunction();

    // Produce the complete WASM binary (magic + version + all sections).
    std::vector<U8> finalize();

public:
    static void appendULEB128(std::vector<U8>& buf, U64 val);
    static void appendSLEB128(std::vector<U8>& buf, S64 val);

private:
    static void appendStr(std::vector<U8>& buf, const char* s);
    static void appendSection(std::vector<U8>& result, WasmSection id,
                               const std::vector<U8>& content);

    struct FuncType {
        std::vector<WasmType> params;
        std::vector<WasmType> results;
    };
    std::vector<FuncType> m_types;

    std::vector<U8> m_importSection;
    bool            m_hasMemoryImport = false;
    U32             m_numImportedFunctions = 0;
    U32             m_numImportedGlobals = 0;

    std::vector<U32> m_localFunctions;   // type indices
    std::vector<U8>  m_exportSection;
    std::vector<U8>  m_codeSection;      // all encoded function bodies
    U32              m_codeFuncCount = 0;

    std::vector<U8>  m_currentBody;      // being built by begin/endFunction
    bool             m_inFunction = false;
    U32              m_ctrlDepth = 0;    // open structural blocks (if/block/loop)

    struct BranchHintEntry {
        U32 funcIndex;
        U32 offset;
        U8  likely;
    };

    void recordBranchHintIfNeeded();
    void appendBranchHintSection(std::vector<U8>& result) const;

    std::vector<BranchHintEntry> m_branchHints;
    U32  m_lastFunctionIndex = 0;
    U32  m_currentFunctionIndex = 0;
    U8   m_nextBranchHint = 0;
    bool m_hasNextBranchHint = false;
};

#endif // BOXEDWINE_WASM_JIT
#endif // __WASM_EMITTER_H__
