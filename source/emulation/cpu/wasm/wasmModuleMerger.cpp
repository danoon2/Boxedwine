/*
 *  Copyright (C) 2012-2026  The BoxedWine Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 */

#include "boxedwine.h"
#ifdef BOXEDWINE_WASM_JIT
#include "wasmModuleMerger.h"

namespace {
struct ParsedModule {
    std::vector<U8> typePayload;
    std::vector<U8> importPayload;
    U32 importedFunctions = 0;
    U32 functionType = 0;
    std::vector<U8> codeBody; // includes its ULEB byte-size prefix
    std::vector<std::pair<U32, std::string>> functionNames;
};

bool readUleb(const std::vector<U8>& bytes, size_t& pos, size_t end, U32& value) {
    value = 0;
    for (U32 shift = 0; shift < 35; shift += 7) {
        if (pos >= end) {
            return false;
        }
        U8 byte = bytes[pos++];
        if (shift == 28 && (byte & 0xf0)) {
            return false;
        }
        value |= (U32)(byte & 0x7f) << shift;
        if (!(byte & 0x80)) {
            return true;
        }
    }
    return false;
}

void appendUleb(std::vector<U8>& out, U64 value) {
    do {
        U8 byte = (U8)(value & 0x7f);
        value >>= 7;
        out.push_back(value ? (U8)(byte | 0x80) : byte);
    } while (value);
}

void appendSection(std::vector<U8>& out, U8 id, const std::vector<U8>& payload) {
    out.push_back(id);
    appendUleb(out, payload.size());
    out.insert(out.end(), payload.begin(), payload.end());
}

bool skipName(const std::vector<U8>& bytes, size_t& pos, size_t end) {
    U32 length = 0;
    return readUleb(bytes, pos, end, length) && length <= end - pos ? (pos += length, true) : false;
}

bool skipLimits(const std::vector<U8>& bytes, size_t& pos, size_t end) {
    if (pos >= end) {
        return false;
    }
    U8 flags = bytes[pos++];
    U32 ignored = 0;
    if (!readUleb(bytes, pos, end, ignored)) {
        return false;
    }
    return !(flags & 1) || readUleb(bytes, pos, end, ignored);
}

bool parseModule(const std::vector<U8>& bytes, ParsedModule& parsed, BString& error) {
    auto fail = [&error](const char* message) {
        error = B(message);
        return false;
    };

    static const U8 header[] = {0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00};
    if (bytes.size() < sizeof(header) || !std::equal(header, header + sizeof(header), bytes.begin())) {
        return fail("invalid wasm module header");
    }

    bool hasType = false;
    bool hasImport = false;
    bool hasFunction = false;
    bool hasCode = false;
    size_t pos = sizeof(header);
    while (pos < bytes.size()) {
        U8 id = bytes[pos++];
        U32 sectionSize = 0;
        if (!readUleb(bytes, pos, bytes.size(), sectionSize)) {
            return fail("bad wasm section size");
        }
        if (sectionSize > bytes.size() - pos) {
            return fail("wasm section exceeds module bounds");
        }
        size_t sectionEnd = pos + sectionSize;

        if (id == 0) {
            size_t q = pos;
            U32 length = 0;
            if (!readUleb(bytes, q, sectionEnd, length) || length > sectionEnd - q) {
                return fail("bad custom section name");
            }
            bool isName = length == 4 && std::equal(bytes.begin() + q, bytes.begin() + q + length, "name");
            q += length;
            // Other custom sections are optional metadata and remain ignored.
            while (isName && q < sectionEnd) {
                U8 subsection = bytes[q++];
                U32 size = 0;
                if (!readUleb(bytes, q, sectionEnd, size) || size > sectionEnd - q) {
                    return fail("bad name subsection size");
                }
                size_t end = q + size;
                if (subsection == 1) {
                    U32 count = 0;
                    if (!readUleb(bytes, q, end, count)) {
                        return fail("bad function name count");
                    }
                    for (U32 i = 0; i < count; ++i) {
                        U32 index = 0;
                        if (!readUleb(bytes, q, end, index) || !readUleb(bytes, q, end, length) || length > end - q) {
                            return fail("bad function name entry");
                        }
                        parsed.functionNames.emplace_back(index, std::string(bytes.begin() + q, bytes.begin() + q + length));
                        q += length;
                    }
                    if (q != end) {
                        return fail("trailing bytes in function names");
                    }
                }
                q = end;
            }
        } else if (id == 1) {
            if (hasType) {
                return fail("duplicate wasm type section");
            }
            parsed.typePayload.assign(bytes.begin() + pos, bytes.begin() + sectionEnd);
            hasType = true;
        } else if (id == 2) {
            if (hasImport) {
                return fail("duplicate wasm import section");
            }
            parsed.importPayload.assign(bytes.begin() + pos, bytes.begin() + sectionEnd);
            size_t q = pos;
            U32 importCount = 0;
            if (!readUleb(bytes, q, sectionEnd, importCount)) {
                return fail("bad import count");
            }
            for (U32 i = 0; i < importCount; ++i) {
                if (!skipName(bytes, q, sectionEnd) || !skipName(bytes, q, sectionEnd) || q >= sectionEnd) {
                    return fail("bad import name");
                }
                U8 kind = bytes[q++];
                U32 ignored = 0;
                switch (kind) {
                case 0: // function
                    if (!readUleb(bytes, q, sectionEnd, ignored)) {
                        return fail("bad function import");
                    }
                    parsed.importedFunctions++;
                    break;
                case 1: // table
                    if (q >= sectionEnd) {
                        return fail("bad table import");
                    }
                    q++; // reference type
                    if (!skipLimits(bytes, q, sectionEnd)) {
                        return fail("bad table limits");
                    }
                    break;
                case 2: // memory
                    if (!skipLimits(bytes, q, sectionEnd)) {
                        return fail("bad memory limits");
                    }
                    break;
                case 3: // global
                    if (sectionEnd - q < 2) {
                        return fail("bad global import");
                    }
                    q += 2;
                    break;
                default:
                    return fail("unsupported import kind");
                }
            }
            if (q != sectionEnd) {
                return fail("trailing bytes in import section");
            }
            hasImport = true;
        } else if (id == 3) {
            if (hasFunction) {
                return fail("duplicate wasm function section");
            }
            size_t q = pos;
            U32 functionCount = 0;
            if (!readUleb(bytes, q, sectionEnd, functionCount) || functionCount != 1) {
                return fail("wasm module must contain one function");
            }
            if (!readUleb(bytes, q, sectionEnd, parsed.functionType) || q != sectionEnd) {
                return fail("bad wasm function section");
            }
            hasFunction = true;
        } else if (id == 10) {
            if (hasCode) {
                return fail("duplicate wasm code section");
            }
            size_t q = pos;
            U32 bodyCount = 0;
            if (!readUleb(bytes, q, sectionEnd, bodyCount) || bodyCount != 1) {
                return fail("wasm module must contain one code body");
            }
            size_t bodyStart = q;
            U32 bodySize = 0;
            if (!readUleb(bytes, q, sectionEnd, bodySize)) {
                return fail("bad wasm code body size");
            }
            if (bodySize > sectionEnd - q || q + bodySize != sectionEnd) {
                return fail("wasm code body exceeds section bounds");
            }
            parsed.codeBody.assign(bytes.begin() + bodyStart, bytes.begin() + sectionEnd);
            hasCode = true;
        }
        pos = sectionEnd;
    }

    if (!hasType) {
        return fail("missing wasm type section");
    }
    if (!hasImport) {
        return fail("missing wasm import section");
    }
    if (!hasFunction) {
        return fail("missing wasm function section");
    }
    if (!hasCode) {
        return fail("missing wasm code section");
    }
    return true;
}
}

bool wasmJitMergeModules(const std::vector<WasmJitMergeInput>& inputs, std::vector<U8>& output, BString& error) {
    output.clear();
    error = B("");
    if (inputs.empty()) {
        error = B("no wasm modules to merge");
        return false;
    }

    std::vector<ParsedModule> parsed(inputs.size());
    for (size_t i = 0; i < inputs.size(); ++i) {
        if (!inputs[i].bytes) {
            error = B("null wasm module input");
            return false;
        }
        if (!parseModule(*inputs[i].bytes, parsed[i], error)) {
            return false;
        }
        if (i && parsed[i].typePayload != parsed[0].typePayload) {
            error = B("wasm module type section mismatch");
            return false;
        }
        if (i && parsed[i].importPayload != parsed[0].importPayload) {
            error = B("wasm module import section mismatch");
            return false;
        }
    }

    const ParsedModule& first = parsed[0];
    std::vector<U8> functionPayload;
    appendUleb(functionPayload, parsed.size());
    for (const ParsedModule& module : parsed) {
        appendUleb(functionPayload, module.functionType);
    }

    std::vector<U8> exportPayload;
    appendUleb(exportPayload, parsed.size());
    for (size_t i = 0; i < parsed.size(); ++i) {
        std::string name = "b" + std::to_string(i);
        appendUleb(exportPayload, name.size());
        exportPayload.insert(exportPayload.end(), name.begin(), name.end());
        exportPayload.push_back(0);
        appendUleb(exportPayload, (U64)first.importedFunctions + i);
    }

    std::vector<U8> codePayload;
    appendUleb(codePayload, parsed.size());
    for (const ParsedModule& module : parsed) {
        codePayload.insert(codePayload.end(), module.codeBody.begin(), module.codeBody.end());
    }

    output = {0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00};
    appendSection(output, 1, first.typePayload);
    appendSection(output, 2, first.importPayload);
    appendSection(output, 3, functionPayload);
    appendSection(output, 7, exportPayload);
    appendSection(output, 10, codePayload);
    // Each input's sole defined function moves after the shared imports.
    // Preserve its profiling name while remapping the function index.
    std::vector<U8> names;
    U32 nameCount = 0;
    for (size_t i = 0; i < parsed.size(); ++i) {
        for (const auto& entry : parsed[i].functionNames) {
            if (entry.first == parsed[i].importedFunctions) {
                appendUleb(names, (U64)first.importedFunctions + i);
                appendUleb(names, entry.second.size());
                names.insert(names.end(), entry.second.begin(), entry.second.end());
                nameCount++;
                break;
            }
        }
    }
    if (nameCount) {
        std::vector<U8> nameMap;
        appendUleb(nameMap, nameCount);
        nameMap.insert(nameMap.end(), names.begin(), names.end());
        std::vector<U8> custom = {4, 'n', 'a', 'm', 'e', 1};
        appendUleb(custom, nameMap.size());
        custom.insert(custom.end(), nameMap.begin(), nameMap.end());
        appendSection(output, 0, custom);
    }
    return true;
}
#endif
