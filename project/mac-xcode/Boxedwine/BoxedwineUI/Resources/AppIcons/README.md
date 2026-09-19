# Built-in app artwork

`notepad.png` is the transparent 256×256 representation from Wine 11.0’s
Notepad icon. It is bundled once for Try Notepad in the library, App Settings,
Removed Apps, and the running app’s Dock icon. Custom user artwork takes precedence.
The launcher does not need to unpack Wine or copy artwork into each app’s files.

Copyright © the Wine project authors. LGPL-2.1-or-later; see Wine-LICENSE.txt
and COPYING.LIB. The original, unmodified `notepad.ico` is included alongside
the converted PNG. Boxedwine converted the largest representation to PNG on
2026-09-13; the artwork was not changed.

Upstream source (Wine 11.0 release tag):
https://github.com/wine-mirror/wine/blob/wine-11.0/programs/notepad/notepad.ico

The PNG was extracted from `opt/wine/lib/wine/i386-windows/notepad.exe` in
filesystem-11 `TinyCore15Wine11.0.zip` (SHA-256
`a3367c4e977dbbe0295ce3b4b102bf7b0baa1278ea121cefe6065d561b960452`).

`winemine.ico` and `winemine.png` come from Wine 11.0’s Minesweeper executable
(`opt/wine/lib/wine/i386-windows/winemine.exe`) in filesystem-11
`TinyCore15Wine11.0.zip`, SHA-256
`52eaea9b29603ea9275c72e45526ea0f760efe87a6718f86fa2fa1146c976230`.
Boxedwine extracted the ICO and converted its largest image to a 128×128 PNG
using the launcher's WindowsIcon reader on 2026-09-15. The same Wine copyright
and LGPL-2.1-or-later notices above apply. No game artwork was changed.
