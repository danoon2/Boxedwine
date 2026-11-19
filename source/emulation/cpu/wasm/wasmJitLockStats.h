/*
 *  Copyright (C) 2012-2026  The BoxedWine Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 */

#pragma once

// Diagnostic only (GCC_EXTRA_FLAGS=-DBOXEDWINE_WASM_JIT_LOCK_STATS): counts
// every lock-prefixed op executed by the interpreter and by native WASM JIT
// atomics, per instruction, and logs the nonzero counts to the browser console
// every few seconds. The counters are shared and atomic, so they perturb timing.
#if defined(BOXEDWINE_WASM_JIT) && defined(BOXEDWINE_MULTI_THREADED) && defined(BOXEDWINE_WASM_JIT_LOCK_STATS)
#define BOXEDWINE_WASM_JIT_LOCK_STATS_ACTIVE

enum WasmJitLockStatsPath : U32 {
    WASM_JIT_LOCK_STATS_INTERPRETER = 0, // every interpreter execution
    WASM_JIT_LOCK_STATS_NATIVE = 1,      // JIT atomic fast path
    WASM_JIT_LOCK_STATS_JIT_SLOW = 2,    // compiled, but sent to the interpreter
};

extern U32 g_wasmJitLockStats[3][NUMBER_OF_OPS];
void wasmJitLockStatsStart();

inline void wasmJitLockStatsCountInterpreter(U32 inst) {
    wasmJitLockStatsStart();
    __atomic_fetch_add(&g_wasmJitLockStats[WASM_JIT_LOCK_STATS_INTERPRETER][inst], 1, __ATOMIC_RELAXED);
}
#endif
