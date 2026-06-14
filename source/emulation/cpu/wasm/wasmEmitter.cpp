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

#include "wasmEmitter.h"
#include <cstring>

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------
void WasmEmitter::appendULEB128(std::vector<U8>& buf, U64 val) {
    do {
        U8 byte = val & 0x7f;
        val >>= 7;
        if (val) byte |= 0x80;
        buf.push_back(byte);
    } while (val);
}

void WasmEmitter::appendSLEB128(std::vector<U8>& buf, S64 val) {
    bool more = true;
    while (more) {
        U8 byte = val & 0x7f;
        val >>= 7;
        if ((val == 0 && !(byte & 0x40)) || (val == -1 && (byte & 0x40))) {
            more = false;
        } else {
            byte |= 0x80;
        }
        buf.push_back(byte);
    }
}

void WasmEmitter::appendStr(std::vector<U8>& buf, const char* s) {
    U32 len = (U32)strlen(s);
    appendULEB128(buf, len);
    for (U32 i = 0; i < len; i++) buf.push_back((U8)s[i]);
}

void WasmEmitter::appendSection(std::vector<U8>& result, WasmSection id,
                                 const std::vector<U8>& content) {
    result.push_back((U8)id);
    appendULEB128(result, (U64)content.size());
    result.insert(result.end(), content.begin(), content.end());
}

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------
WasmEmitter::WasmEmitter() {}

// ---------------------------------------------------------------------------
// Type section
// ---------------------------------------------------------------------------
U32 WasmEmitter::addFuncType(const std::vector<WasmType>& params,
                              const std::vector<WasmType>& results) {
    FuncType ft;
    ft.params  = params;
    ft.results = results;
    U32 idx = (U32)m_types.size();
    m_types.push_back(ft);
    return idx;
}

// ---------------------------------------------------------------------------
// Import section
// ---------------------------------------------------------------------------
void WasmEmitter::addMemoryImport(const char* module, const char* field) {
    appendStr(m_importSection, module);
    appendStr(m_importSection, field);
    m_importSection.push_back(0x02);  // import kind = memory
#ifdef BOXEDWINE_MULTI_THREADED
    // In a pthreads build Emscripten creates wasmMemory as a shared
    // WebAssembly.Memory (backed by SharedArrayBuffer).  The WebAssembly spec
    // requires that a module importing a shared memory declares it as shared
    // AND specifies a maximum.  Limits flag 0x03 = has-max + shared.
    // We declare max = 65536 pages (4 GiB) so this module accepts any shared
    // memory Emscripten creates regardless of the -sMAXIMUM_MEMORY setting
    // (import validation requires provided.max <= declared.max, so a large
    // declared max is always compatible with a smaller actual max).
    m_importSection.push_back(0x03);  // limits: has-max, shared
    appendULEB128(m_importSection, 256);   // min 256 pages = 16 MB
    appendULEB128(m_importSection, 65536); // max 65536 pages = 4 GiB
#else
    m_importSection.push_back(0x00);  // limits: min only (non-shared)
    appendULEB128(m_importSection, 256);   // min 256 pages = 16 MB
#endif
    m_hasMemoryImport = true;
}

U32 WasmEmitter::addFunctionImport(const char* module, const char* field, U32 typeIdx) {
    appendStr(m_importSection, module);
    appendStr(m_importSection, field);
    m_importSection.push_back(0x00);  // import kind = function
    appendULEB128(m_importSection, typeIdx);
    return m_numImportedFunctions++;
}

U32 WasmEmitter::addGlobalImport(const char* module, const char* field) {
    appendStr(m_importSection, module);
    appendStr(m_importSection, field);
    m_importSection.push_back(0x03);          // import kind = global
    m_importSection.push_back((U8)WasmType::I32);
    m_importSection.push_back(0x00);          // mutability = const
    return m_numImportedGlobals++;
}

// ---------------------------------------------------------------------------
// Function + Export sections
// ---------------------------------------------------------------------------
U32 WasmEmitter::addFunction(U32 typeIdx) {
    U32 idx = m_numImportedFunctions + (U32)m_localFunctions.size();
    m_localFunctions.push_back(typeIdx);
    m_lastFunctionIndex = idx;
    return idx;
}

void WasmEmitter::addExport(const char* name, U32 funcIdx) {
    appendStr(m_exportSection, name);
    m_exportSection.push_back(0x00);  // export kind = function
    appendULEB128(m_exportSection, funcIdx);
}

// ---------------------------------------------------------------------------
// Code section – function body builders
// ---------------------------------------------------------------------------
void WasmEmitter::beginFunction(const std::vector<std::pair<U32, WasmType>>& locals) {
    m_currentBody.clear();
    m_inFunction = true;
    m_ctrlDepth = 0;
    m_currentFunctionIndex = m_lastFunctionIndex;
    clearNextBranchHint();
    // Encode locals declaration
    appendULEB128(m_currentBody, (U64)locals.size());
    for (auto& lp : locals) {
        appendULEB128(m_currentBody, lp.first);
        m_currentBody.push_back((U8)lp.second);
    }
}

void WasmEmitter::endFunction() {
    // Append end opcode
    m_currentBody.push_back(WASM_END);
    // Prepend size of body, then append to code section
    std::vector<U8> sizedBody;
    appendULEB128(sizedBody, (U64)m_currentBody.size());
    sizedBody.insert(sizedBody.end(), m_currentBody.begin(), m_currentBody.end());
    m_codeSection.insert(m_codeSection.end(), sizedBody.begin(), sizedBody.end());
    m_codeFuncCount++;
    m_inFunction = false;
    m_currentBody.clear();
    clearNextBranchHint();
}

// ---------------------------------------------------------------------------
// Instruction emitters (all append to m_currentBody)
// ---------------------------------------------------------------------------
void WasmEmitter::emitByte(U8 b) { m_currentBody.push_back(b); }

void WasmEmitter::emitULEB(U32 val) { appendULEB128(m_currentBody, val); }
void WasmEmitter::emitSLEB(S32 val) { appendSLEB128(m_currentBody, (S64)val); }

void WasmEmitter::emitLocalGet(U32 idx) {
    m_currentBody.push_back(WASM_LOCAL_GET);
    appendULEB128(m_currentBody, idx);
}
void WasmEmitter::emitLocalSet(U32 idx) {
    m_currentBody.push_back(WASM_LOCAL_SET);
    appendULEB128(m_currentBody, idx);
}
void WasmEmitter::emitLocalTee(U32 idx) {
    m_currentBody.push_back(WASM_LOCAL_TEE);
    appendULEB128(m_currentBody, idx);
}
void WasmEmitter::emitGlobalGet(U32 idx) {
    m_currentBody.push_back(WASM_GLOBAL_GET);
    appendULEB128(m_currentBody, idx);
}
void WasmEmitter::emitI32Const(S32 val) {
    m_currentBody.push_back(WASM_I32_CONST);
    appendSLEB128(m_currentBody, (S64)val);
}
void WasmEmitter::emitI64Const(S64 val) {
    m_currentBody.push_back(WASM_I64_CONST);
    appendSLEB128(m_currentBody, val);
}

static void emitMemArg(std::vector<U8>& buf, U32 align, U32 offset) {
    WasmEmitter::appendULEB128(buf, align);
    WasmEmitter::appendULEB128(buf, offset);
}

void WasmEmitter::emitI32Load(U32 offset, U32 align) {
    m_currentBody.push_back(WASM_I32_LOAD);
    emitMemArg(m_currentBody, align, offset);
}
void WasmEmitter::emitI32Load8S(U32 offset) {
    m_currentBody.push_back(WASM_I32_LOAD8_S);
    emitMemArg(m_currentBody, 0, offset);
}
void WasmEmitter::emitI32Load8U(U32 offset) {
    m_currentBody.push_back(WASM_I32_LOAD8_U);
    emitMemArg(m_currentBody, 0, offset);
}
void WasmEmitter::emitI32Load16S(U32 offset) {
    m_currentBody.push_back(WASM_I32_LOAD16_S);
    emitMemArg(m_currentBody, 1, offset);
}
void WasmEmitter::emitI32Load16U(U32 offset) {
    m_currentBody.push_back(WASM_I32_LOAD16_U);
    emitMemArg(m_currentBody, 1, offset);
}
void WasmEmitter::emitI32Store(U32 offset, U32 align) {
    m_currentBody.push_back(WASM_I32_STORE);
    emitMemArg(m_currentBody, align, offset);
}
void WasmEmitter::emitI32Store8(U32 offset) {
    m_currentBody.push_back(WASM_I32_STORE8);
    emitMemArg(m_currentBody, 0, offset);
}
void WasmEmitter::emitI32Store16(U32 offset) {
    m_currentBody.push_back(WASM_I32_STORE16);
    emitMemArg(m_currentBody, 1, offset);
}
void WasmEmitter::emitI64Load(U32 offset, U32 align) {
    m_currentBody.push_back(WASM_I64_LOAD);
    emitMemArg(m_currentBody, align, offset);
}
void WasmEmitter::emitI64Store(U32 offset, U32 align) {
    m_currentBody.push_back(WASM_I64_STORE);
    emitMemArg(m_currentBody, align, offset);
}
void WasmEmitter::emitF64Load(U32 offset, U32 align) {
    m_currentBody.push_back(WASM_F64_LOAD);
    emitMemArg(m_currentBody, align, offset);
}
void WasmEmitter::emitF64Store(U32 offset, U32 align) {
    m_currentBody.push_back(WASM_F64_STORE);
    emitMemArg(m_currentBody, align, offset);
}

void WasmEmitter::emitOp(U8 op) { m_currentBody.push_back(op); }

void WasmEmitter::emitCall(U32 funcIdx) {
    m_currentBody.push_back(WASM_CALL);
    appendULEB128(m_currentBody, funcIdx);
}
void WasmEmitter::emitCallIndirect(U32 typeIdx, U32 tableIdx) {
    m_currentBody.push_back(WASM_CALL_INDIRECT);
    appendULEB128(m_currentBody, typeIdx);
    appendULEB128(m_currentBody, tableIdx);
}

void WasmEmitter::setNextBranchHint(WasmBranchHint hint) {
    m_nextBranchHint = (U8)hint;
    m_hasNextBranchHint = true;
}

void WasmEmitter::clearNextBranchHint() {
    m_hasNextBranchHint = false;
}

void WasmEmitter::recordBranchHintIfNeeded() {
    if (!m_hasNextBranchHint) {
        return;
    }
    // Branch-hint offsets are relative to the start of the function body
    // (after locals). m_currentBody starts there, so the current size is the
    // exact byte offset the metadata section expects.
    m_branchHints.push_back({
        m_currentFunctionIndex,
        (U32)m_currentBody.size(),
        m_nextBranchHint,
    });
    clearNextBranchHint();
}

void WasmEmitter::appendBranchHintSection(std::vector<U8>& result) const {
    if (m_branchHints.empty()) {
        return;
    }

    std::vector<U8> content;
    appendStr(content, "metadata.code.branch_hint");

    U32 functionCount = 0;
    U32 lastFuncIndex = (U32)-1;
    for (const BranchHintEntry& hint : m_branchHints) {
        if (hint.funcIndex != lastFuncIndex) {
            functionCount++;
            lastFuncIndex = hint.funcIndex;
        }
    }

    appendULEB128(content, functionCount);
    for (U32 i = 0; i < m_branchHints.size();) {
        U32 funcIndex = m_branchHints[i].funcIndex;
        U32 j = i;
        U32 hintCount = 0;
        while (j < m_branchHints.size() && m_branchHints[j].funcIndex == funcIndex) {
            hintCount++;
            j++;
        }
        appendULEB128(content, funcIndex);
        appendULEB128(content, hintCount);
        for (; i < j; i++) {
            appendULEB128(content, m_branchHints[i].offset);
            appendULEB128(content, 1); // hint payload size
            appendULEB128(content, m_branchHints[i].likely);
        }
    }

    appendSection(result, WasmSection::Custom, content);
}

void WasmEmitter::emitIf(WasmType blockType) {
    recordBranchHintIfNeeded();
    m_currentBody.push_back(WASM_IF);
    m_currentBody.push_back((U8)blockType);
    ++m_ctrlDepth;
}
void WasmEmitter::emitElse() { m_currentBody.push_back(WASM_ELSE); }
void WasmEmitter::emitBlock(WasmType blockType) {
    m_currentBody.push_back(WASM_BLOCK);
    m_currentBody.push_back((U8)blockType);
    ++m_ctrlDepth;
}
void WasmEmitter::emitLoop(WasmType blockType) {
    m_currentBody.push_back(WASM_LOOP);
    m_currentBody.push_back((U8)blockType);
    ++m_ctrlDepth;
}
void WasmEmitter::emitEnd() {
    m_currentBody.push_back(WASM_END);
    if (m_ctrlDepth > 0) {
        --m_ctrlDepth;
    }
    // The trailing `end` that closes the function body itself runs with
    // depth already 0; the underflow guard above covers that case.
}
void WasmEmitter::emitBr(U32 depth)  {
    m_currentBody.push_back(WASM_BR);
    appendULEB128(m_currentBody, depth);
}
void WasmEmitter::emitBrIf(U32 depth) {
    recordBranchHintIfNeeded();
    m_currentBody.push_back(WASM_BR_IF);
    appendULEB128(m_currentBody, depth);
}
void WasmEmitter::emitDrop()         { m_currentBody.push_back(WASM_DROP); }
void WasmEmitter::emitReturn()       { m_currentBody.push_back(WASM_RETURN); }
void WasmEmitter::emitUnreachable()  { m_currentBody.push_back(WASM_UNREACHABLE); }

// ---------------------------------------------------------------------------
// Finalize: produce the complete WASM binary
// ---------------------------------------------------------------------------
std::vector<U8> WasmEmitter::finalize() {
    std::vector<U8> result;

    // WASM magic + version
    const U8 magic[] = { 0x00, 0x61, 0x73, 0x6d }; // \0asm
    const U8 ver[]   = { 0x01, 0x00, 0x00, 0x00 }; // version 1
    result.insert(result.end(), magic, magic + 4);
    result.insert(result.end(), ver,   ver   + 4);

    // Type section
    if (!m_types.empty()) {
        std::vector<U8> typeContent;
        appendULEB128(typeContent, (U64)m_types.size());
        for (auto& ft : m_types) {
            typeContent.push_back(0x60); // func type marker
            appendULEB128(typeContent, (U64)ft.params.size());
            for (auto t : ft.params)  typeContent.push_back((U8)t);
            appendULEB128(typeContent, (U64)ft.results.size());
            for (auto t : ft.results) typeContent.push_back((U8)t);
        }
        appendSection(result, WasmSection::Type, typeContent);
    }

    // Import section
    if (!m_importSection.empty()) {
        std::vector<U8> importContent;
        U32 importCount = m_numImportedFunctions + m_numImportedGlobals + (m_hasMemoryImport ? 1 : 0);
        appendULEB128(importContent, importCount);
        importContent.insert(importContent.end(),
                             m_importSection.begin(), m_importSection.end());
        appendSection(result, WasmSection::Import, importContent);
    }

    // Function section (local functions)
    if (!m_localFunctions.empty()) {
        std::vector<U8> funcContent;
        appendULEB128(funcContent, (U64)m_localFunctions.size());
        for (U32 typeIdx : m_localFunctions)
            appendULEB128(funcContent, typeIdx);
        appendSection(result, WasmSection::Function, funcContent);
    }

    // Export section
    if (!m_exportSection.empty()) {
        // Count exports by counting export entries (name+kind+idx triples)
        // We track count separately to avoid re-parsing
        std::vector<U8> expContent;
        appendULEB128(expContent, (U64)1); // only one export for now: "execute"
        expContent.insert(expContent.end(), m_exportSection.begin(), m_exportSection.end());
        appendSection(result, WasmSection::Export, expContent);
    }

    appendBranchHintSection(result);

    // Code section
    if (m_codeFuncCount > 0) {
        std::vector<U8> codeContent;
        appendULEB128(codeContent, m_codeFuncCount);
        codeContent.insert(codeContent.end(), m_codeSection.begin(), m_codeSection.end());
        appendSection(result, WasmSection::Code, codeContent);
    }

    return result;
}

#endif // BOXEDWINE_WASM_JIT
