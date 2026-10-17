#pragma once

#if defined(BOXEDWINE_WASM_JIT) && defined(BOXEDWINE_MULTI_THREADED)
#ifdef __TEST
constexpr bool WASM_JIT_DEFAULT_SAFE_MEMORY = true;
#else
constexpr bool WASM_JIT_DEFAULT_SAFE_MEMORY = false;
#endif
// Configure once at startup, before any CPU can execute guest instructions.
// Shared by all pthreads and guest processes; never changed afterward.
extern bool wasmJitNeedsMemoryFence;
void wasmJitInitMemoryOrdering(bool safeMemory = WASM_JIT_DEFAULT_SAFE_MEMORY);
#endif
