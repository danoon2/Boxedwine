# D3D8 CPU uploads to multisampled surfaces

`wine11_wined3d_msaa_upload.patch` implements the OpenGL texture upload path
needed by D3D8 `CopyRects` from an image surface into a multisampled render
target. Z: Steel Soldiers demo uses this for its splash screens with 4x MSAA.
Wine 11 previously returned without uploading the pixels and logged
`Not supported for multisample textures.` The exit presentation therefore
appeared black while its timer and mouse skipping continued to work.

## Fix

Multisample textures cannot receive `glTexSubImage*` uploads. The patch uploads
the data into a temporary single-sample texture with the destination format,
then performs an equal-size framebuffer blit into the requested destination
rectangle. Existing upload code handles source offsets, row pitch and format
conversion; existing framebuffer code preserves pixels outside a partial update.
The temporary texture is released after the copy. The requested sample count,
game configuration and Boxedwine OpenGL emulation are unchanged.

The patch is registered for Wine 11.0 in `wine_builds.json`, which covers the
regular Wine runtime in both full and web filesystem builds. It does not change
the separate WebGL PE DLL series or published archives. The affected OpenGL
multisample texture targets are not exposed by the browser renderer.

## Regression probe

Build the same executable for native Windows and Boxedwine/Wine:

```sh
i686-w64-mingw32-gcc -O2 -Wall d3d8-msaa-upload.c \
  -o d3d8-msaa-upload.exe -ld3d8 -luser32
```

Run directly on Windows, or place it in a Boxedwine prefix's `C:` drive and
launch it with `/bin/wine C:\d3d8-msaa-upload.exe`. No Wine overrides are needed.
Require `D3D8_MSAA_UPLOAD_PASS cases=12 skipped=0 failures=0` in the guest log.
Do not rely solely on the host engine's exit code. Unsupported sample counts
are reported as skips, so a passing run with skips is not equivalent coverage.

The 12 cases combine A8R8G8B8/RGB565, 0/2/4 samples, and copies inside/outside
`BeginScene`. Each checks all 4,096 pixels after a full upload and again after
a partial upload with nonzero source and destination offsets. The partial
check also verifies that surrounding pixels survive. Reading back a resolved
target checks that the uploaded color reaches every sample.

## Validation on November 13, 2026

| Runtime | Cases | Failed checks | Skips |
| --- | ---: | ---: | ---: |
| Microsoft Direct3D 8 on this Windows host | 12 | 0 | 0 |
| Boxedwine + original v35 WineD3D | 12 | 16 | 0 |
| Boxedwine + patched WineD3D | 12 | 0 | 0 |

The original build failed both pixel checks in all eight multisampled cases;
the four single-sample control cases passed. The patched build has no
multisample-upload warnings. This is supported Windows behavior, not a game
workaround.

The game was then tested in the isolated installed demo prefix with the normal
4x multisampling request and no `SampleCount` override. Both exit screens
(screenshot montage and full-game advertisement) rendered, including the fade.
The user also confirmed that previously missing intro images appeared.
This check covers splash rendering and exit, not a complete gameplay session.

Logs, test launch configurations and the test DLL are under
`C:\Boxedwine2\tmp\steel-soldiers-demo`. Relevant logs are `probe-windows.log`,
`probe-wine-original.log`, `probe-wine-upload.log` and
`native-exit-upload-fixed.log`. The test DLL SHA-256 is
`2ddfa110f3a03ac76857492586f77d0c58cebd9a9fbdd8db6a4124d46a13c312`.
Filesystem 36 now includes this fix. The final full archive also passes all
12 cases with no skips or failures; v35 has not been overwritten.
