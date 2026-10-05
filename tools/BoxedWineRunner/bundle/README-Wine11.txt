Boxedwine automation: separate full Wine filesystem
==================================================

This archive contains the automation apps, recorded input, screenshot references,
performance scripts, and Java runner. It contains no filesystem ZIP or Boxedwine
executable. Extract it above the automation/ directory as before.

The launchers download the full Wine filesystem pinned in filesystem.properties,
check its exact byte count and SHA-256, and cache it beside automation/ in
automation-filesystems/. The current pin is full Wine 11.0, filesystem v14:
https://boxedwine.org/v2/14/TinyCore15Wine11.0.zip

The verified download is staged unchanged as fs/fs.zip. A separate fs/user.reg
preserves the source settings and applies these existing automation presets:

  [Software\\Wine\\Direct3D]
  "VideoMemorySize"="256"
  "DirectDrawRenderer"="gdi"
  "renderer"="gdi"

The runner seeds every fresh test root with that registry, including retries.
No mouse override is added. A failed download or hash check stops the launcher.

Windows (Java in PATH):
  runAll.bat C:\path\to\Boxedwine.exe
  runCinebench.bat C:\path\to\Boxedwine.exe

Linux/macOS (Java in PATH):
  bash runall.sh /path/to/boxedwine
  bash runCinebench.sh /path/to/boxedwine

Jenkins supplies its own Boxedwine build, rebuilt Java runner, and current
filesystem.properties from the checkout. It downloads the full filesystem
separately before running functional or Cinebench automation. Browser automation
has its own web filesystem pin and does not use this bundle's filesystem pin.

Application assets, input scripts, and screenshot references are unchanged from
the source bundle. validation.json records packaging checks, not a new runtime
qualification. Earlier documentation and qualification records are retained under
provenance/ and apply only to the older inputs recorded there.
