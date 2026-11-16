# DirectPlay startup capabilities and lobby state

`wine11_dpnet_startup.patch` fixes two Wine 11.0 behaviors used during
MechCommander 2 startup, including its single-player mode:

- Peer `GetCaps` and `SetCaps` now validate arguments, expose Windows defaults,
  preserve short/extended structure boundaries, and retain changed values.
  Closing and reinitializing the peer resets its initialization state and caps.
- A directly launched lobbied application has no lobby connection. Initialize
  writes zero to the output handle; requests for absent connection settings
  return `DPNERR_INVALIDHANDLE` instead of success with uninitialized data.

This is shared Wine behavior, with no executable-name check or game bypass.
It does not implement DirectPlay multiplayer transport.

## Reproduce

Build one executable and run that same executable on Windows and Boxedwine:

```sh
i686-w64-mingw32-gcc -O2 -Wall dpnet-startup.c -o dpnet-startup.exe -lole32 -ldxguid
```

An optional first argument writes the result to a file. Compare all lines with
`dpnet-startup.windows.txt`; the executable's exit status alone is insufficient.
Use a fresh Boxedwine root and the final filesystem ZIP, without a DLL overlay.

The reference was recorded on this Windows host on November 14, 2026. Both
filesystem 36 variants reproduce all 40 lines exactly. The original filesystem
35 fails the capability calls and leaves the lobby handle unchanged. The patched
native game reaches gameplay, confirmed by the user.

Artifacts and final-archive results are in `tmp/mechcommander2-demo`, including
`release36-probes.json`. Browser gameplay is qualified separately from this
API-level regression check.
