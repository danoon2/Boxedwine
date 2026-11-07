# Unaligned guest memory access

Findings from November 2026. They explain why a `-sWASM=0` (wasm2js) Emscripten build
failed and what the accessor fix costs.

## Symptom

A Boxedwine build linked with `-sWASM=0` failed in Chromium and in netsurf-java with

    wine: could not load ntdll.so: /opt/wine/bi/../lib/wine/i386-unix/ntdll.so

(it should say `/opt/wine/bin/...`). Linking the same objects as normal wasm worked, and
so did wasm2js with `-sBINARYEN_EXTRA_PASSES=--dealign`.

## Cause

The guest RAM fast paths read and wrote through typed pointers at whatever address the
guest used:

- `KMemory::readdInline` / `writedInline` (`source/emulation/softmmu/kmemory_soft.h`):
  `*(U32*)(&ram[address & 0xFFF])`
- `KMemory::readw` / `writew` / `readq` / `writeq` (`source/kernel/kmemory.cpp`): `*(U16*)`,
  `*(U64*)`
- `RWPage::readw` / `writew` / `readd` / `writed` (`source/emulation/softmmu/soft_rw_page.cpp`)

x86 guests often use addresses that aren't aligned. Dereferencing a misaligned
`U16*`/`U32*`/`U64*` is undefined behaviour in C++, and the compiler is entitled to
assume natural alignment. In the `Build/Release` object files (`llvm-objdump`; the
second byte is log2 of the declared alignment):

| function | instruction | declared align |
|---|---|---|
| `readdInline` | `28 02 00 i32.load` | 4 |
| `readw` | `2f 01 00 i32.load16_u` | 2 |
| `readq` | `29 03 00 i64.load` | 8 |
| `writew` | `3b 01 00 i32.store16` | 2 |
| `writeq` | `37 03 00 i64.store` | 8 |

In wasm the alignment is only a hint: the engine must still perform a misaligned access
correctly, so the wasm build works. wasm2js trusts the hint and emits `HEAP32[addr >> 2]`,
which rounds the address down and reads the wrong bytes. `--dealign` sets every access to
align=1, which is why it worked.

### Native builds

- UBSan `-fsanitize=alignment` reports the construct: `load of misaligned address ... for
  type 'U32', which requires 4 byte alignment`.
- x86-64 and ARMv8 (clang -O2) emit a plain `mov` / `ldr`, and the hardware handles the
  misaligned address. The bug is latent there and could surface through vectorisation or
  atomics.
- On ARMv7 built with `-mno-unaligned-access`, the typed version still emits a single `ldr`
  because the compiler assumes alignment, while `memcpy` emits `ldrb` + `orr`.

## Fix

Use `hostLoadUnaligned<T>` / `hostStoreUnaligned<T>` (`memcpy` of `sizeof(T)` bytes,
`kmemory_soft.h`) in all of the paths above. They compile to a single load or store with
align=1 in wasm, and to the same `mov` / `ldr` on x86-64 and ARMv8.

The same change:

- Removes the `UNALIGNED_MEMORY` option. Nothing defined it, it did not compile
  (`x11common.cpp` had a literal `oops`), and its `RWPage::writed` stored to `ram[offset]`
  four times. The `memcpy` accessors give every host the alignment-free behaviour it was
  meant to provide.
- Makes `XSetWindowAttributes::get` (`source/x11/xwindow.cpp`) alias guest RAM only when
  the struct is naturally aligned. Otherwise it copies the struct field by field.
- Fixes `project/emscripten/makefile` so a build directory copied to a new `BUILD_DIR`
  tracks header changes. The `.d` files named their object under the original directory,
  so header edits recompiled nothing. They now name it as a literal `$(BUILD_DIR)/...`.

## How often guest accesses are misaligned

PERF_W95 boot and benchmark loop, counting fast-path RAM accesses only (temporary
counters, misaligned/total):

| point | 16-bit | 32-bit | 64-bit |
|---|---|---|---|
| boot (10 s) | 0.03% | 0.38% (1.25M / 326M) | 23% |
| benchmark loop (40 s) | 0.03% | 0.08% | 95% (28.4M / 29.8M) |

## Cost

V8 microbenchmark (node 24, emcc -O2, best of 7 runs): the alignment hint itself costs
nothing (aligned addresses: 0.442 vs 0.439 ns per load sequential, 0.566 vs 0.566 random).
Addresses that really are misaligned cost about 0.1-0.3 ns more per load with either hint.
That cost is in the hardware, and no accessor change removes it.

PERF_W95 (`app=PERF_W95.EXE.zip&p=PERF_W95.EXE&registry=win98;gdi&wine-override=nomono&bpp=8&resolution=640x480`):

| build | Chromium (headless) warm MIPS |
|---|---|
| wasm, before | 184, 185 |
| wasm, after | 185, 184, 186 |
| wasm2js, before | fails (`/opt/wine/bi/`) |
| wasm2js `--dealign`, before | 25 |
| wasm2js, after (no `--dealign`) | 51 |

netsurf-java (`make boxedwine`, endive, default 10x time dilation, title MIPS; these figures
are apparent MIPS, so real MIPS is about a tenth):

| build | first frame | title MIPS at ~340 s |
|---|---|---|
| wasm, before | 104.4 s, 113.0 s | 78, 70 |
| wasm, after | 103.6 s, 105.9 s | 74, 77 |

Within run-to-run noise, the fix does not change wasm speed in V8 or endive. It makes the
wasm2js build correct without `--dealign`, and twice as fast as the `--dealign` build.

## Not covered

Other host code still casts guest RAM to structs (`getRamPtr` in the Vulkan and OpenGL
marshalling, and the multi-threaded `common_lock.cpp` atomics). They need the same audit
before a wasm2js or strict-alignment build can be trusted beyond this boot path.
