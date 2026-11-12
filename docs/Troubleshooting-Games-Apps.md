If your app or game does not work, the first thing you should do is check out https://appdb.winehq.org/. If you can find your app or game on there, it should give some hints that might be useful. If it does not work on Wine then there is a good chance it won't work on Boxedwine either.

###  Things to try
For a game made before 2007, try setting its Windows version to Windows XP
and restarting it, especially if characters have black rectangular shadows.
For games made before 2000, Windows 98 is another setting to try.
These are troubleshooting steps, not requirements for every older game;
restore the previous version if the change causes other problems.

With this branch's Wine 11 patch, XP and earlier also select a more compatible
shadow-format default. This behavior is pending the next Wine filesystem
release; changing the Windows version in an existing v26 package does not
enable the new workaround by itself.

### Mouse capture

Native desktop builds capture games that repeatedly recenter a hidden cursor,
including Portal 2 with raw input disabled, even when Wine only grabs the X11
pointer briefly around each recenter. Detection also handles PointerRoot focus
and Wine's separate display connections for windows and input.
A single cursor reposition or a visible menu cursor does not trigger capture.
Showing the cursor, changing focus, or closing the game window clears detection.
Menu/gameplay cursor changes also refresh when Wine sets the cursor on a parent
window, so the menu pointer reappears without needing to leave the game surface.

Press Ctrl+Alt+G (Ctrl+Option+G on macOS) to release a captured pointer, then click
inside the game to resume. The status bar shows the current capture state.

###  Graphical Issues
Some older DirectDraw games, like Diablo and Starcraft, might not draw Windows common controls correctly or perhaps not at all. Checking the box on the container options for "DD GDI Renderer" could help. This option will have no affect on Direct3D and OpenGL games.

### RalliSport Challenge

A visible cursor with an unresponsive main menu after the intro can be caused
by a missing GStreamer WAV parser. In the tested Wine 11 v31 filesystem,
DirectShow could not play the game's PCM WAV menu music, leaving the main thread
looping through failed playback. Installing `libgstwavparse.so` restored menu
input and allowed a race to run. CPU affinity and Windows XP mode did not fix
this case; DirectPlay was not needed for the menu fix.

Filesystem v33 includes the parser in the base, full and web archives. It is
staged locally; see the [release receipt](../tools/buildWine/RELEASE-v33.md) for
package identities and publication status. Existing application package pins
are preserved, so the application must select the new package to receive it.

### The Sims 2 Double Deluxe

New filesystem builds default WineD3D's reported video-memory budget to
512 MiB through the string `HKCU\Software\Wine\Direct3D\VideoMemorySize`.
This avoids advertising modern multi-gigabyte GPUs to older games that probe
memory by allocating textures until failure. Per-executable values under
`HKCU\Software\Wine\AppDefaults\<exe>\Direct3D` take precedence. This builder
change is not yet in the staged v26 archives or existing installed prefixes.

On a high-refresh monitor, the "Failed to find any DirectX 9.0c compatible
graphics adapters" dialog can mean that no 60 Hz fullscreen mode was advertised.
The game can identify the GPU and shader capabilities successfully and still
stop before `IDirect3D9::CreateDevice`. Boxedwine's virtual XRandR modes now
include 60 Hz alongside the host refresh rate; the host monitor is not switched.
The game's `-w` argument also bypasses fullscreen mode selection.

The separate WineD3D startup failure `D3DERR_INVALIDCALL` occurs when this version
of the game enables software vertex processing and uploads 1024 vertex shader
constants. Stock Wine 11's D3D9 implementation rejects counts above 256.
The Wine 11 patches on this branch preserve 8,192 software/mixed-device float
constants and use OpenGL uniform buffers to execute shaders that read them.
This fixes the game's startup and household rendering on the tested native
OpenGL host; it is not a complete CPU software vertex processor.

With those patched Wine modules installed, set the per-game string
`HKCU\Software\Wine\AppDefaults\Sims2SP4.exe\DllOverrides\d3d9` to `builtin`.
In that executable's `Direct3D` key, string `VideoMemorySize=256` is the tested
per-game memory override. To avoid black rectangles under Sims, select Windows
XP and restart the game. With no explicit `DisableA8` setting, the patch hides
A8 render-target support for XP and earlier; Vista and later retain normal
support. Ordinary A8 textures remain available, and the game keeps High shadows.
For an explicit override on any Windows version, set DWORD `DisableA8` to `1`
to hide A8 render targets or `0` to retain normal support. Per-game overrides
take precedence over global settings, and `WINE_D3D_CONFIG` takes precedence
over both. Stock Wine does not recognize this option. This is a compatibility
policy, not a claim that XP hardware lacked A8 render targets.

See [the implementation and test notes](sims2-wined3d.md) for limitations,
regression results, and local development overrides. The package ZIP has not
been replaced, and the changes have not yet qualified a filesystem release.

As an alternative on a Vulkan-capable native host, the filesystem includes
DXVK in `C:\dxvk`.
Copy its x32 `d3d9.dll` beside `Sims2SP4.exe` and set the game-specific Wine
override `HKCU\Software\Wine\AppDefaults\Sims2SP4.exe\DllOverrides`, string
`d3d9=native`. An adjacent `dxvk.conf` containing
`d3d9.maxAvailableMemory = 256` bounds the game's startup texture-memory probe.
This uses Vulkan regardless of the UI's host OpenGL selection. Back up any
existing DLL, config, and override before replacing them.

On November 8, 2026, Double Deluxe SP4 1.8.0.80 reached Pleasantview and the
Goth household with patched WineD3D/OpenGL on an RTX 4070 SUPER. Cassandra
and Mortimer rendered and animated, and the A8 override removed the observed
black rectangles. This was a household smoke test, not a full playthrough.
The earlier DXVK 3.1.1 run reached neighborhood selection. Local artifacts are
under `tmp/sims2-swvp-20261108` and `tmp/sims2-dx9-20261108`.
