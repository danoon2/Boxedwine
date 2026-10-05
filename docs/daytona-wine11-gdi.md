# Daytona USA demo: Wine 11 GDI buffer lifetime

Diagnosed on 2026-09-29 using Wine 11.0 filesystem version 13 and the September
29 Boxedwine release executable. The original Daytona container and installed
filesystem ZIP were used as read-only inputs; experiments used copied roots.

## Cause

The game uses DirectDraw's GDI renderer. WineD3D creates persistent device
contexts (`WINED3DUSAGE_OWNDC`) whose bitmaps reference the texture's system-memory
allocation. After presentation, the command stream marks the back buffer
discarded. `wined3d_texture_validate_location()` then calls
`wined3d_texture_evict_sysmem()`, freeing that allocation even though the GDI
bitmap still references it. The next flip uses freed memory; a following
front-buffer blit writes through a null `resource.heap_memory` pointer.

The standalone probe reproduces this with two flips and a front-buffer color
fill. The exact same null write occurs under native Wine in WSL, establishing
that Boxedwine is not required to trigger this failure.

## Fix and validation

`tools/buildWine/patches/wine11_wined3d_owndc_sysmem.patch` excludes OWNDC textures
from system-memory eviction. Their allocations remain valid for the lifetime
of the persistent GDI DC and are freed during ordinary resource destruction.
The Wine 11 build configuration applies this patch. The changelog retains
filesystem version 13. On 2026-09-29, the tested WineD3D module was installed in
`C:\Users\james\AppData\Roaming\Boxedwine\FileSystems2\TinyCore15Wine11.0.zip`.

`tools/buildWine/probes/ddraw-flip.c` is an independent DirectDraw regression:

```sh
i686-w64-mingw32-clang -O2 -static ddraw-flip.c -o ddraw-flip.exe \
    -lddraw -ldxguid -luser32
WINE_D3D_CONFIG=renderer=gdi wine ddraw-flip.exe
```

- Unpatched Wine 11: access violation on frame 0, after the second flip,
  in both native WSL Wine and Boxedwine.
- Patched Wine 11: `DDRAW_FLIP_PASS` in both environments, completing all eight
  iterations / sixteen flips and the front-buffer blits.
- Daytona with a patched WineD3D root overlay: survives the 55-second startup
  test. A separate visible run reaches the Daytona USA title screen without
  game input. The original build exits in about 13 seconds.
- Filesystem/build configuration tests: 98 passed.
- Packaged DLL in a fresh Boxedwine root, without a loose module override:
  `DDRAW_FLIP_PASS`, completing eight iterations / sixteen flips. All 370
  explicit ZIP directories were preserved. The installed archive's SHA-256 is
  `d1b063b46033445d9c8479610e2064d01a8e80a6e15e080e15befe957e8a77da`.

Packaging, ZIP-based regression, and installation records are in
`tmp/filesystem-v13-daytona-20260929`. That directory also contains the previous
archive as `TinyCore15Wine11.0-before-daytona.zip`.

The user's Wine 9 result was not independently confirmed: the local Wine 9
comparison encountered prefix setup and a Vulkan-stub compatibility failure
before completing the test. Native Wine 11 reproducing the same null write is
the decisive evidence for ownership of this bug.

Artifacts and launch scripts are in `tmp/daytona-20260929`. The effective patched
native run is `native-flip-final.log`; the effective patched Boxedwine run is
`boxed-flip-patched.log`. Earlier attempts named `*-fixed` did not override Wine's
installed DLL, because `WINEDLLPATH` is searched after its installation directory.
For native testing, an isolated installed-runtime copy supplies the patched
module. For Boxedwine, the test root overlays
`/opt/wine/lib/wine/i386-unix/wined3d.dll.so` directly.

## Separate WebGL PE DLL

The filesystem also contains `home/username/.wine/drive_c/webgl/wined3d.dll`,
built separately with the DirectX-to-WebGL series. The v41 DLL reproduced the
same null write after two flips with `renderer=gdi`. Updating the main ELF
WineD3D did not update this PE DLL.

The v42 series appends
`tools/d3dToWebGL/webgl-gdi-owndc-sysmem-against-wine-11.0.patch` with the same
OWNDC guard. All 84 modified/new source files in the cached build matched a
fresh replay of the complete series. Only WineD3D required recompilation;
the other seven WebGL DLLs retain their previous hashes. The full filesystem
builder now selects `webgl_filesystems_v14.json`, retaining those v42 DLL hashes.

The rebuilt PE DLL has SHA-256
`261c38bb2f5d0c6f3bda46386c711a894354896aee2401322b6cda520243e9e1`.
It passes eight iterations / sixteen flips with both `renderer=gdi` and
`webgl=1,renderer=gdi`, loaded directly from the candidate ZIP in fresh roots
without loose DLL overrides. ZIP CRC, PE/import/hash, graphics-library and
library-cache validation passed, as did all 118 build/packaging unit tests.
This validates the affected GDI path; the browser graphics matrix was not rerun.

The version 13 archive containing both WineD3D fixes has SHA-256
`5e0ae4efd4b48ea6293ad2c897004c7a5616e64e86a2236aca814329a06dbf29`.
All 370 explicit directory entries are preserved. Build, packaging, test and
installation records are in `tmp/daytona-webgl-20260929`, alongside the backup
`TinyCore15Wine11.0-before-webgl-gdi.zip`.
