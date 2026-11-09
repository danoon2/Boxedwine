# Wine 11.0 filesystem revision 27

Built November 8, 2026 on `james/sims2`. Staged locally; upload is pending.
Both variants include the Sims 2 extended software/mixed vertex float constants,
sparse uploads, context-safe buffer cleanup, the pre-Vista A8 render-target
fallback, and a 512 MiB video-memory default for fresh prefixes.

## Artifacts and upload destinations

Local directory: `C:\Boxedwine2\tmp\filesystem-v27-release`.

| Local artifact | Bytes | Upload path |
| --- | ---: | --- |
| `TinyCore15Wine11.0.zip` | 215,235,475 | `/v2/27/TinyCore15Wine11.0.zip` |
| `TinyCore15Wine11.0-web.zip` | 99,650,033 | `/v2/27/TinyCore15Wine11.0-web.zip` |
| `filesV2.hosted-v27.xml` | | `/v2/26R2/filesV2.xml` |

Hashes are pinned in [the release manifest](webgl_filesystems_v27.json) and
the staging directory's `SHA256SUMS.txt` / `release-manifest.json`.
Copy the exact ZIP bytes to the matching `/v2/latest/` filenames too.
The staged hosted catalog preserves the other Wine versions and demos.

## Web engine dependency

Deploy rebuilt browser engines from this branch with v27. WebGL 2 has uniform
buffers, but the previous engine's desktop-style OpenGL 3.0 advertisement did
not expose `GL_ARB_uniform_buffer_object`, so Wine could not use them. This
branch adds that capability name. The old engine fails 22 extended-constant
pixel checks; the updated engine passes. Updating a filesystem alone does not
replace an already deployed engine. All four normal browser modes were rebuilt.

The v51 WebGL series also sizes its constant deduplication cache for the full
device register bank. The expanded probe checks restoring a low constant after
an update across the old 256-register boundary. The original test policy stays
unchanged. Both archives contain all eight freshly rebuilt WebGL DLLs; D3D8,
D3D9, DirectDraw and WineD3D change from v26, while DXGI and the three D3DX DLLs
remain byte-identical. Both regular Wine variants were rebuilt completely.

## Completed validation

- Clean Wine 11.0 builds at `db11d0fe6a169c457e23d007e20404643d067aa8`, applying all 32 selected patches; clean v51 WebGL build with 75 production patches and the selected test adaptation.
- Fresh addons, prefix setup and final ZIPs; full Gecko installation and packaged HTML/JavaScript/layout smoke.
- Both packaged URI/Temp smoke tests, exact archive hashes/CRCs, DLL/import and GL shim pins, patch sources, registry defaults, directory permissions and inventories. No files or directories disappear compared with v26. The web ZIP is below 100,000,000 bytes.
- Native Windows, both final ZIPs in fresh roots: 544 SWVP/A8 checks and 17 GL cleanup checks per variant, 512 MiB reported memory, and four A8 configurations per variant (XP/Windows 10 defaults and explicit reverse overrides, 80 checks total).
- Packaged browser rendering: 24,046 checks per run, zero failures/skips in five runs (full ST JIT; web ST/MT with and without JIT). Includes SWVP, constant caching, dynamic buffers, 76 indexed-draw cases and 5,120 signed-texture pixels.
- Builder (103), web tooling (31), site (20), local site (7), publish site (12), Linux UI (76), Windows UI core (50), and Java runner regressions pass.
- Updated native catalog, Jenkins, runner and demo-site pins; staged Mac App Store/Direct resources and demo site using the final candidates. These resource directories are not finished native installers.

## Upload and remaining qualification

Upload the two ZIPs to `/v2/27/`, refresh `/v2/latest/`, then upload the hosted
XML to `/v2/26R2/filesV2.xml`. `verify-published.ps1` downloads and checks those
exact identities after upload. Activate the repository/UI/Jenkins pins only
after the URLs are available. Deploy the rebuilt browser runtimes with the new
web root. Existing applications retain their saved package records and local
overrides; new packages are not installed into them by this build.

Sims 2 gameplay was not repeated against the final ZIPs; the earlier local-DLL
household smoke remains historical evidence. Native functional/Cinebench
workers, AbiWord's four automation modes, Mac/ARM execution, and clean hosted
downloads remain unqualified for these artifacts. automation36, the AbiWord
overlay, demo catalog and earlier filesystem identities are retained unchanged.

The staging directory retains source/build identities, logs, inventories,
sidecars, the old-engine failing control, fixed probe results, and
`validation-summary.json`. Follow [RELEASING.md](RELEASING.md) for the full
consumer checklist. No upload, commit or push was performed.
