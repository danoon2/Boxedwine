/*
 * Copyright (C) 2026 The BoxedWine Team
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "boxedwine.h"

#ifdef __TEST
#include "../cpu/testCPU.h"
#include "../../io/fsmemnode.h"
#include "../../kernel/loader/kelf.h"
#include "bufferaccess.h"
#include "loader.h"

namespace {
class TestElfNode final : public FsMemNode {
public:
    explicit TestElfNode(const std::vector<U8>& bytes)
        : FsMemNode(0, 0, B("/test-static-elf")),
          contents(BString::copy((const char*)bytes.data(), (int)bytes.size())) {}
    FsOpenNode* open(U32 flags) override {
        return new BufferAccess(shared_from_this(), flags, contents);
    }
private:
    BString contents;
};
}

void testElfInterpreterHonorsProgramHeaderCount() {
    for (U32 offset : {52u, 96u}) {
        std::vector<U8> bytes(4096, 0);
        k_Elf32_Ehdr header = {};
        memcpy(header.e_ident, "\177ELF\1\1\1", 7);
        header.e_phoff = offset;
        header.e_phentsize = sizeof(k_Elf32_Phdr);
        header.e_phnum = 2;
        memcpy(bytes.data(), &header, sizeof(header));
        k_Elf32_Phdr segment = {};
        segment.p_type = 1; // Two declared PT_LOAD entries.
        memcpy(bytes.data() + offset, &segment, sizeof(segment));
        memcpy(bytes.data() + offset + sizeof(segment), &segment, sizeof(segment));
        // Valid executable payload can resemble PT_INTERP. It is outside the
        // declared table and must never influence interpreter selection.
        segment.p_type = 3;
        segment.p_offset = 2048;
        constexpr char fakeInterpreter[] = "/not-an-elf-interpreter";
        segment.p_filesz = sizeof(fakeInterpreter);
        memcpy(bytes.data() + offset + 2 * sizeof(segment), &segment, sizeof(segment));
        memcpy(bytes.data() + segment.p_offset, fakeInterpreter, sizeof(fakeInterpreter));
        auto node = std::make_shared<TestElfNode>(bytes);
        BString loader, interpreter;
        std::vector<BString> arguments;
        FsOpenNode* opened = ElfLoader::inspectNode(B("/"), node, loader, interpreter, arguments);
        if (!opened || loader.length() || interpreter.length() || !arguments.empty()) {
            testFail("ELF loader interpreted payload beyond the declared program headers");
        }
        if (opened) { opened->close(); delete opened; }
    }
}
#endif
