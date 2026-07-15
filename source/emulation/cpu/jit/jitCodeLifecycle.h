/*
 *  Copyright (C) 2012-2025  The BoxedWine Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 */

#ifndef __JIT_CODE_LIFECYCLE_H__
#define __JIT_CODE_LIFECYCLE_H__

#include <vector>

class DecodedOp;
class KMemory;

using JitCodeInvalidatedCallback = void (*)(KMemory* memory, const std::vector<DecodedOp*>& decodedOps);
using JitMemoryInvalidatedCallback = void (*)(KMemory* memory);

struct JitLifecycleCallbacks {
    JitCodeInvalidatedCallback codeInvalidated = nullptr;
    JitMemoryInvalidatedCallback memoryInvalidated = nullptr;
    bool usesCodeMemory = true;
};

// The selected JIT backend may register process-lifetime callbacks for work
// that has not yet produced an installed JIT entry.
void setJitLifecycleCallbacks(const JitLifecycleCallbacks& callbacks);

// True when pfnJitCode points into KMemory's native executable-code allocator.
bool jitUsesCodeMemory();

// Notify the selected JIT backend before decoded operations and their
// installed entries are invalidated.
void jitCodeInvalidated(KMemory* memory, const std::vector<DecodedOp*>& decodedOps, const std::vector<void*>& jitEntries);

// Notify the selected JIT backend before an address space or its entire
// decoded-operation cache is reset. jitEntries may be empty when only the
// address-space identity is being detached from backend-owned pending work.
void jitMemoryInvalidated(KMemory* memory, const std::vector<void*>& jitEntries);

#endif
