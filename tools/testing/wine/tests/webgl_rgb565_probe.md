# RGB565 transfers in WebGL

`d3d9_rgb565_upload_probe.c` checks managed and dynamic RGB565 uploads,
all 65,536 packed colors, padded rows, a nonzero mip level, partial updates,
and GPU-to-system-memory RGB565 readback. It writes `C:/rgb565-results.txt`.
Compare the CASE hashes between the published and candidate DLLs as well as
requiring zero failures. The mip level is a constant color to avoid ambiguous
point-sampling ties when scaling 128 pixels to the 257-pixel render target.

Build in WSL:

```sh
i686-w64-mingw32-gcc -O2 -Wall -Wextra \
  tools/testing/wine/tests/d3d9_rgb565_upload_probe.c -ld3d9 -o rgb565-probe.exe
```

Package the EXE under `home/username/.wine/drive_c/rgb565-probe.exe` and the
selected DLL under `home/username/.wine/drive_c/webgl/wined3d.dll` in a temporary
overlay. Launch with the Wine 11 WebGL root, `p=c:/rgb565-probe.exe`, and
`storage=memory`. Use installed Chrome and test each affected engine target.
The environment parameter must preserve the literal colon:
`em-env=%22BOXEDWINE_WEBGL_THREAD_CANVAS:0%22`. Verify the runtime ENV value is
`0`; the current shell parser does not decode `%3A` in this parameter.

The three BENCH passes each dirty and upload twenty full 800x600 textures,
draw, and flush through readback. The three READBACK passes each draw and
read ten 800x600 RGB565 render targets. Divide by 20 or 10 respectively.
These are transfer probes, not game FPS. Run baseline and candidate
sequentially without other active test tabs when comparing performance.

Local v58 candidate artifacts, pixel checks, exact DLL hashes, screenshots,
and replay server are in
`test-results/mechcommander2-st-conversion-2026-11-24/`. The candidate changes
only WineD3D; it is not a published filesystem revision.

The packed transfer paths live in the existing RGB565 upload-scaling and
color-readback-loop patches. The v58 manifest updates their hashes without
adding production patches; the series still contains 80.

Filesystem revision 48 packages the tested WineD3D DLL in both full and web
archives. Final-archive checks and screenshots are retained in
`tmp/filesystem-v48-release/`. The EXE-only probe overlay leaves the packaged
DLLs in use. MechCommander 2 also requires engine commit `39a6f6599` or later
and `BOXEDWINE_WEBGL_THREAD_CANVAS=0`.
