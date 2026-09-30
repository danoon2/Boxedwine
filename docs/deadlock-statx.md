# Deadlock demo startup: statx on an unlinked open file

Diagnosed on 2026-09-29 with the rebuilt Wine 11.0 filesystem, version 13.
Tests used copies of `Deadlock_demo_(1996)-6353/root`.

## Cause

The original release executable stopped after about 10 seconds with:

```text
Loading library WING32.dll ... failed (error c0000020).
Importing dlls for ... DEADLOCK.EXE failed, status c0000135
```

The installer-supplied WING32.DLL has writable shared PE sections. Wine's
server prepares a temporary backing file for those sections, unlinks it while
keeping its descriptor open, then queries the descriptor's metadata.
The newer libc implements this `fstat()` operation using
`statx(fd, "", AT_EMPTY_PATH, ...)`.

Boxedwine resolved that descriptor back to its former pathname and looked up
the pathname again. Once unlinked, lookup failed with ENOENT. Wine rejected
the image with STATUS_INVALID_FILE_FOR_SECTION (`c0000020`). The same pathname
lookup could also return the wrong file after a replacement at that name.

A standalone 32-bit Linux probe reproduces the defect without Wine: open a
file, extend it to 8192 bytes, unlink it, then call libc `fstat()` and raw
`statx()`. Native WSL succeeds; the old Boxedwine executable returned ENOENT
for both calls.

## Fix

`KProcess::statx()` now uses the open file object for an empty pathname with
AT_EMPTY_PATH, including its live length. Nonempty pathnames remain pathname
queries even when that flag is present, and absolute paths ignore dirfd.
The existing Unix-socket descriptor handling is preserved.

`FsFileNode::remove()` also clears the final link count after unlink, so an
open but unlinked file reports zero links, including when Windows requires
Boxedwine to retain the backing file under its temporary deletion directory.

This is a Boxedwine fix. No Wine module or filesystem ZIP changes are required.

## Validation and installation

- The native Linux probe and the fixed Boxedwine executable both report
  successful fstat/statx, size 8192, and link count zero.
- `testStatxEmptyPathUsesOpenFile` covers unlink, filename reuse, growth of the
  original descriptor, invalid/closed descriptors, absolute and relative
  paths with AT_EMPTY_PATH, and querying the current directory.
- Eight focused Windows unit tests pass, including existing hard-link and
  Linux pathname-resolution checks.
- The copied Deadlock root loads WING32 successfully with no overrides and
  remains running for the 150-second startup observation, then the harness
  stops its own process.
- The final run using the installed executable was closed by the user after
  the game appeared. The user confirmed they could start a game successfully.

The tested release executable is installed at
`project/msvc/BoxedWine/x64/Release/BoxedWine.exe`, SHA-256
`556241d7f811f4f6cc8512f2c332f3dccd6b705aeef9b9c5300e2ebfd21876ab`.
The original executable is backed up as
`tmp/deadlock-20260929/BoxedWine-before-statx.exe`.
Probe sources, copied roots, logs, build records, and installation hashes are
in `tmp/deadlock-20260929`.
