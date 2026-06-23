# Boxedwine Wine Filesystem Builder

`build_wine.py` builds a pinned Wine release and packages it into a Boxedwine filesystem zip. It is intended to make repeatable Wine release filesystems, usually for `.0` Wine releases such as `wine-11.0`, while keeping release-specific patches, cherry-picks, and reverts in `wine_builds.json` instead of hard-coding them in the script.

The current supported host environment is Debian or WSL running Debian/Ubuntu-style packages.

## Quick Start

From this directory:

```bash
python3 build_wine.py wine-11.0
```

Useful options:

```bash
python3 build_wine.py wine-11.0 --jobs 12
python3 build_wine.py wine-11.0 --config wine_builds.json
python3 build_wine.py wine-11.0 --skip-env-check
python3 build_wine.py wine-11.0 --skip-wineboot
```

The output is written as:

```text
Wine-<version>.zip
```

For example:

```text
Wine-11.0.zip
```

The script cleans its temporary build outputs at startup, but it leaves the completed Wine source/build tree afterward so build failures can be inspected.

## What The Script Does

The build is driven by `wine_builds.json`.

At a high level, the script:

1. Checks that the host environment has the commands and Debian packages needed for a 32-bit Wine build.
2. Downloads or validates the configured base filesystem zip.
3. Fetches or updates the Wine source checkout.
4. Checks out the requested Wine git tag.
5. Applies configured patches, cherry-picks, and reverts that match the requested Wine version.
6. Regenerates Wine bitmap TTF fonts when Wine's font sources are present.
7. Configures Wine with the configured compiler, linker, and `./configure` flags.
8. Builds Wine and installs it into a staging directory under `/opt/wine`.
9. Validates Wine `config.log` to make sure required features were really enabled.
10. Packages the staged `/opt/wine` files into the base filesystem zip.
11. Builds the native Linux Boxedwine binary.
12. Runs Boxedwine with a fresh temporary root and launches `/opt/wine/bin/wineboot -u`.
13. Disables Wine's X11 window-manager decoration/control settings in the generated registry.
14. Copies the initialized `/home/username/.wine` tree into the output zip.

The final zip includes:

```text
/opt/wine
/home/username/.wine
/wineVersion.txt
/name.txt
/build.txt
```

`changes.txt` and `version.txt` come from the base filesystem and are preserved.

## Base Filesystem Handling

`wine_builds.json` points at the current base filesystem:

```json
"base_filesystem": {
  "url": "https://boxedwine.org/v2/7/TinyCore15WineBase.zip",
  "filename": "TinyCore15WineBase.zip",
  "size": 67443325,
  "sha256": "46769cd240696968e31182bc86808b9038916fc44cdc3ef0ff217fd942ed44c0"
}
```

If `TinyCore15WineBase.zip` already exists locally, the script verifies that it is a valid zip and that it matches the configured size and SHA-256. If the file is stale, such as an older v5 base with the same filename, it is downloaded again before packaging.

If a config does not provide `size` or `sha256`, the script tries a remote `HEAD` request and compares `Content-Length` as a fallback.

## Wine Configuration

The default Wine build is a 32-bit Unix-style Wine build for Boxedwine:

```json
"cc": "gcc -m32",
"cxx": "g++ -m32",
"ld": "ld -m elf_i386",
"targetflags": "-m32",
"configure_args": [
  "--with-oss",
  "--with-opengl",
  "--with-vulkan",
  "--without-mingw",
  "--without-wayland",
  "--prefix=/opt/wine",
  "--disable-tests"
]
```

`--without-mingw` disables MinGW/PE Wine builds. This keeps the Wine filesystem closer to older Boxedwine filesystems. The script still builds Wine's generated `i386-windows` fake modules, but it does not require or run `i686-w64-mingw32-strip` in this default mode.

Required Wine features are listed in `required_config_checks`. After `./configure`, the script reads `config.log` and fails if key Boxedwine features such as FreeType, OpenGL, Vulkan, GnuTLS, GStreamer, SDL2, fontconfig, or OSS support were not enabled.

## Patches And Backports

Patches and release-specific operations live in `wine_builds.json` under `operations`.

Supported operations:

```json
{ "apply_patch": "file.patch" }
{ "cherry_pick": "commit_hash" }
{ "revert": "commit_hash" }
```

Operations can be limited to a Wine version range:

```json
{
  "when": { "min": "9.3", "max": "10.8" },
  "cherry_pick": "6fee37c2b6eb486ae60d6af4dd6e45549be0b624",
  "reason": "Backport Wine fix used by previous Boxedwine 9.x and 10.x builds"
}
```

Patch files live in `patches/`. On WSL, if the Boxedwine checkout is on a Windows filesystem and patch files have CRLF line endings, the script normalizes those patch files into `tmp_patches/` before applying them.

## Debian Packages

On a new Debian or WSL environment, enable i386 packages first:

```bash
sudo dpkg --add-architecture i386
sudo apt update
```

Install the packages used by the default `--without-mingw` build:

```bash
sudo apt install \
  build-essential \
  git \
  gcc-multilib \
  g++-multilib \
  libc6-dev-i386 \
  libx11-dev:i386 \
  libxext-dev:i386 \
  libxrender-dev:i386 \
  libxrandr-dev:i386 \
  libxi-dev:i386 \
  libxcursor-dev:i386 \
  libxinerama-dev:i386 \
  libxxf86vm-dev:i386 \
  libfreetype-dev:i386 \
  libgl-dev:i386 \
  libegl-dev:i386 \
  libvulkan-dev:i386 \
  libgnutls28-dev:i386 \
  libgstreamer1.0-dev:i386 \
  libgstreamer-plugins-base1.0-dev:i386 \
  libsdl2-dev:i386 \
  libfontconfig-dev:i386 \
  pkg-config \
  flex \
  bison \
  fontforge \
  gettext \
  libsdl2-dev \
  libssl-dev \
  libminizip-dev \
  libcurl4-openssl-dev \
  zlib1g-dev \
  libgl-dev
```

The non-`:i386` SDL/OpenSSL/minizip/curl/zlib/OpenGL packages are for building the native Linux Boxedwine binary that runs `wineboot`. The `:i386` packages are for Wine itself.

If you remove `--without-mingw` and want MinGW PE Wine builds, also install:

```bash
sudo apt install binutils-mingw-w64-i686
```

The environment checker prints a package hint if it finds missing commands or common missing 32-bit compile/link support.

## OSS Sound Header

The default config uses `--with-oss`. The script stages the OSS header from:

```text
oss/soundcard.h
```

into `tmp_oss/` and sets `OSSLIBDIR` for Wine configure. If `--with-oss` is enabled and the header is missing, the script fails before rebuilding Wine.

## Output And Diagnostics

The script writes `build.txt` into the output filesystem. It records:

- requested Wine tag
- Wine version
- base filesystem name
- output zip name
- git operations
- configure environment and command
- build/install commands

Common temporary paths:

```text
wine-git/       Wine source and build tree
tmp_install/    staged filesystem overlay
tmp_root/       temporary Boxedwine root for wineboot
tmp_patches/    CRLF-normalized patch files
tmp_oss/        staged OSS header tree
```

These paths are cleaned at the start of each run as needed, but the Wine build tree remains after the run for inspection.
