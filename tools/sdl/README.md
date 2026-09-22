# Mac SDL dependency

Native Xcode builds use the official SDL **2.32.10** universal Mac framework,
pinned in `resources/sdl-mac.lock.json`. `tools/fetch_sdl.py` replaces the older
framework from the legacy Mac dependency archive before compilation/linking.
Other platforms retain their existing SDL dependencies.

The installer checks the disk image's size and SHA-256 before mounting it
read-only, then checks every framework file and symlink. It stages the complete
framework before replacing the previous version, serializes concurrent installs,
and restores the previous directory if replacement fails. Xcode signs only the
embedded copy. SDL now includes hidapi in the main binary rather than a nested
framework; the packaging script removes stale nested copies.

The default download cache is `~/Library/Caches/BoxedwineBuild/sdl`.
`BOXEDWINE_SDL_CACHE` overrides it. An installed matching framework needs neither
network nor a cache. To prepare explicitly, from the repository root:

```sh
python3 tools/fetch_sdl.py
python3 tools/fetch_sdl.py --archive /path/to/SDL2-2.32.10.dmg
python3 tools/fetch_sdl.py --offline
python3 -m unittest discover -s tools -p 'test_fetch_sdl.py'
```

## Hidden Metal renderer regression

SDL 2.0.14's hidden-window presentation path flushes drawing commands but skips
the Metal backend's present/commit. Destroying that renderer under Xcode's Metal
validation triggers `MTLCommandBuffer is in an invalid status when being destroyed`.
This caused background `winecfg /v win98` to abort after successfully configuring
Motorhead, before its installer could start. SDL 2.32.10 permits hidden rendering
on macOS and commits the buffer; no Boxedwine software-rendering workaround is
needed.

Run this native smoke test from the repository root in a logged-in Mac session:

```sh
mkdir -p tmp/sdl-smoke
clang++ tools/sdl/hidden-renderer.cpp -I lib/mac/SDL2.framework/Headers \
  -F lib/mac -framework SDL2 -Wl,-rpath,"$PWD/lib/mac" \
  -o tmp/sdl-smoke/hidden-renderer
MTL_DEBUG_LAYER=1 tmp/sdl-smoke/hidden-renderer
```

Expected: SDL 2.32.10 and a clean exit. The original 2.0.14 framework aborts in
the same test. Also exercise a copied Wine prefix using `-hideWindow`, configure
Windows 98 with Wine 11, terminate wineserver, and verify exit status zero with
Metal validation enabled. Normal visible installer rendering should still work.

This fixes SDL's configuration shutdown failure. It does not add the geometry
shader or shader cull-distance features required by upstream DXVK to MoltenVK.
