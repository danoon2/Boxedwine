All tests used Wine 10 unless otherwise noted.

x64/Win64 Windows 11 on Intel i7-14700 (28 core)

||25R1|26R1
|---|---|---|
|Quake 2 800x600 Software|79fps 1083MB|73fps 809MB (99fps with Wine 11)
|Cinebench 11.5 CPU Test|6.15 1796MB|10.02 1403MB (63% faster)

x86/Win32 Windows 11 on Intel i7-14700 (28 core)

||25R1|26R1
|---|---|---|
|Quake 2 800x600 Software|32fps 395MB|57.3 fps 450MB (68.7fps with Wine 11)
|Cinebench 11.5 CPU Test|1.16 957MB|3.90 983MB (236% faster)

ArmV8 Windows 11 Snapdragon X - X126100 (8 core)

||25R1|26R1
|---|---|---|
|Quake 2 800x600 Software|46.3fps 1027MB|57.2fps 852MB (71.4fps with Wine 11)
|Cinebench 11.5 CPU Test|1.85 1688MB|4.14 1477MB (124% faster)

ArmV8 Mac OS 15.6 on Mac Mini M4 (10 core)

||25R1|26R1
|---|---|---|
|Quake 2 800x600 Software|74.6fps|84.3fps (105.3fps with Wine 11)
|Cinebench 11.5 CPU Test|3.16|4.71 (49% faster)

ArmV8 Asahi Linux on Mac Mini M1 (8 core)

||25R1|26R1
|---|---|---|
|Quake 2 800x600 Software|48.0fps|63.3fps (70.4fps with Wine 11)
|Cinebench 11.5 CPU Test|1.46|2.67 (83% faster)

`Quake 2 command line: +timedemo 1 +map demo1.dm2`

## Wasm JIT loop benchmarks and branch hints

The JIT keeps guest GP/XMM registers in Wasm locals across eligible loop
branches. Memory helpers still publish dirty registers before faults or exits.
The generated Wasm marks memory-helper paths as unlikely using
`metadata.code.branch_hint`. The engine must honor these hints to reproduce
the measured memory-loop benefit. Hints are optional for correctness.

Node 22.16.0 (V8 12.4.254.21-node.26), bundled with the local Emscripten SDK,
ignores those hints by default. For generated-JIT performance measurements,
enable them explicitly for both baseline and candidate:

```sh
node --experimental-wasm-exnref --no-liftoff \
  --experimental-wasm-branch-hinting path/to/benchmark.js
```

`--experimental-wasm-exnref` supports the test build's exception format;
`--no-liftoff` uses optimized Wasm code from the start to avoid tier-up noise.
Check `node --v8-options` for flag availability/defaults on another runtime,
and record the Node/V8 versions and hint setting with results. Browser engines
can differ; Node results alone do not establish browser or game performance.

In the 2026-10-09 aligned `ADD EAX,[ESI]` loop investigation, V8 emitted 15
native stack accesses per repeated fast-path iteration with cached registers
and hints ignored, versus 4 in the original loop. Honoring just the memory
slow-path hint reduced that to 6; honoring all emitted hints reduced it to 5.
The extra stack spills explain why removing guest-register stores could
otherwise make this benchmark slower.

Three alternating paired runs, with hints enabled for both builds, measured:

| Loop | Execution time reduction from cached registers |
| --- | ---: |
| Single-threaded, 64 iterations | 17.0% |
| Single-threaded, 1,024 iterations | 11.8% |
| Multithreaded, 64 iterations | 16.2% |
| Multithreaded, 1,024 iterations | 18.4% |

These are standalone generated-JIT measurements, not game FPS. With hints
disabled, the single-threaded 1,024-iteration case instead took 7.6% longer.
Exclude setup, compilation and warmup from timing, verify no new modules are
compiled during samples, and use the same hint setting for both builds. Pause
other CPU-heavy activity, allow ten seconds to settle, and run benchmarks
serially without competing builds or tests. Include short/variable counts,
cross-page helpers, and register/fault-state checks when changing loop caching.

## Wasm loop-invariant carry

On 2026-10-09, caching carry once per bounded loop activation removed repeated
lazy-carry helper calls from loops such as `PADDD; DEC ECX; JNZ`. The shared
`JitCodeGen::canCacheLoopCarry` analysis rejects instructions that set or leave
carry undefined, and uses the conservative loop-register operation contract.
The Wasm backend initializes a dedicated local on entry, including re-entry
after the 64-iteration budget. CPU lazy-flag stores remain intact for exits
and faults. Native backends do not yet use this cache; they need their own
entry and storage handling before any native performance claim can be made.

Five alternating baseline/candidate pairs per suite and target, using the
Node/V8 version and flags above, measured these nanoseconds per complete
64-iteration invocation (including generated-code entry/exit and dispatch):

| Loop | Target | Baseline ns | Candidate ns | Time reduction |
| --- | --- | ---: | ---: | ---: |
| PADDD / DEC / JNZ | Single-threaded | 267.22 | 61.04 | 77.2% |
| PADDD / DEC / JNZ | Multithreaded | 286.19 | 73.23 | 74.4% |
| DEC / JNZ | Single-threaded | 258.22 | 57.61 | 77.7% |
| DEC / JNZ | Multithreaded | 274.46 | 69.33 | 74.7% |

The baseline includes loop-register caching and adjacent-result forwarding.
Each case has five timed samples per process, with setup, compilation and
warmup excluded. The retained run has 10,300 samples across both targets and
the mixed/focused suites; an earlier interrupted quiet-period batch was
excluded. No new JIT modules were compiled during samples. Variable counts
from 1 to 64 and counts through 1,024 retained substantial gains. Single-
iteration cases had no meaningful regression. The 81 unaffected cases per
target had median changes of -0.05% (ST) and -0.21% (MT); individual control
changes ranged from -3.51% to +2.95%. Inspected carry-changing control loops
had identical V8 machine instructions after normalizing relocation addresses.

Validation passed 787 selected tests per Wasm target with hints enabled,
10 focused tests per target with hints disabled, and all 811 native fast
tests from WSL `/tmp`. Coverage includes materialized and lazy incoming
carry, consecutive loops, early exits, cross-page helpers, and load/store
faults around the loop budget boundary. These synthetic gains do not predict
game FPS. Local commands, raw results and snapshots are in
`tmp/wasm-loop-carry-20261009/`, with retained measurements in the two
`timing*-quiet-redo/` directories and a summary in `summary.json`.

## Rejected Wasm loop flag-store experiments

On 2026-10-09, two follow-up experiments tried to remove more stores from
register-only loops with one INC/DEC flag producer and a zero/sign backedge.
The baseline already included the retained carry cache described above.
The first deferred oldCF, lazyFlagType and result publication until loop
exits. The second hoisted only invariant oldCF/type stores to loop entry,
leaving result publication in every iteration. Both passed correctness but
repeatedly regressed some loops, so neither implementation was retained.

Five alternating pairs per suite and target, using the Node/V8 configuration
above, gave these changes in median execution time (positive is slower):

| Loop | Iterations | Target | Defer all three fields | Hoist invariant fields |
| --- | ---: | --- | ---: | ---: |
| DEC / JNZ | 64 | ST | +5.8% | +6.1% |
| DEC / JNZ | 64 | MT | +4.1% | -5.3% |
| DEC / JNZ | 1,024 | ST | +2.7% | +2.4% |
| DEC / JNZ | 1,024 | MT | +5.2% | -3.9% |
| PADDD / DEC / JNZ | 64 | ST | -0.4% | +4.6% |
| PADDD / DEC / JNZ | 64 | MT | -9.6% | +2.9% |
| PADDD / DEC / JNZ | 1,024 | ST | -8.0% | -4.1% |
| PADDD / DEC / JNZ | 1,024 | MT | -11.7% | -2.7% |

The ST DEC/64 regression appeared in every pair for both implementations.
The narrower version also regressed DEC/65 by 9.5% ST and PADDD/64 in every
pair on both targets. Some variable-count and larger loops improved, but
the benefit depended on the loop and target. The second version kept Wasm
module sizes unchanged and shrank inspected DEC V8 bodies by 64 bytes;
smaller code and fewer stores were still insufficient to ensure a gain.
Fourteen inspected unaffected control functions had identical normalized
V8 instructions. Generated register allocation and branch layout changed
in affected loops; their precise contribution to timing remains unproven.

Each experiment collected 10,300 samples, excluding setup, compilation and
warmup, in a separate user-approved quiet window after a ten-second pause.
Counts covered 1, 4, 16, 64, 65, 1,024 and variable 1..64; mixed tests also
included 256. Generated execution includes dispatch and entry/exit costs.
Both candidates passed 787 selected tests per Wasm target with hints on,
10 focused tests per target with hints off, and all 811 native fast tests
from WSL /tmp. Additional candidate coverage checked all condition codes,
lazy/materialized incoming carry, overflow, register/XMM progress,
post-loop faults, consecutive loops and raw budget/SMC exits (282 subcases).
No native runtime optimization was enabled in these experiments.

Production and test sources were restored byte-for-byte to the pre-experiment
snapshots, preserving earlier staged and unstaged work. Candidate source,
tests, binaries, exact commands and results are saved locally in
`tmp/wasm-loop-flags-20261009/`; see `summary.json`, `hoisted-source/`,
`deferred-source/`, and the four `timing*-quiet/` result directories.

## Native x64 fixed-stack x87 loop cache

On 2026-10-09, retained native x64 x87 register values across eligible
backedges instead of publishing and reloading them every iteration. Shared
`JitFPU` analysis accepts straight-line register-only loops using up to four
fixed x87 slots. It rejects memory operations, stack changes, interior
branches/entries, overlapping selected loops and other compiled edges into
the body. The x64 backend imports values at the public header entry and
places its private backedge label after initialization. TOP scratch metadata
is spilled before that label; only stable value registers cross the backedge.
Normal exits and scheduling exits publish the cache. Interior entry promotion
and self-modifying code retain the existing recompilation path.

Five alternating baseline/candidate pairs per memory mode measured generated
native x64 execution on WSL Linux, pinned to logical CPU 2, after user Ready
and a ten-second quiet period. The baseline is the exact pre-experiment
working tree, including earlier retained optimizations. Each process reports
five samples per case; 6,300 samples cover counts 1, 4, 16, 64, 65, 1,024 and
variable 1..64. Setup, compilation, calibration, warmup and verification are
outside reported timings. Times below are per complete 64-iteration inner
loop, including entry/exit and outer repetition control; negative is faster.

| Loop | Linear baseline (ns) | Linear candidate (ns) | Linear time change | Guarded time change |
| --- | ---: | ---: | ---: | ---: |
| FADD, two values | 140.8 | 104.5 | -25.8% | -24.1% |
| Three FADDs, four values | 254.5 | 138.4 | -45.6% | -45.0% |
| FCHS / FABS | 151.8 | 94.7 | -37.7% | -39.0% |
| FMUL / FADD | 184.0 | 129.3 | -29.7% | -30.6% |
| FADD / PXOR | 141.3 | 103.6 | -26.7% | -28.0% |

Every eligible case with four or more iterations, and every variable-count
case, improved in all five paired comparisons in both memory modes. Variable
counts reduced time by 12-28%. Single-iteration two-value FADD was essentially
unchanged (+0.9% linear, -0.6% guarded); other single-iteration cases improved.
The 28 unchanged controls per mode had median time changes of +0.59% linear
and -0.12% guarded, with individual cases ranging from -3.13% to +4.41%.
Memory, push/pop, five-value and interior-branch control functions had
identical normalized generated instructions. Eligible inspected blocks grew
5.8-8.5% (32-104 bytes) to handle initialization and separate exit writebacks.

All 811 native fast tests passed from WSL `/tmp`, plus six focused x87 tests
with linear memory disabled. New subcases cover all eight TOP values,
raw/cached inputs, one through five slots, precision modes, guest SSE,
integer flags, header re-entry, interior-entry promotion, code invalidation, scheduling
exit/resume and faults after the loop. Both ST and MT Wasm test targets built
without compiler warnings and passed 16 focused cache/entry/exit tests each.
An initial broader ST Wasm selection was stopped during unrelated exhaustive
remainder tests; it is not counted as a completed suite. Runtime retention
is enabled only on x64; Win64 and ARM64 were not measured in this experiment.
These synthetic results do not predict game FPS.

Snapshots, private benchmark harness, generated code, exact build/test
commands and raw paired results are in `tmp/native-x87-loops-20261009/`.
See `summary.json`, `benchmark.inc`, `private-builds.py`, `validate.py`,
`validate-wasm-focused.py`, `code-comparison.json` and
`timing-quiet/results.json`. Reproduce timings with
`wsl -e python3 /mnt/c/Boxedwine2/tmp/native-x87-loops-20261009/run-timing.py --pairs 5 --tag <new-tag>`
only after a separate quiet-machine confirmation and ten-second pause.

## Selective Wasm x87 loop preparation

On 2026-10-09, reused the shared fixed-stack loop proof and
`prepareFpuRegisterLoop` helper for ST and MT Wasm. Eligible loops prepare
only the unconditionally used x87 values, validating them before the loop,
without initializing the general eight-slot tag/validity state. Existing
dirty prefix values and their mapping remain live. Other loops retain
`prepareFpuCacheForLoop`, including memory, push/pop and conditional bodies.
The shared proof now permits a backedge at the end of a compiled block;
the x64 consumer retains its internal-branch restriction explicitly.
No additional native runtime optimization is enabled by this pass.

Five alternating baseline/candidate pairs per target collected 6,300 samples
after a user-approved quiet period and ten-second pause. Node 22.16.0 ran
with `--experimental-wasm-exnref --no-liftoff
--experimental-wasm-branch-hinting`, pinned to logical CPU 2 in WSL Linux.
The exact baseline includes the earlier native x87 optimization and all
retained Wasm changes. Timings cover generated execution, dispatch and loop
entry/exit; setup, compilation, warmup and checks are excluded. Inputs are
warmed double values. Counts cover 1, 4, 16, 64, 65, 1,024 and variable 1..64;
65 and above also exercise generated loop-budget exits and re-entry.
Negative changes below mean less execution time.

| Loop | Iterations | ST time change | MT time change |
| --- | ---: | ---: | ---: |
| FADD, two slots | 1 | -51.4% | -41.2% |
| FADD, two slots | 64 | -13.7% | -12.9% |
| FADD, two slots | 1024 | -18.8% | -11.9% |
| FADD, two slots | Variable 1..64 | -16.1% | -14.1% |
| Three FADDs, four slots | 64 | -15.1% | -8.8% |
| FCHS / FABS, one slot | 64 | -10.4% | -8.7% |
| FMUL / FADD, three slots | 64 | -13.1% | -10.5% |
| FADD / PXOR | 64 | -12.9% | -13.7% |
| Seven FADDs, eight slots | 64 | -9.0% | -9.1% |

All 84 eligible target/case combinations improved in all five paired
comparisons. Variable-count gains ranged from 6.9% to 20.2%. Single-iteration
gains were 13.2-53.3%; no eligible case showed a regression. The 21 unchanged
controls per target had median changes of -0.87% ST
and +0.52% MT. Individual control medians ranged
from -2.47% to +4.34%. One MT push/pop/64 pair had a large interruption-like
outlier (+80.9%); all samples were retained, and results use medians. These
small control shifts are not claimed as effects of the optimization.

For the inspected FADD/two-slot block, ST Wasm size fell from 5,827 to 3,125
bytes and V8 instruction size from 5,192 to 2,308 bytes; MT fell from 7,040
to 3,454 Wasm bytes and 6,196 to 2,552 V8 bytes. Across eligible inspected
blocks V8 instruction size fell 9-70%. Wasm still declares the backend's
fixed local layout; unused slots no longer require initialization or exit
writeback, and V8 can discard their unused locals. Six inspected fallback
control functions have identical normalized V8 instructions.

The new suite contains 536 subcases covering every TOP, sparse/high/all
slots, raw and cached inputs, exact untouched 80-bit values and tags,
24/64-bit precision settings, dirty prefix values, prefix pushes/FXCH,
post-loop slot use, integer flags, raw budget/SMC exits and resumption,
and page-crossing faults after the loop. It passed on ST and MT with hints
enabled and disabled. Twenty further focused x87/cache/entry tests passed
per target, along with all 811 native fast tests and six guarded-memory x87
tests from WSL `/tmp`. Both Wasm builds emitted no compiler warnings.
No browser or game performance was measured.

Artifacts and exact commands are in `tmp/wasm-x87-slots-20261009/`: baseline
snapshots, `benchmark.inc`, `private-builds.py`, `validate.py`,
`validate-native.py`, `inspect.py`, `code-sizes.json`, `code-comparison.json`,
`summary.json` and `timing-quiet/results.json`. Reproduce timings with
`wsl -e python3 /mnt/c/Boxedwine2/tmp/wasm-x87-slots-20261009/run-timing.py --pairs 5 --tag <new-tag>`
after a separate quiet-machine confirmation and ten-second pause.

## Shared x87 memory loops and ARM retention

During the 2026-10-09 autonomous 90-minute pass, extended the shared fixed-slot
x87 loop proof to supported FADD/FMUL/FSUB/FSUBR memory forms, including signed
word/dword integer operands. Wasm prepares only the selected values and their
validity, without unused tag state. Memory-first loops keep raw 80-bit inputs
lazy until the read succeeds. Native x64 and ARM accept memory only when all
imported values are used before the first potentially faulting read. Stores,
interior entries/branches, rotating stacks and unsupported helpers are excluded.
ARM now retains up to eight fixed x87 values in D8-D15 across those backedges;
x64 retains its four-value limit. Dirty values are still published before faults.

The baseline is the exact working tree before this pass, including the earlier
retained native/Wasm x87 work, rather than Git HEAD. Five alternating paired
processes, five samples per case, measure warmed generated-JIT execution with
entry/exit and loop control. Setup, compilation, calibration, warmup and checks
are outside samples. Counts are 1, 4, 16, 64, 65, 1024 and variable 1..64, with
aligned, unaligned, cross-page and sequential operands. Wasm verifies no new
modules during samples; native verifies stable compiled entry pointers.
Guarded x64 cross-page timing is excluded because that fallback invalidates its
entry; the path remains covered by correctness tests.

Local measurements use the i7-14700, WSL Linux and Node 22.16.0 with optimized
Wasm and branch hints enabled, pinned to logical CPU 2. ARM measurements ran on
the real Apple M4 Mac at james@192.168.48.103, macOS 27.0.1, with normal OS
scheduling. Builds/tests finished before timing, followed by ten seconds idle;
timings ran serially. The user authorized unattended timing for this period.
ARM has some large individual scheduling outliers; medians retain all samples.
These are synthetic measurements and do not establish game FPS or Windows timing.
Negative changes below mean less elapsed time.

### Wasm memory-loop preparation

The refined comparison collected 11,900 samples. Representative 64-iteration
cases are below. No targeted case had a median regression across the tested
counts. Unchanged register controls had median shifts of +0.13% ST and -0.30% MT.

| Loop | ST change | MT change |
| --- | ---: | ---: |
| FADD double | -12.4% | -10.3% |
| FADD single | -10.7% | -10.8% |
| Cross-page FADD double | -12.8% | -15.1% |
| FIADD word | -12.5% | -11.7% |
| FADD ST7 then FADD single | -14.6% | -15.3% |

### Native x64 memory-loop retention

This comparison collected 11,550 samples. For FADD ST7 followed by FADD single:

| Count | Linear baseline ns | Linear candidate ns | Linear change | Guarded change |
| --- | ---: | ---: | ---: | ---: |
| 1 | 2.97 | 3.32 | +11.6% | -0.6% |
| 4 | 10.18 | 9.29 | -8.7% | -16.5% |
| 64 | 160.54 | 116.35 | -27.5% | -20.6% |
| 1024 | 2557.55 | 1865.27 | -27.1% | -23.9% |
| Variable 1..64 | 82.11 | 68.18 | -17.0% | -6.2% |

The single-iteration linear case costs about 0.35 ns more. Four-plus iterations
and variable counts improve consistently. For the memory-loop change in
isolation, all 31 unaffected inspected control functions have identical
normalized native instructions; their median timing shifts were +0.21% linear
and -0.12% guarded. A broader experiment that skipped
clean source-value writebacks regressed several existing x64 register loops
(including roughly 15% for single-iteration mixed SSE), and was reverted on x64.

### ARM loop retention and clean imports

ARM imports through getF64 already update CPU storage. The shared import API
now lets that backend avoid marking unchanged source values dirty. Arithmetic
destinations remain dirty. x64 conservatively retains its previous marking,
and Wasm local conversions still require publication. The original-baseline
comparison collected 5,950 samples; representative 64-iteration cases follow.

| Loop | Baseline ns | Candidate ns | Change |
| --- | ---: | ---: | ---: |
| Two-value FADD | 140.56 | 91.01 | -35.3% |
| Four-value FADD | 221.97 | 118.91 | -46.4% |
| FMUL / FADD | 173.71 | 95.26 | -45.2% |
| Eight-value FADD | 541.18 | 235.55 | -56.5% |
| FADD ST7 then FADD single | 226.64 | 124.10 | -45.2% |

Every pair improved for these cases, despite some scheduling outliers. Entry
overhead remains visible in short cases: single-iteration FCHS/FABS is about
11.5% slower and register-plus-memory about 5.5% slower; longer and variable
loops benefit substantially. These costs should be considered with real workloads.

### Sign operations and TOP snapshot experiments

Shared FCHS/FABS helpers preserve the existing mask fallback for x64. ARM lowers
them to scalar FNEG/FABS. Its 7,350-sample incremental comparison was mostly
neutral, with about 2% less time for long FCHS/FABS loops and smaller code.
All five pairs improved for the 1024-iteration unary case; other small changes
overlap control noise and are not broad performance claims.

Wasm direct FNEG plus FABS improved long combined-sign loops 4-7%, but isolated
MT FCHS regressed about 3%. The retained narrower version uses direct FABS only.
Its 14,700-sample comparison improved variable-count combined-sign loops by
3.6% ST and 2.7% MT, with FCHS-only generated sizes unchanged. Single-iteration
and fixed-length gains vary; ST at 16 iterations regressed 2.5% in every pair.
The unchanged-control median shifts were +0.21% ST and +0.28% MT. Keep this
tradeoff visible rather than treating fewer instructions as a universal win.

The native TOP experiment saves the immutable entry-TOP spill copy once when
a cache region starts. Stack pushes/pops change only the relative mapping.
The broad x64 version improved push/pop and eight-value loops but regressed
the fixed memory loop about 4%, so fixed-slot x64 loops retain their original
preheader store placement. ARM uses the single snapshot throughout.

A final 11,550-sample comparison of the retained combined x64 source against
the original baseline measured these 64-iteration reductions: push/pop 14.9%
linear and 14.3% guarded; eight-value arithmetic 17.7% and 18.0%; mixed memory
27.5% and 20.6%. All five pairs improved for these cases. Other-case medians
were -0.16% linear and -0.01% guarded. The ARM incremental push/pop/64 case
improved about 1%, with four fewer generated bytes and improvement in all five
pairs. Other ARM changes largely overlap noise. Frozen comparisons and the
narrowed source remain available for review.

### Correctness, builds and reproduction

All 811 Linux native fast tests and six guarded-memory FPU tests passed from
WSL `/tmp`. ST and MT Wasm passed the expanded FPU suite with hints both enabled
and disabled, plus 20 focused tests each. New coverage includes 1,580 Wasm
memory-loop cases, 328 shared eager-memory cases, and 2,592 exact sign-bit cases:
all TOP values, cached/raw inputs, untouched 80-bit values/tags, dirty prefixes,
unaligned/page-crossing accesses, integer flags, and precise register/EIP state
at faults before/after the loop budget. Sign cases cover signed zeros, denormals,
infinities and signaling/quiet NaN payloads with normal and FTZ/DAZ MXCSR settings.
ARM host D8-D15 preservation is checked as well.

The M4 full suite passed 805 of 809 tests in baseline and candidate. The same
four pre-existing failures were reproduced: Movsb 2a4, Movsw 0a5, Movsd 2a5
direction-switch cases, and SSE Movmsk/Approx 350/351/352/353 (rsqrt approximation).
These were present before this FPU experiment, not necessarily before the
performance branch. They were subsequently resolved in the Jenkins repair below.
All six focused FPU tests passed. No game, browser UI or graphical app was launched.

MSVC x64 also passed all 811 fast tests and six focused FPU tests. Its 17
compiler warnings are confined to unchanged testString.cpp (14 C4146 and
three C4267); no new code warned. Normal Wasm, Linux and Mac builds emitted no
compiler warnings in this pass. Private benchmark injections intentionally
produce a Mac unreachable-code warning, separate from production builds.
The final ST/MT Wasm test binaries are byte-identical to the validated FABS
candidate, and the production ST JIT was rebuilt and HTTP/hash-verified.

Artifacts are in `tmp/jit-micro-20261009/`: `before/`, `refined-source/`,
`state.json`, `retention.json`, `summary.json`, benchmark includes, build scripts,
test logs, generated-code dumps and raw paired results. Core comparisons are
`timing-refined-quiet/`, `timing-final-native-quiet/`, `timing-retained-native-quiet/`,
`mac-results/timing-arm-clean/`, `sign/timing-abs-quiet/`,
`mac-results/timing-arm-sign/`, `timing-top-native-quiet/` and
`mac-results/timing-arm-top/`. Each runner records its environment and command;
see `run-refined.py`, `run-final-native.py`, `sign/run-abs.py`,
`run-top-native.py`, `run-retained-native.py` and the remote Mac runners. Frozen
benchmark executables
must be preserved: backend virtual-method changes make mixed old/new C++
objects unsuitable for reconstructing baselines. Use a fresh quiet-machine
confirmation and ten-second settling period for future timing sessions.


## Wasm register retention across forward joins (2026-10-09)

The initial retained change preloads GP/XMM inputs once for a bounded register-only
acyclic region, keeps those locals across its forward joins, and publishes
every potentially written register on exits. The eligibility policy is in
JitCodeGen; Wasm supplies the local-register implementation. Native x64/ARM
lowering is unchanged. This optimizes joins inside an existing generated
function. Fusion of noncontiguous guest blocks remains separate work: the
current block ownership, invalidation and persistence paths assume contiguous
decoded-op chains.

This initial version requires at least four internal forward branches, at most 128
decoded instructions, no internal backedges, and the existing conservative
GP/XMM operation whitelist. Guest memory operands are excluded. Existing loop
retention is unchanged. The branch threshold avoids eager initialization costs
that did not pay off in the two-join tests; 128 is a conservative analysis cap,
not a measured universal optimum.

### Paired generated-JIT measurements

Frozen baseline: faa513985c694313777ac6314a2d5d2fb15a6fc3. Node 22.16.0,
Emscripten 5.0.7, WSL Ubuntu x64. Each of two comparisons used five alternating
paired processes, five samples per case, and 48 shapes per target, for 4,800
samples per comparison. ST and MT ran serially on CPU 2, after explicit quiet
confirmation and a ten-second settling period. Every timed process used
`--experimental-wasm-exnref --no-liftoff --experimental-wasm-branch-hinting`.
The emitted modules contain the branch-hint metadata, including memory slow
paths. These are optimized-V8 results with hints enabled.

Both builds run identical guest bytes, varying all low four branch-input bits.
Cases cover integer additions, memory additions and SIMD additions; 0/2/4/8
joins and 1/4/16/64 operations per conditional arm. A separate compiled driver
varies inputs and repeats execution through normal dispatch. Setup, JIT
compilation, warmup and verification are outside timings; the module counter
must remain stable throughout calibration and timed batches. The reported
nanoseconds are per region-plus-driver execution. V8 can simplify repeated
arithmetic, so these synthetic results are not game FPS predictions.

Final narrowed candidate, four arithmetic operations per conditional arm:

| Kind / joins | ST baseline ns | ST candidate ns | ST change | MT baseline ns | MT candidate ns | MT change |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| Integer / 4 | 18.98 | 16.31 | -14.0% | 28.79 | 26.25 | -8.8% |
| Integer / 8 | 22.06 | 18.58 | -15.8% | 32.24 | 28.87 | -10.4% |
| SSE / 4 | 18.75 | 16.86 | -10.1% | 28.70 | 26.95 | -6.1% |
| SSE / 8 | 21.35 | 18.71 | -12.4% | 31.80 | 28.92 | -9.0% |

All five pairs improved for those table cases. Across all ten changed shapes,
ST time decreased 7.5-15.8% and MT 6.1-10.4%; one MT integer/4-join/16-work
pair was slower, with the other four faster. Generated Wasm decreased by
84-309 bytes per changed case (cumulative generated module bytes, not V8 native
machine-code size). The 38 unchanged shapes per target had median timing
shifts of +0.60% ST and -1.29% MT. Do not interpret those control shifts as gains.

### Rejected memory-region extension

The initial broader prototype also retained locals across memory operations.
For four joins with 16 loads per arm, its generated modules grew from 9,959 to
10,330 bytes in ST, and execution regressed 9.4% ST / 6.4% MT in every pair.
The whole-region dirty masks emitted additional register publication at each
potentially faulting helper. Permission-check counts did not increase. The
precise V8 allocation/layout reason for the hot-path slowdown was not isolated;
larger slow paths are an observed code-generation difference, not proof of a
particular V8 optimization failure. Hints were enabled in this experiment too.

The retained version leaves memory regions and regions with fewer than four
internal forward branches on their prior path. All 192 final benchmark
functions were disassembled and matched the intended measured baseline or
broader candidate exactly except verified embedded DecodedOp/nextJump addresses.
The final memory/4-join/16-work timing shifts were +0.94% ST / -1.70% MT, with
generated code restored to baseline. A future experiment could track dirty
state per control-flow join rather than excluding memory regions wholesale.

### Validation and artifacts

The broader prototype passed 787 selected Wasm regression tests per target
with hints on and ten focused tests per target with hints off. The final
narrowed source passed ten focused tests per target with hints both on and off,
including 157 new join/exit, SIMD-lane, interior-entry, invalidation, memory
fallback, precise-fault and self-modification executions each time. All 48
benchmark shapes per target passed six warmup-count checks in both baseline
and candidate. The final Linux native source passed all 811 fast tests from
WSL `/tmp`. No compiler warnings were emitted; make reported only its existing
jobserver warning. `git diff --check` passed. No browser/game was launched.

Artifacts: `tmp/jit-forward-regions-20261009/` contains `before/`, frozen
`baseline/`, broader `candidate-tests/`, final `refined/`, `retained-source/`,
`state.json`, `benchmark.inc`, build/validation scripts and logs.
`timing/results.json` records the broader experiment;
`timing-refined/results.json` records the retained comparison. Commands:
`build.py refined`, `validate.py refined`, `dump.py`, `compare-code.py`, and
`time-refined.py`. Run the timing script only after a fresh quiet-machine
confirmation and ten-second pause. `modules/normalized-comparison.json`
records every verified address-only difference. Production changes remain
unstaged; existing unrelated changes were preserved.


## Wasm locals across small memory regions (2026-10-09)

This extends the preceding register-only forward-region optimization to small
memory spans. It retains locals while separating cache validity from dirty
state: publish dirty GP/XMM values before a guest direct branch and on the
fallthrough edge before a forward label closes, then mark them clean. Every
incoming edge agrees that CPU state is current; the local mappings remain
valid. Conditional implementation-helper spills preserve dirtiness for paths
that did not take the helper. Subsequent writes become dirty normally.

The shared eligibility scan now records memory use and allows at most one
guest memory operation per straight-line span, resetting at an internal
forward destination or direct branch. The existing minimum four forward
branches, 128-op cap, no-internal-backedge rule and GP/XMM whitelist remain.
Wasm excludes FS/GS segment aliases and preloads other used segment bases.
The prior register-only and loop lowering is unchanged. Native x64/ARM do not
use the new forward-region policy, so there is no native performance claim.

### Measured benefit and cutoff

Baseline is the retained register-only source from the previous section,
including its unstaged changes, not bare HEAD. Frozen binaries are under
`tmp/jit-forward-memory-20261009/baseline/`. The 48-case benchmark and guest
inputs are unchanged. Five alternating paired processes, five samples per
case, ST/MT serially on CPU 2, explicit user quiet confirmation and ten seconds
idle produced 4,800 samples. Node 22.16.0 used
`--experimental-wasm-exnref --no-liftoff --experimental-wasm-branch-hinting`.
Setup, warmup, JIT compilation and verification are excluded, and generated
module counts must remain stable throughout timing.

For one memory addition per conditional arm, including normal driver dispatch:

| Forward joins | ST baseline ns | ST candidate ns | ST change | MT baseline ns | MT candidate ns | MT change |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| 4 | 19.69 | 17.21 | -12.6% | 29.34 | 28.02 | -4.5% |
| 8 | 23.75 | 21.08 | -11.3% | 33.80 | 31.43 | -7.0% |

All five pairs improved for those retained cases. Generated Wasm decreased by
161 bytes at four joins and 357 bytes at eight joins. This is generated module
size, not V8 native-code size. The 43 unchanged cases per target had median
timing shifts of -0.06% ST and -0.15% MT. A few control samples had scheduling
outliers; median/pair results are retained in full rather than discarded.

The broader prototype removed the earlier extra slow-path publications and
made all eligible memory cases smaller, but it did not improve all timings.
Four loads per arm were mixed: about -2.1% ST / -0.9% MT at four joins, and
-1.6% / -1.1% at eight joins, without consistent paired gains in both modes.
Sixteen loads per arm still regressed +2.1% ST / +2.5% MT (four of five ST
pairs and all MT pairs slower). Its ST module size decreased from 9,959 to
9,798 bytes. Smaller Wasm alone therefore does not establish faster execution.
The remaining V8 allocation/layout cause was not isolated.

The retained cutoff accepts only the clearly improved one-memory-op spans.
Counts two and three were not measured, so this is a conservative eligibility
limit, not a demonstrated optimal crossover. Regions containing a longer span
use the established lowering in full. The narrowed source was rebuilt and
validated; all 192 benchmark functions were disassembled and matched the
measured candidate for retained cases or measured baseline for other cases,
including byte size and branch hints. Normalization allows only verified
embedded DecodedOp/nextJump addresses. There was no second timing batch after
this code-identity check. These are synthetic gains, not an MDK FPS claim.

### Correctness and reproduction

The broad prototype passed 787 selected Wasm tests per target with hints on,
plus ten focused tests per target with hints off. Final narrowed ST/MT builds
passed ten focused tests with hints on and off, including the 157 forward
state checks. Memory cases now include distinct joins between load and store,
faulting reads/writes after GP/XMM changes made since the last publication,
cross-page accesses, signal-frame register/EIP state, external interior entry,
parent invalidation and a self-modifying store with dirty SIMD state. All 48
benchmark shapes per target passed all six warmup-count checks in frozen
baseline and final candidate. Final Linux native passed all 811 fast tests
from WSL `/tmp`. No compiler warnings; make's jobserver warning is unchanged.

Artifacts are in `tmp/jit-forward-memory-20261009/`: `before/` preserves the
previous retained source, `candidate-source/` the measured broad prototype,
and `retained-source/` the final limited version. `state.json` records source
hashes and outcomes. `timing/results.json` contains all raw paired summaries;
individual samples and environment are alongside it. Reproduction scripts:
`build.py candidate`, `validate.py candidate --full`, `inspect-code.py`,
`time.py`, `narrow.py`, `build.py narrowed`, `validate.py narrowed`, and
`verify-narrowed-code.py`. The last script records the exact final comparison
in `modules/narrowed-comparison.json`. `time.py` uses frozen broad-candidate
binaries; preserve those artifacts and obtain a fresh quiet-machine window
before rerunning. No staging, commit or push was performed.


## Wasm page translations for adjacent reads (2026-10-09)

The retained version reuses one guarded host address for four to eight
consecutive dword MOV/ADD memory reads, within a maximum 32-byte span. The
shared JitCodeGen scan requires matching segment/base/index/scale, unchanged
address inputs, 32-bit addressing and no intervening branch, store or helper.
An interior jump target ends the group. FS/GS accesses and other operations
retain their existing lowering. Native x64/ARM have no consumer of this
analysis and their generated execution is unchanged.

The Wasm backend uses one existing scratch local, reserved only while the
group is emitted. At runtime the first read checks that the whole span is
in one readable RAM page and records the translated minimum address. Each
following read uses a constant offset from that pointer. A failed group
guard leaves the pointer zero and uses the original checks for each actual
access, preserving guest order and exact fault state. No future guest load
is performed by the range guard. The pointer is refreshed on every group
execution, including each loop iteration; it is never kept across a guest
branch, store, mapping-changing helper or dispatch boundary. No DecodedOp
fields or Wasm local declarations were added.

### Paired measurements and explicit tradeoff

Baseline is the exact preceding working tree, including the retained forward
memory regions. Node 22.16.0 / Emscripten 5.0.7, WSL x64, CPU 2, with
`--experimental-wasm-exnref --no-liftoff --experimental-wasm-branch-hinting`.
Five alternating paired processes per ST/MT target, five samples per case,
48 cases, 12 ms target batches: 4,800 samples total. The user confirmed a
quiet machine; timings began after ten seconds idle with all build/test jobs
finished. Setup, compilation, calibration, warmup and verification are outside
reported timings, and generated module counts stay fixed while timing.
Execution includes normal generated dispatch through a separate driver.

The benchmark covers MOV, accumulating ADD and separated-load controls;
1/2/4/8 loads; aligned, unaligned, page-crossing and changing-base inputs.
MOV/8 overwrites four earlier destinations; ADD consumes every loaded value.
Raw output retains the reused harness names `joins` for load count and `work`
for layout (0 aligned, 1 unaligned, 2 crossing, 3 changing base).

Aligned accumulating ADD loads, nanoseconds per sequence plus driver:

| Reads | ST baseline ns | ST candidate ns | ST change | MT baseline ns | MT candidate ns | MT change |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| 4 | 16.27 | 14.62 | -10.1% | 28.40 | 25.01 | -11.9% |
| 8 | 20.12 | 15.05 | -25.2% | 31.87 | 26.97 | -15.4% |

All five pairs improved for these cases. Four-load aligned/unaligned MOV/ADD
cases reduce time 8.3-10.6% ST and 10.4-11.9% MT; eight-load cases reduce
time 22.6-25.2% ST and 15.4-18.5% MT. Changing-base cases also improved in
every pair, with smaller four-load benefits and larger eight-load benefits.
The 24 unchanged controls per target had median shifts +0.08% ST / -0.40% MT;
some individual controls moved several percent. These are synthetic results,
not a game FPS prediction.

The fallback has a measurable cost. When EVERY sequence crosses a page,
four-load MOV/ADD cases regress 8.1-9.6% ST / 1.8-4.1% MT; eight-load cases
regress 15.7-18.5% ST / 7.7-10.4% MT, consistently across all pairs. The guard
and extra fallback branches cause extra work; this pass does not claim to
eliminate that cost or measure its frequency in games. Keep this tradeoff
visible when testing real workloads. Generated Wasm grows by 130 bytes for
four reads and 202 bytes for eight reads (not V8 native-code size).

Two-load gains were mostly small, so the final cutoff is four. Three reads
were not measured. The 32-byte cap bounds neighboring fields rather than
attempting wide sparse-page coverage; it is conservative, not a measured
universal optimum. All 192 final benchmark functions were disassembled and
match the measured candidate for retained shapes, or measured baseline for
one/two-load and separated controls, including byte sizes and branch hints.
Normalization permits only verified embedded DecodedOp/nextJump addresses.
There was no second timing batch after this final code-identity check.

### Validation and reproduction

The prototype passed 787 selected Wasm tests per target with hints enabled
and ten focused tests per target with hints disabled. Final narrowed ST/MT
passed ten focused tests with hints on and off. The added 234 executions
cover positive/negative/repeated offsets, aligned and unaligned/page-spanning
reads, register and exact flag results, base/index writes, interior branch
entry, remapping between activations, 16-bit address wrapping, loop refresh
across the 64-iteration budget, and faults at each of four load positions.
Fault tests verify the signal frame's EIP, register progress, flags and dirty
SIMD prefix. All 48 benchmark cases pass six warmup counts in both variants.
Final Linux native passed all 811 fast tests from WSL /tmp. No compiler
warnings; make's existing jobserver warning remains. git diff --check passed.

Artifacts: `tmp/jit-read-groups-20261009/` contains `before/`, frozen
`baseline/`, measured `candidate/`, final `narrowed/`, source snapshots,
`state.json`, the benchmark and raw timing logs. Commands: `build.py baseline`,
`build.py candidate`, `validate.py candidate --full`, `native.py`,
`inspect-code.py`, `time.py --pairs 5`, `narrow.py`, `build.py narrowed`,
`validate.py narrowed`, `verify-narrowed-code.py`. time.py uses the measured
broad candidate; preserve its binaries and obtain a fresh quiet confirmation
before rerunning. `modules/narrowed-comparison.json` records final identity.
The source changes remain unstaged. No game or browser UI was launched.

### ARM Jenkins test repairs (2026-10-09)

Jenkins `james/performance_work` build 1 at `faa513985` compiled successfully,
but ARM test execution failed: two tests on Windows ARM64, three on Linux
ARM64, and four on Mac ARM64. The current tree reproduced all four Mac
failures in a full serial run on the M4.

Two production string issues originated in `7e2547fe0`:

- The REP MOVSB/MOVSW direction/count guard modified `getReadOnlyFlags()`.
  ARM returns its live flags register. Copying it to a temporary preserves
  flags and the direction used by the fallback helper.
- Unrolled MOVSD committed multiple overlapping elements before updating
  registers. An ARM LDAPR/STLUR alignment fault could restart that copy from
  the original registers and change its result. The shared guard now asks
  the backend for RAM-access alignment and uses the existing helper when
  alignment can fault. Other backends return alignment 1.

The tests also assumed that native host faults never replace compiled code,
and that reciprocal approximations always have native JIT result bits.
Code-identity checks now allow fault-driven recompilation while retaining
byte, register and flag checks. SSE memory tests use the exact SIMDe result
when a recorded host fault caused interpreter execution; native JIT checks
remain exact. The SSE test failure predates this performance branch.

Final validation:

| Target | Tests | Result |
|---|---:|---|
| Mac M4 ARM64, full serial | 809 | 0 failed, 80 seconds |
| Mac M4 ARM64, full, 4 workers | 809 | 0 failed, 28 seconds |
| Linux x64, fast, WSL `/tmp` | 811 | 0 failed, 7 seconds |
| Wasm ST JIT, string/approximate, branch hints enabled | 21 | 0 failed |
| Wasm MT JIT, string/approximate, branch hints enabled | 21 | 0 failed |

Artifacts and exact commands are in `tmp/jenkins-performance-build-1/`:
`mac-test.py`, `validate.py`, `state.json`, and the build/test logs. Mac
baseline and final applications remain in the isolated
`/private/tmp/boxedwine-jenkins-performance-build-1` directory. The Windows
ARM64 and Linux ARM64 Jenkins agents were not rerun. No commit or push was
performed, and the preceding Wasm performance changes were preserved.

### MT Wasm memory ordering repair (2026-10-10)

Jenkins build 2 compiled successfully and passed the native ARM tests, but
MT JIT shard 4/16 failed `locked cmpxchg against plain store` on Linux ARM64.
The same failure reproduced with Node 22.16 on the Mac ARM64 host: the
current build failed repetition 24 (phase 152), and the snapshot from before
forward-region retention failed repetition 42 (phase 228). This predates
the latest forward-region and adjacent-read changes.

The locked helper already uses atomic compare/exchange, but ordinary Wasm
loads/stores do not supply the surrounding x86 memory ordering. Emit
`atomic.fence` before guest memory instructions in the MT Wasm JIT, inside
branch/loop entry labels and before either the MMU fast path or helper path.
Apply the same boundary during interpreter warmup and single-op fallback.
This leaves native and ST Wasm code unchanged. The fence uses the
[standard sequentially consistent encoding](https://webassembly.github.io/threads/core/binary/instructions.html#atomic-memory-instructions).

This is a correctness repair, with a cost to MT memory-heavy workloads.
No performance improvement is claimed. A future optimization can replace
conservative fences with ordered accesses or prove some fences redundant;
removing them based only on passing races on x64 is insufficient. This
change orders instruction boundaries, not individual elements inside bulk
string helpers, and does not turn unlocked read/modify/write instructions
into atomic operations.

The unchanged 1,000-phase race passed 200 consecutive ARM64 processes with
the repair. The final test now runs 10,000 phases and passed another 50
ARM64 processes (500,000 phases). Tests and exact commands are recorded in
`tmp/jenkins-performance-build-2/`, including `reproduce.py`, `validate.py`,
`final-validate.py`, and per-target logs.

| Validation | Result |
|---|---|
| Original failing shard, Node ARM64 / x64, full mode | 57 / 57 passed on each host |
| Final concurrency tests, Node ARM64 / x64 | 14 / 14 passed on each host |
| Final Wasm MT / ST selected regressions, fast mode | 787 / 787 passed per target |
| Native x64, fast mode, WSL `/tmp` | 811 / 811 passed |

Node runs used `--experimental-wasm-exnref --no-liftoff
--experimental-wasm-branch-hinting`. The full failing shard used
`-shard 3 16`; final concurrency coverage used `769 14 1`. Browser CI will
rerun after the push; these local results use Node, not Firefox.
