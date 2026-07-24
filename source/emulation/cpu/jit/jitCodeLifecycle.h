/*
 *  Copyright (C) 2012-2026  The BoxedWine Team
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
class CPU;
class KMemory;

class PreparedJitCodeInvalidation {
public:
    using Commit = void (*)(void* context) noexcept;
    using Discard = void (*)(void* context) noexcept;

    PreparedJitCodeInvalidation() = default;
    PreparedJitCodeInvalidation(void* context, Commit commit, Discard discard) noexcept : context(context), commitCallback(commit), discardCallback(discard) {}
    PreparedJitCodeInvalidation(const PreparedJitCodeInvalidation&) = delete;
    PreparedJitCodeInvalidation& operator=(const PreparedJitCodeInvalidation&) = delete;
    PreparedJitCodeInvalidation(PreparedJitCodeInvalidation&& other) noexcept;
    PreparedJitCodeInvalidation& operator=(PreparedJitCodeInvalidation&& other) noexcept;
    ~PreparedJitCodeInvalidation();

    void commit() noexcept;

private:
    void reset() noexcept;

    void* context = nullptr;
    Commit commitCallback = nullptr;
    Discard discardCallback = nullptr;
};

using JitPrepareCodeInvalidationCallback = PreparedJitCodeInvalidation (*)(KMemory* memory, const std::vector<DecodedOp*>& decodedOps, const std::vector<void*>& jitEntries);
using JitMemoryInvalidatedCallback = void (*)(KMemory* memory);
using JitThreadStartPreparingCallback = void (*)(CPU* cpu);
using JitThreadStartCancelledCallback = void (*)(CPU* cpu);

struct JitLifecycleCallbacks {
    JitPrepareCodeInvalidationCallback prepareCodeInvalidation = nullptr;
    JitMemoryInvalidatedCallback memoryInvalidated = nullptr;
    JitThreadStartPreparingCallback threadStartPreparing = nullptr;
    JitThreadStartCancelledCallback threadStartCancelled = nullptr;
    bool aggregatePreparedCodeInvalidation = false;
    bool usesCodeMemory = true;
};

// The selected JIT backend may register process-lifetime callbacks for work that has not yet produced an installed JIT entry.
void setJitLifecycleCallbacks(const JitLifecycleCallbacks& callbacks);

// True when pfnJitCode points into KMemory's native executable-code allocator.
bool jitUsesCodeMemory();

// True when one backend preparation transaction must cover every block in an
// invalidated range. Native backends retain one transaction per code block.
bool jitAggregatesPreparedCodeInvalidation();

// Prepare every backend action whose failure must abort invalidation before
// decoded operations or installed entries are changed. The returned commit is
// noexcept; best-effort post-retirement work must contain its own failures.
PreparedJitCodeInvalidation prepareJitCodeInvalidation(KMemory* memory, const std::vector<DecodedOp*>& decodedOps, const std::vector<void*>& jitEntries);

// Convenience entry point for callers which have not started mutation.
void jitCodeInvalidated(KMemory* memory, const std::vector<DecodedOp*>& decodedOps, const std::vector<void*>& jitEntries);

// Notify the selected JIT backend before an address space or its entire decoded-operation cache is reset. jitEntries may be empty when only the address-space identity is being detached from backend-owned pending work.
void jitMemoryInvalidated(KMemory* memory, const std::vector<void*>& jitEntries);

void jitThreadStartPreparing(CPU* cpu);
void jitThreadStartCancelled(CPU* cpu);

#endif
