# Wine 11.0 filesystem revision 28

Built November 8, 2026. Local candidates; publication and final packaged-game
qualification remain pending. This release adds PCRE2 and Windows Media playback
dependencies, targets GLib 2.78, and includes the ASF writer availability stub
and reader seek/preroll fixes. Movie encoding remains unsupported.

## Artifacts and upload destinations

Local directory: `C:\Boxedwine2\tmp\filesystem-v28-release`.

| File | Bytes | Upload path |
| --- | ---: | --- |
| `TinyCore15WineBase.zip` | 72,005,312 | `/v2/28/TinyCore15WineBase.zip` |
| `TinyCore15Wine11.0.zip` | 217,088,401 | `/v2/28/TinyCore15Wine11.0.zip` |
| `TinyCore15Wine11.0-web.zip` | 96,903,258 | `/v2/28/TinyCore15Wine11.0-web.zip` |
| `TinyCore15WineBase-media-sources.zip` | 25,647,064 | `/v2/28/TinyCore15WineBase-media-sources.zip` |
| `filesV2.hosted-v28.xml` | | `/v2/26R2/filesV2.xml` |

SHA-256 identities, in the same archive order:

```text
bb7bf1807721deafaf98dff10091f27b21e5471d357c7b07330b84cc299aee83  TinyCore15WineBase.zip
f4ee64fa4568a96bfa3dd540d72c63155167716d872485bc81547f56beae9d8b  TinyCore15Wine11.0.zip
0ef37445a4d31af044a435e6b68f79ba23eaa7197655106f165d89ef5caf950c  TinyCore15Wine11.0-web.zip
877da89fd04fd54f87de6bda54c43b705c70dba0789c4713aeec9738dce3b905  TinyCore15WineBase-media-sources.zip
```

After qualification, upload and verify the versioned archives, copy the exact
main/web bytes to their matching `/v2/latest/` filenames, then publish the hosted
catalog and activate the repository/UI/Jenkins pins. Preserve older versioned
archives. The source bundle contains upstream archives and the media-base build
script/settings; link it alongside the downloads. It is not a runtime dependency.

## Build and engine dependencies

Both Wine variants were rebuilt at
`db11d0fe6a169c457e23d007e20404643d067aa8` targeting GLib 2.78. The subsequent
ASF reader correction was rebuilt in both trees and included in fresh packages
and prefixes. WineGStreamer imports `g_once_init_enter` / `g_once_init_leave`,
without the GLib 2.80 pointer variants. The v51 WebGL DLL identities remain
unchanged from revision 27.

The base uses the prebuilt PCRE2 10.42 8-bit library and selected GStreamer
1.16.2 plugins from pinned TinyCore extensions, plus a minimal FFmpeg 4.4.6
decoder build. See [build_media_base.py](build_media_base.py) for exact inputs
and flags. No Microsoft media DLLs, game data, encoders, or FFmpeg command-line
programs are added. Web recompression preserves every member and its permissions.

Playback also requires the engine's FPU initialization-order and Linux pthread
floating-point inheritance fixes. The tested Windows x64 engine is:

`C:\Boxedwine2\tmp\movies-writer-stub-20261108\media-engine\BoxedwineEngine.exe`

SHA-256: `2f305611dd6ac56590c8e35aebcaa306cc46786c645dd51509c3b6aca1fb3179`.

Deploy rebuilt platform engines with these fixes before activating the new
filesystems. Updating a ZIP does not update an installed or hosted engine.

## Completed validation and remaining checks

- Both final archives pass CRC, PE/import, GL/cache and manifest validation.
  The main archive has 7,143 entries; web has 4,666 and remains below 100 MB.
- Both packaged URI/Temp smoke checks pass; main also passes Gecko
  HTML/JavaScript/layout checks.
- Fresh roots using each final ZIP and the updated Windows engine pass the
  writer contract, full WMV decode (429 audio/video samples through EOF), and
  DirectShow graph probes. Graph checks cover stopped seeks, completed preroll,
  playback, EOS, Stop/restart, and seeking during playback. No native Microsoft
  media DLLs or loose runtime overrides are present in these probe roots.
- The pthread floating-point probe changes from three failures to zero; the
  full Linux engine suite passes 873 tests. Builder and web tooling suites pass.
- The Movies reached its tutorial studio lot with the writer stub and builtin
  media playback during the reader-fix experiment. That run used a loose rebuilt
  Wine `qasf` module; it is preliminary evidence, distinct from the final-ZIP
  probe results. Final packaged-game visual verification was interrupted when
  Computer Use was stopped and remains pending.

Browser execution, macOS/Linux game execution, full gameplay, saving/loading,
and export failure handling were not qualified for this release. The writer
supports creation and graph inspection but explicitly rejects encoding.

Probe sources and reproduction notes are in
[probes/movies-writer-stub](probes/movies-writer-stub/README.md).
Local logs are in `tmp/movies-writer-stub-20261108`; artifact validation logs
and sidecars are in `tmp/filesystem-v28-release`. Generated binaries, archives,
game registry data, and diagnostic roots are not source-control inputs.
