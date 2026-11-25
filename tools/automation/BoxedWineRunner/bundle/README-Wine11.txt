Boxedwine automation: separate full Wine filesystem
==================================================

This archive contains the automation apps, recorded input, screenshot references,
performance scripts, and Java runner. It contains no filesystem ZIP or Boxedwine
executable. Extract it above the automation/ directory as before.

The launchers download the full Wine filesystem pinned in filesystem.properties,
check its exact byte count and SHA-256, and cache it beside automation/ in
automation-filesystems/. The current full Wine 11.0 filesystem URL, size and
SHA-256 are selected by the accompanying filesystem.properties.

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
separately before running functional or Cinebench automation. Browser app
automation uses this same asset bundle and the checkout's full filesystem pin.

Native and browser automation read the packaged recordings and screenshot
references directly. This archive includes the tested Daytona and Firefight
installer recovery changes and Firefight's cropped completion-message reference;
no runtime recording patches or image crops are applied. App executables and
gameplay references are unchanged.

validation.json records packaging checks. Recording changes and their input
hashes are recorded under provenance/, alongside earlier documentation and
qualification records that apply only to the inputs recorded there.
