# Mac host library dependencies

Updated September 11, 2026. All sizes below are regular-file logical bytes without duplicate framework symlinks; MB means 1,000,000 bytes. These are uncompressed bundle sizes, not App Store download sizes.

## Completed removal

All Mac targets now omit software Mesa: the common Mac arm64 definition and automation definitions are removed, along with the OSMesa dependency group's link/copy entries. Native OpenGL remains in use. SDL/hidapi, MoltenVK where used, system libraries, ZIP support, and the Wine filesystem are retained. Other platforms' OSMesa support is unchanged.

The shared `project/mac-xcode/remove-mesa-libraries.sh` clears the thirteen retired libraries and two obsolete Intel C++/ABI libraries from incremental app outputs, including the old automation copies beside its executable. The legacy UI and automation targets invoke it in sandboxed build phases; the native bundle preparation phase calls it before signing. The native audit rejects any of these files left in the bundle, even if unused.

Older Mac launch commands and saved settings selecting `-opengl osmesa` fall back to native OpenGL. Native library/backup/recovery metadata remains readable; launch construction strips the pair, and saving edited settings removes it. The legacy Mac UI ignores its old software-renderer selection.

| Measured app | Before | After |
|---|---:|---:|
| Native Debug | 388,337,276 bytes | 210,108,295 bytes |
| Native Release | Not measured in initial audit | 203,761,303 bytes |
| Native Mach-O images | 18 | 5 |

Debug saves **178,228,981 bytes (178.2 MB), about 46%** after rebuilding and signing. Both final bundles include the same 167,059,509-byte Wine 11/filesystem-11 ZIP, SHA-256 `fccc6fc9fe5294eaf9a8b0dca5461c40d577b5e801cfc2cb513c4a15cbfaa715`. Their Frameworks folders contain SDL2 (with nested hidapi) and MoltenVK. The downloaded build-dependency archive remains unchanged and may still contain unused inputs.

Verification on Apple M4:

- Native Debug and Release builds and dependency/signature audits passed, each with five Mach-O images.
- Legacy Boxedwine and BoxedwineAutomation arm64 Debug targets built in isolated unsigned test outputs. Their old dependency-download scheme preactions were not run.
- All 198 Swift tests and eleven Python bundle-audit checks passed. New checks cover old OSMesa arguments and incremental cleanup while preserving unrelated libraries.
- Native and legacy runtime symbols contain no `OsMesaGL` or `KOpenGLMesa`; native load commands contain none of the removed direct dependencies.
- A Windows probe launched through the regular native UI in the existing Alice environment reported `GL_VENDOR=Apple`, `GL_RENDERER=Apple M4`, and `GL_VERSION=2.1 Metal - 90.5`. It created a WGL pbuffer, cleared it, and read back RGBA `64,128,191,255` with no GL error. Its success dialog rendered, and the utility closed.
- The existing Motorhead installation, launched through the regular native UI, displayed a textured demo race labeled **3DFX RENDERER**. This exercises psVoodoo and WineD3D through native OpenGL. The game exited through its own exit screen with runtime exit code 0. Its log still contains WineD3D capability-probe and point-sprite warnings; this smoke check does not establish that all OpenGL calls or games are correct.

The macOS 15 deployment target remains unchanged. Intel, older macOS, extended gameplay, and other platforms were not tested in this removal. Local build/test logs, audit reports, bundle measurements, and graphics probe evidence are in `tmp/native-ui-test/mesa-removal/`.

## Initial audit, before removal

The initial read-only audit inspected the Debug arm64 `Boxedwine Native.app`, its signed runtime helper, native bundle preparation script, and Xcode targets. OSMesa and its complete non-system dependency group contained 13 libraries totaling **178,114,080 bytes (178.1 MB)**. No other consumer of these libraries was found. The dependency-file estimate before relinking/signing was 388,337,276 → 210,223,196 bytes; the actual rebuilt sizes above supersede that estimate.

| Removable with OSMesa | MB |
|---|---:|
| `libLLVM.dylib` | 123.140 |
| `libicudata.76.dylib` | 31.948 |
| `libOSMesa.8.dylib` | 16.902 |
| `libicuuc.76.dylib` | 1.828 |
| `libiconv.2.dylib` | 1.167 |
| `libxml2.2.dylib` | 1.107 |
| `libzstd.1.dylib` | 0.590 |
| `libglapi.0.dylib` | 0.442 |
| `libncurses.6.dylib` | 0.348 |
| `libedit.0.dylib` | 0.213 |
| `liblzma.5.dylib` | 0.184 |
| `libffi.8.dylib` | 0.137 |
| `libz.1.dylib` | 0.108 |

The dependencies recorded by `otool -L` are:

```text
OSMesa → glapi, LLVM, bundled zlib, zstd
LLVM → ffi, edit, bundled zlib, zstd, xml2
edit → ncurses
xml2 → bundled zlib, lzma, ICU common, iconv
ICU common → ICU data
```

The runtime explicitly linked LLVM, xml2, ncurses, iconv, and lzma despite having no imported functions from these five libraries (`nm -u -m`). Those link entries required removal along with the copied files, otherwise dyld would still require them at startup. Searching Boxedwine's source found its OSMesa dynamic loader and no independent loads of these dependency libraries. OSMesa was enabled for Mac arm64 in `include/boxedwine.h` under `BOXEDWINE_MAC_JIT` / `TARGET_CPU_ARM64`; removing an Xcode preprocessor entry alone was insufficient. The pre-removal helper contained `OsMesaGL` and `KOpenGLMesa` symbols, confirming that this path was compiled in.

## Independent dependencies

- SDL2 and its nested hidapi framework remain needed for the emulator's window/input/controller integration. Their inspected load commands lead only to macOS libraries and each other.
- MoltenVK (15.1 MB) belongs to the Vulkan backend. It has no dependency on the bundled Mesa/LLVM libraries. Vulkan loads entry points through SDL, so the absence of direct imported Vulkan functions does not establish that this library is unused.
- Boxedwine's JIT uses its own code and asmjit. `source/emulation/cpu/armv8/llvm_helper.cpp` contains copied helper implementations, not calls into the shared LLVM library.
- ZIP support remains in use. The built native launcher and helper link Apple's `/usr/lib/libz.1.dylib`; the separate bundled `libz.1.dylib` serves the Mesa group.
- The native catalog readers use Foundation `XMLParser`; the legacy UI uses pugixml. Neither requires this bundled xml2/ICU group.
- The inspected arm64 bundle used Apple's system C++ runtime and already omitted the old Intel-only `libc++.1.dylib` / `libc++abi.1.dylib` copies. The legacy Xcode targets still contained copy entries; those are now removed too.
- This audit concerns macOS host libraries. Wine's emulated Linux/Windows OpenGL, Direct3D, and Glide components inside the Wine ZIP are separate.

Raw pre-removal dependency inventory, import-provider lists, and the read-only inspection script are in `tmp/native-ui-test/mesa-dependencies/`.
