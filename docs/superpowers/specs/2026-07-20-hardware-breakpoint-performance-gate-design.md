# Hardware Breakpoint Performance Gate Design

## Context

After merging `master` into `james/wine_test`, repeated native benchmarks show a performance regression:

- Cinebench: master averages about 5% faster.
- Quake 2: master averages more than 25% faster.

Compared with master, `james/wine_test` adds hardware-breakpoint and single-step support to several hot paths. `NormalCPU::run` checks `debugTrapActive` at dispatch entry, and every byte, word, dword, and qword guest-memory write can perform a relaxed atomic load before checking data watchpoints. The branch already caches debugger state to reduce the cost, but production builds still execute the conditional work when no debugger is active.

## Goal

Remove hardware-breakpoint and single-step checks from production hot paths at compile time while retaining the implementation for future use and keeping its unit tests active.

## Non-goals

- Do not remove ordinary software-breakpoint (`int3`) handling.
- Do not remove general `ptrace` attach, continue, register access, or process-stop support.
- Do not change debugger behavior when hardware-breakpoint support is enabled.
- Do not combine unrelated performance changes with this experiment.

## Compile-time Contract

Introduce `BOXEDWINE_HARDWARE_BREAKPOINTS` as the feature define.

- Production builds leave it undefined by default.
- Builds may opt in explicitly with `-DBOXEDWINE_HARDWARE_BREAKPOINTS`.
- `include/boxedwine.h` defines it automatically when `__TEST` is defined, so all existing native and Emscripten test configurations continue exercising the debugger implementation without duplicating build-system settings.

## Implementation Scope

Use the feature define around only the debugger-specific execution hooks that add production overhead:

- The `debugTrapActive` dispatch in `NormalCPU::run`.
- The pre-instruction and post-instruction debugger work inside `CPU::runNextSingleOp`.
- The `debugTrapActive` exits used after `popf` changes the trap flag.
- Calls that refresh debugger state when the trap flag changes.
- Data-watchpoint callbacks in `KMemory` byte, word, dword, and qword write paths.
- The relaxed atomic watchpoint-active field and its slow-path maintenance functions when they are not needed by a build.

Keep the debugger state fields and core implementation layout stable where removing them could change `CPU` offsets consumed by native and WASM JIT code. This avoids turning a targeted hot-path experiment into a JIT layout change.

When the feature is disabled, preprocessor selection must remove the branches, atomic loads, and calls from compiled production code. A runtime flag or branch-prediction hint is insufficient because it preserves the cost being measured.

## Behavior

With `BOXEDWINE_HARDWARE_BREAKPOINTS` enabled, execution breakpoints, data watchpoints, trap-flag single stepping, and ptrace single stepping retain their current behavior.

With the feature disabled, those hardware debug facilities are unavailable. Software `int3` traps and non-debugger `ptrace` behavior remain available. Writes to debug-register state may still be stored for ABI compatibility, but they do not activate instruction or memory hot-path checks.

## Testing and Verification

1. Establish a red check that production preprocessing or generated Release code still contains the current hot-path debugger hooks before the change.
2. Build the native x64 Test configuration. Because `__TEST` enables the feature, the existing hardware-breakpoint, data-watchpoint, trap-flag, and ptrace single-step tests must compile and pass.
3. Run the complete native fast test suite.
4. Build the native x64 Release configuration with the feature undefined.
5. Inspect the Release preprocessor or disassembly output for the affected functions to confirm that the debugger branches, atomic loads, and watchpoint calls are absent.
6. Build the Emscripten JIT test targets to catch cross-platform preprocessor or layout problems.
7. Benchmark the resulting production binary with the same Cinebench and Quake 2 workloads and compare repeated runs with both master and the pre-change `james/wine_test` numbers.

## Success Criteria

- Native and Emscripten Test builds keep hardware-breakpoint coverage enabled.
- The native fast suite has no regressions.
- Native Release builds contain no hardware-breakpoint checks in the selected CPU and memory hot paths unless explicitly opted in.
- Software breakpoint handling remains compiled in.
- Cinebench and Quake 2 measurements determine how much of the observed regression this single change explains before any second optimization is attempted.
