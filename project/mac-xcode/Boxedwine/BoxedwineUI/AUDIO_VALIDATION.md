# F-16 audio looping investigation

September 12, 2026. The regular native UI's installed F-16 demo used Wine 11.0 / filesystem 11, package SHA-256 `c5de48e0d8f9f2e022a0d5f70603c11800252ab96356a353c3363d851645eb37`, and packaged psVoodoo through WineD3D OpenGL. The user reported several seconds of normal sound followed by a repeating short sample, with intermittent recovery. This was also present outside the native UI branch.

The runtime opened SDL queued audio as 44,100 Hz, stereo, 32-bit float, with matching requested and obtained formats. A thread sample showed both the guest audio writer and host audio threads active. Debugger attachment was unavailable on this machine, so a temporary runtime diagnostic recorded writes that lost an already accepted byte count.

`FsOpenNode::write` splits a guest write at guest page boundaries. When a later `writeNative` returns an error, it replaced the accumulated successful byte count with that error. F-16 repeatedly accepted 8,192 bytes and then received `-EWOULDBLOCK` on the next chunk. At its audio format, that is approximately 23.22 ms of audio. Wine 11's OSS driver returns without advancing its buffer offset after a failed write, allowing those same samples to be queued repeatedly. The baseline diagnostic recorded thousands of occurrences.

The fix preserves the successful byte count when a later chunk fails. Errors before any progress still propagate, and partial positive writes still stop the operation normally. It changes the shared guest-write path, so the behavior applies on other hosts too. The temporary diagnostic was removed. SDL buffering, the existing native GETOSPACE compatibility policy, Wine, and game settings were not changed.

Validation:

- A deterministic test writes across guest page boundaries at three source alignments. It checks EWOULDBLOCK and EIO after a successful prefix, zero-length progress on a later chunk, retrying exactly the remaining bytes, an initially blocked write, and a fully successful write.
- Six error-after-progress cases failed with the original implementation. The complete test passes with the fix.
- The existing DSP math test, thirteen related mapped-file/writeback tests, and the file-write notification test pass (16 test entries total, including the new regression).
- The Debug native build and bundle audit pass. The fixed build was relaunched through the regular UI using the same installed F-16 demo. The user confirmed that sound is good after the fix.

Raw logs and the baseline sample are in `tmp/native-ui-test/f16-audio/`. The inspected upstream driver source is [Wine 11 OSS audio output](https://github.com/wine-mirror/wine/blob/wine-11.0/dlls/wineoss.drv/oss.c), particularly `oss_write_data`.
