# WebGL virtual display modes

Boxedwine's `source/x11/xrandr.cpp` owns the resolution list. In Emscripten it
advertises 640x480, 800x600, 1024x768 and the initial virtual desktop size, with
duplicate dimensions removed. Native builds retain their host-derived larger
modes. Each display caches its list so a mode switch does not replace its
initial size.

WineD3D's old WebGL shortcut returned only `ENUM_CURRENT_SETTINGS`, bypassing
that list. The v52 patch removes the shortcut and uses normal display
enumeration, retaining only 32-bit entries when the WebGL profile is enabled.
The existing logical mode-switch implementation remains unchanged; it resizes
the virtual display without switching the host monitor's mode. GDI and native
WineD3D enumeration are not filtered by this patch.

## Build and run

This is a local candidate, not a filesystem release. The candidate configuration
pins all 76 production patches and all eight DLL identities:

```sh
python3 tools/buildWine/webgl_filesystem.py build \
  --config tools/buildWine/webgl_filesystems_display_modes.json \
  --work-dir /path/to/fresh-wine-build --jobs 12
source /path/to/emsdk/emsdk_env.sh
make -C project/emscripten jit -j12
i686-w64-mingw32-gcc -O2 -Wall -Wextra -Werror \
  tools/wineTests/tests/webgl_display_modes_probe.c \
  -ld3d9 -lgdi32 -o display-modes.exe
zip probe.zip display-modes.exe
```

Package the candidate DLLs into a local copy of the Wine 11 web filesystem with
`webgl_filesystem.py package --config tools/buildWine/webgl_filesystems_display_modes.json
--input <web-filesystem.zip> --dll-dir <build>/wine-build/boxedwine-webgl-dlls
--output <candidate.zip>`. Serve that filesystem, `probe.zip`, and the rebuilt
single-threaded JIT runtime locally. Run the probe through `boxedwine.html` with:

```text
?root=candidate.zip&rootBase=/&app=probe.zip&appBase=/&p=display-modes.exe&storage=memory&resolution=800x600&bpp=32&wine-override=nomono&auto=true
```

Repeat with `resolution=1280x720`. The standard starting size checks duplicate
removal; the custom size checks preservation of the starting resolution.
Use a fresh in-memory guest for each run. To capture the process exit code,
launch this shell script from the mounted app instead of the EXE:

```sh
cd /home/username/.wine/dosdevices/c:/files || exit 99
/bin/wine /home/username/.wine/dosdevices/c:/files/display-modes.exe >/tmp/display-mode-probe.log 2>&1
status=$?
cat /tmp/display-mode-probe.log
echo PROBE_EXIT:$status
/opt/wine/bin/wineserver -k
/opt/wine/bin/wineserver -w
echo PROBE_CLEANUP:$?
exit $status
```

The probe checks enumeration, 32-bit formats, duplicates and invalid indices;
cycles twice through the three standard sizes and the starting size; verifies
device/adapter modes, backbuffer dimensions and four pixel readbacks; presents
each result; and checks restoration after a windowed reset and device release.
The mode list is rechecked after each fullscreen switch and after release.

## Local validation, 2026-11-12

Single-threaded JIT in the ChatGPT Chromium browser, Wine 11 web filesystem
revision 33 with only `C:/webgl/wined3d.dll` replaced:

| Starting resolution | Checks | Failures | Result |
| --- | ---: | ---: | --- |
| 800x600 | 366 | 0 | Three sizes, no duplicate starting mode; switches and restoration passed |
| 1280x720 | 426 | 0 | Four sizes, including the custom starting mode; switches and restoration passed |
| 1280x720, old v51 WineD3D control | 276 | 30 | Only the starting mode enumerated; all three standard sizes missing on each of ten enumeration checks |

The control could already switch and render at all sizes. Its failures isolate
the original mode-enumeration restriction. DOM observation confirmed that both
the SDL and WebGL canvases reached the requested fullscreen dimensions.
Both the control and candidate logged the existing WineD3D backbuffer-reference
warning during resets; the pixel, size and release assertions still passed.
Both candidate runs exited with status zero, completed wineserver cleanup and
reported no browser JavaScript errors.
This is focused D3D9 coverage, not a full DirectDraw/D3D8/game or native-platform
qualification. Existing broad Wine-test skips are retained in v52.

The Emscripten engine and probe compiled successfully; the native XRandR path
also passed a C++ syntax check. A clean Wine source checkout
accepted all 76 production patches. All eight PE32/i386 DLLs passed the
candidate manifest's size, SHA-256 and import validation. WineD3D was rebuilt
incrementally from a copy of the v51 build; the other seven DLLs were unchanged.

| Artifact | SHA-256 |
| --- | --- |
| Candidate `boxedwine.wasm` | `0bcaef47ffc97dea6f8a37273329f53843b259c5c0f0943761e22eb30de9d600` |
| Candidate `wined3d.dll` | `f06e4456908735c6374a1356c37d5961e8717beecc5fc635b0ec75ad6144179b` |
| Probe EXE | `7195fef2172da5996413c5996be5c12b0d15bf2d5ecb9202e88ccadb5de29a7a` |

Local runtime artifacts and raw browser logs are under
`tmp/webgl-display-modes/`. Releasing this requires deploying both the rebuilt
engine and a filesystem containing the candidate WineD3D; neither has been
published by this validation.
