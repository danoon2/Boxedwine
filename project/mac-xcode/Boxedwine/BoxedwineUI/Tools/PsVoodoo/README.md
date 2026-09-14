# Building the Boxedwine psVoodoo fork

The maintained source, build tooling, and graphics regressions now live in [danoon2/psVoodoo](https://github.com/danoon2/psVoodoo). This repository's `build.py` checks an exact source commit and delegates to the fork's build script. It does not apply source patches.

Pinned commit: **`aaa990e330554008980b9248277fcfd8f014bddc`**.

```sh
git clone https://github.com/danoon2/psVoodoo.git psvoodoo-source
git -C psvoodoo-source checkout aaa990e330554008980b9248277fcfd8f014bddc
python3 build.py --source /path/to/psvoodoo-source \
  --toolchain /path/to/llvm-mingw/bin --output /path/to/new-build \
  --d3d9-only --build-probes
```

Use a clean checkout and a new output directory. The tested compiler is LLVM-MinGW `20260908`, UCRT, macOS universal. The `--d3d9-only` option matches the tested WineD3D OpenGL configuration with Wine 11 and Boxedwine filesystem ABI 11. The fork retains the original gamma shader and the optional upstream D3D9Ex path for other builds.

The fork contains the utility texture-memory query, framebuffer color, clipping-order, and detail-alpha fixes as separate commits. Its tests cover the clipping rows, 48 detail/blending panels, and every RGB565 color plus partial updates. Build output includes source hashes, Git revision, compiler commands, PE inspection, and DLL fingerprint. See the fork's [README](https://github.com/danoon2/psVoodoo#readme) and [test instructions](https://github.com/danoon2/psVoodoo/blob/main/tests/README.md).

When updating the dependency, verify the new fork commit, run the relevant regressions/game checks, then update `REVISION` in `build.py` and this document together. Updating the source pin does not replace a shared Wine ZIP or change the demo catalog; the packaged filesystem's checksum must still be recalculated after an upload.

The earlier build remains reproducible through `build-upstream-snapshot.py` and the retained patch files. [BUILD_HISTORY.md](BUILD_HISTORY.md) records those experiments. These snapshots are historical; make future fixes in the fork.
