# Pinned Mac MoltenVK

Native Xcode builds install the standard public-API MoltenVK **1.4.2** artifact
pinned in `resources/moltenvk.lock.json`. The `Boxedwine` target prepares it before
linking; `BoxedwineUI` embeds and re-signs it. SDL is prepared from its own
[Mac dependency pin](../sdl/README.md). No graphics default or guest Wine package is changed.

The official library is universal arm64/x86_64, with a macOS 12 minimum; the
native app still requires macOS 15. The private-API artifact is not used.

`tools/fetch_moltenvk.py` verifies both archive and library sizes and SHA-256s,
reads only the named regular tar member, and atomically replaces the old library.
An already matching library requires no network. Downloads are cached under
`~/Library/Caches/BoxedwineBuild/moltenvk` (override with
`BOXEDWINE_MOLTENVK_CACHE`). A missing pin/cache requires a connection and fails
clearly if unavailable, rather than silently using a different version. A failed
fetch leaves the previous library intact. No download takes place in the app.

For a downloaded archive, or an explicitly offline developer check:

```sh
python3 tools/fetch_moltenvk.py --archive /path/to/MoltenVK-macos.tar
python3 tools/fetch_moltenvk.py --offline
python3 tools/test_fetch_moltenvk.py
```

The source revision, external dependency revisions, archive hashes and notices
are recorded in `project/mac-xcode/Boxedwine/BoxedwineUI/Licensing/native-dependencies.json`
and `native-notices.json`. When updating the pin, review and update those records
and notices together. Retain the corresponding source archives with release
materials; this downloader installs only the binary build input.

## Native GPU smoke test

From the repository root, after fetching dependencies:

```sh
mkdir -p tmp/moltenvk
xcrun clang++ -std=c++17 tools/moltenvk/probe.cpp \
  -I source/vulkan/vk -I lib/mac/SDL2.framework/Headers \
  -F lib/mac -framework SDL2 -L lib/mac/vulkan/lib -lMoltenVK \
  -Wl,-rpath,"$PWD/lib/mac" -Wl,-rpath,"$PWD/lib/mac/vulkan/lib" \
  -o tmp/moltenvk/probe
MVK_CONFIG_LOG_LEVEL=1 tmp/moltenvk/probe "$PWD/lib/mac/vulkan/lib/libMoltenVK.dylib"
```

This briefly creates an SDL Vulkan window, initializes the first GPU, clears an
image, checks all 256 read-back RGBA pixels, and presents a swapchain image. It
exercises SDL's direct library loading and surface creation, not the system
Vulkan loader. It does not test shader compilation, guest Vulkan marshaling,
DXVK game compatibility, or other GPU/OS combinations. Failures terminate the
probe without touching app libraries or user files.
