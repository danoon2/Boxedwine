# CNC DDraw OpenGL loader fix

The Wine filesystem contains CNC DDraw 6.9, revision
`a902db06e9830a9feafda69da05c766a81722b9b` from
[FunkyFr3sh/cnc-ddraw](https://github.com/FunkyFr3sh/cnc-ddraw).

This patch loads `glGetIntegerv` from `opengl32.dll`. It is an OpenGL 1.1
export; Wine returns NULL when CNC DDraw requests it through
`wglGetProcAddress`. Without it, CNC DDraw falls back to
`glGetString(GL_EXTENSIONS)` inside its OpenGL core context, causing
`GL_INVALID_ENUM` and a switch to GDI. On Boxedwine's Mac backend that
leaves an empty OpenGL window in front of the GDI output.

Build the matching source, after applying the patch, with LLVM-MinGW:

```sh
git clone https://github.com/FunkyFr3sh/cnc-ddraw.git /tmp/cnc-ddraw
git -C /tmp/cnc-ddraw checkout --detach a902db06e9830a9feafda69da05c766a81722b9b
git -C /tmp/cnc-ddraw apply /absolute/path/to/Boxedwine/tools/cnc-ddraw/patches/0001-load-glGetIntegerv-from-opengl32.patch
python3 tools/cnc-ddraw/build.py --source /tmp/cnc-ddraw \
  --toolchain /absolute/path/to/llvm-mingw/bin --output /tmp/cnc-ddraw-release
```

The build is a 32-bit release DLL, marked `a902db0-boxedwine1`, with no debug
logging. Put it at `home/username/.wine/drive_c/ddraw/ddraw.dll` in the Wine
filesystem. Keep CNC DDraw's MIT license with the distribution. MDK needs no
additional INI change: `renderer=auto` selects OpenGL under Wine.

Verification uses MDK Performance on Wine 11: the original debug DLL reports
a NULL `glGetIntegerv`, error `0x500`, and GDI fallback. With only this loader
fix, the function is non-NULL, CNC's texture/shader self-tests pass, and the
benchmark renders through OpenGL. No Boxedwine OpenGL error suppression is
required. Runtime evidence and release artifacts are under
`tmp/native-ui-test/mdk-opengl/`.

Norse By Norse West using CNC on the current Mac Wine 11 package needs a 32-bit
desktop (`-bpp 32`) and this per-game section in `ddraw.ini`:

```ini
[NORSE95]
fake_mode=320x240x16
```

With the demo's old `-bpp 16`, even the patched DLL fails before OpenGL
initialization and falls back to GDI. Changing only the desktop to 32-bit
allows OpenGL to start but leaves a black picture with 640x480 surfaces.
The mode override keeps Norse's drawing surfaces at 320x240x16 while OpenGL
uses the 32-bit desktop. Diagnostic logs confirm Apple's OpenGL 4.1 core
context and no GDI fallback with this combination; the release DLL renders
the intro and menu without the software-rendering warning. Wine's default
OpenGL backend is used; GLX with a 16-bit desktop did not resolve the issue.

The September 13 Wine filesystem now contains the patched release DLL.
MDK's catalog recipe uses automatic rendering again, with uncapped timing.
Game-specific overrides live in each app's private INI, seeded by its demo
recipe. Norse's schema-7 recipe supplies `CNCDDrawFakeMode=320x240x16` and
`BitsPerPixel=32`; it creates or updates `[NORSE95]` in the app's private
`C:/ddraw/ddraw.ini`, retaining all other package defaults and sections.
The shared filesystem does not need this Norse section. Evidence:
`tmp/native-ui-test/norse-opengl/` and
`tmp/native-ui-test/wine-refresh-cnc-20260913/`.

A subsequent test disabled CNC for the existing Norse installation and
restored `-bpp 16`. The user confirmed that it works well with Wine's default
renderer; no explicit Wine GDI override was applied. The earlier black-window
capture was temporary, despite Wine logging an OpenGL context initialization
failure. This configuration does not use the CNC INI section. The installed
copy was subsequently restored to CNC at the user's request to retain its
scaling. The recipe now seeds the private mode setting automatically.
Evidence: `tmp/native-ui-test/norse-without-cnc/`.

Recipe/import evidence: `tmp/native-ui-test/norse-cnc-recipe/`.
