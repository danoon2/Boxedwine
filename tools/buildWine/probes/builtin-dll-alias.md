# Builtin DLL aliases and VC80 activation contexts

`wine11_builtin_module_alias.patch` fixes a Wine 11 loader failure seen in
Sims3Launcher with native .NET 2 and Wine's builtin VC80 runtime.

## Failure and fix

.NET first loads MSVCR80 through the WinSxS assembly version 8.0.50727.42.
TSLHost.dll then loads MSVCP80 through version 8.0.50727.762, which also imports
MSVCR80. Wine correctly reuses its already mapped builtin MSVCR80 image, but
only remembers the first path.

During MSVCP80 initialization, `init_cxx_funcs()` calls
`GetModuleHandleA("msvcr80.dll")`. The active assembly context redirects that
name to the .762 path. Wine fails to find the module it just loaded, leaving
the C++ function pointers null. Initialization faults at address zero and
.NET reports that TSLHost.dll could not load (HRESULT 0x800703E6).

The patch records additional full paths when a successful image lookup reuses
a builtin module. Full-name module lookup recognizes these aliases, and
unloading the module releases them. The original module filename, reference
counting, and native DLL behavior are preserved. Paths that have never been
loaded do not become visible simply because their basenames match.

The patch is registered in `wine_builds.json` for Wine 11, including regular
and web filesystem builds. It does not modify previously published archives.

## Regression probe

Build the 32-bit probe:

```sh
i686-w64-mingw32-gcc -O2 -Wall builtin-dll-alias.c -o builtin-dll-alias.exe
```

Run it inside a Wine prefix with builtin VC80 selected:

```sh
WINEDLLOVERRIDES=msvcr80,msvcp80=b wine builtin-dll-alias.exe
wine builtin-dll-alias.exe --loader-only
```

For Boxedwine, pass the environment through its `-env` argument and mount the
probe into the guest filesystem. Each test creates and removes its own
temporary fixture directory. Capture standard output and require the
`BUILTIN_DLL_ALIAS_PASS` marker; the host engine's exit status alone does not
necessarily reflect the guest process's result.

The default mode has 26 checks. It copies builtin CRT placeholders into
different directories, loads them through two paths, and activates a manifest
that redirects MSVCP80 and MSVCR80 to the second directory. It checks full-path
and activation-context lookups, C++ initialization, original module naming,
reference counting, and removal of aliases after unloading.

`--loader-only` uses version.dll for 54 checks across three load/unload cycles.
It also runs on native Windows, where separate native files may have distinct
module handles. The CRT mode uses one cycle because this ELF Wine build has a
separate VC80 reinitialization failure after complete unload/reload, observed
with both original and patched loaders. That behavior is outside this fix.

## Validation on November 10, 2026

| Filesystem / loader | CRT checks | Loader checks |
| --- | --- | --- |
| Wine 11 v30, original | 5 failures / 26 | 12 failures / 54 |
| Wine 11 v30, patched | 26 passed | 54 passed |
| Wine 11 web v30, patched | 26 passed | 54 passed |
| Native Windows | Not used as a CRT reference | 54 passed |

Both regular and web ntdll modules built successfully and produced identical
binaries. The Wine builder's 91 unit tests passed. Sims3Launcher also opened
with `WINEDLLOVERRIDES=msvcr80,msvcp80=b`; loader tracing confirmed that both
VC80 DLLs were builtin and that the previously failing lookup returned the
loaded MSVCR80 handle.

After removing the saved native VC80 overrides, a normal launch through the
Boxedwine UI opened the launcher, and its Play button reached the New Game /
Sunset Valley menu. No saved game or extended gameplay was tested.
