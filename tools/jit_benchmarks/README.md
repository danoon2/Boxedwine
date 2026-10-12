# JIT benchmarks

These small, freestanding i386 programs measure generated instruction sequences
without a Windows installation, compiler optimization, or application startup in
the timed region. They complement the CPU correctness tests and application
benchmarks; a faster instruction kernel is not a prediction of application speed.

`generate.py` assembles the kernels with NASM. Each program first runs 200 ms of
CPU work, then warms its loop for 10,000 iterations, measures it with the guest
monotonic clock, and writes `result.bin`. Both warmups are excluded from the
measurement. `--preheat-ms` and `--warmup-iterations` control them; keep these
settings identical across builds and preserve the generated manifest. New kernels
include all eight XMM registers, general registers, and a small memory output in
that result. The runner compares these bytes across builds and repetitions. Use
the CPU test suite to validate instruction semantics, aliases, flags, and faults.

```sh
python3 tools/jit_benchmarks/generate.py /tmp/jit-bench-root/files --nasm nasm
python3 tools/jit_benchmarks/run.py \
  --build before=/path/to/before/boxedwine \
  --build after=/path/to/after/boxedwine \
  --root /tmp/jit-bench-root --output /tmp/jit-comparison \
  --filter 'shufps_|vector_transform|_control$' --cpu 4 --rounds 7
```

Choose a performance CPU appropriate to the machine. Runs are serial and reverse
build order each round. Do not build, run other benchmarks, or profile during a
timing comparison. Keep each binary and generated kernel directory immutable for
an experiment. `results.json` records binary hashes, per-case medians and ranges,
and every timing sample; `samples.jsonl` preserves completed samples if a run is
interrupted. Existing 144-byte kernels are supported alongside the newer format.

Use `--build-env after:VARIABLE=VALUE` to compare an existing optional code path
without rebuilding. Environment variables are inherited, so start comparisons
with known Boxedwine settings. Several Boxedwine switches test for presence,
rather than whether a value is `0`.

For x64/Wasm portability experiments, `known_flags_*` covers comparison proofs,
`division_*` covers repeated SSE exception checks, `scalar_ss_*` covers low-lane
arithmetic, and `math_*` covers cached and raw x87 math. Compare one executable
against itself with the relevant fallback control present on the baseline:
`BOXEDWINE_X64_KNOWN_FLAGS_SLOW`, `BOXEDWINE_JIT_DIV_GUARDS_SLOW` (Wasm),
`BOXEDWINE_WASM_SCALAR_REGION_SLOW`, or `BOXEDWINE_X64_FPU_HELPERS_SLOW`.
`BOXEDWINE_X64_CACHED_MATH_SLOW` isolates cached math from direct helper calls.
Use the same environment policy when creating each browser runtime, and disable
persistent module replay for those experiments. These are translation controls;
changing one does not replace blocks that have already been compiled.

Shuffle cases include dependent chains, independent register streams, distinct
inputs, and a mixed vector transform. Memory cases include loads, stores, copies,
scalar-double arithmetic, and spills. Helper-heavy controls help expose costs
when assigning a host register to a different purpose. Some arithmetic controls
reach infinities; they measure an unchanged instruction path, not a realistic
numerical computation.

Use a new or empty output directory for each generation or measurement run.
The tools reject an occupied directory so logs, samples and kernels from an
earlier experiment cannot be silently replaced or mixed. Iteration and warmup
counts must be positive; the supported preheat range is 0–2000 ms.

The data segment is padded to complete 16 KiB host-page groups. This matters on
Asahi: an incompletely mapped group can make Boxedwine recompile the same memory
operation to its guarded MMU path. Check a JIT dump when distinguishing direct
memory access from that fallback; ELF segment alignment alone is insufficient.

For Cinebench R11.5, `cinebench.py` uses its `-cb_cpux` command-line option and
parses the score from stdout. Supply an isolated root with the application and
its accepted-disclaimer preferences, and the exact filesystem ZIP. It repeatedly
checks every host thread's CPU affinity, since guest threads can change their
affinity after launch. Use `--headless` when the host has no active display, and
use identical settings for both builds. Profiling (`--perf`) must be a separate
run from timing trials. `analyze_profile.py` joins the resulting realtime perf
timestamps with `BOXEDWINE_JIT_MAP` records to locate hot guest and native blocks.

All runners accept an absolute ISO timestamp through `--deadline` to stop
starting new samples. They leave existing results intact.

For cross-checks against an installed FEX/muvm setup, add
`--build fex=/usr/bin/muvm --fex fex` to `run.py`. The same ELF and output checks
are used in both emulators. Verify the VM's host affinity and guest CPU mapping;
the default uses host cores 4–7 and pins the benchmark to guest CPU 0. Exclude
CPUID kernels because different CPU identification is expected. A single smoke
run only verifies execution and matching outputs, not a performance result.

`sevenzip.py` runs the 32-bit 7-Zip bundled in the supplied filesystem ZIP. It
records compression and decompression MIPS separately, alternates build order,
and checks host thread affinity. Use explicit thread and dictionary sizes. A
FEX comparison requires `--fex-exe` pointing to the identical extracted 7za.
The builtin benchmark complements the instruction kernels with a larger integer
and memory workload. Preserve the complete logs and also verify an archive
round trip when evaluating memory changes.
