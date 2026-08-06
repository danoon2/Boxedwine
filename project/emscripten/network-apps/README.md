# Boxedwine Emscripten Network Apps

Build Linux ELF app:

```sh
./build-linux-apps.sh
```

Build Win32 PE versions of the network share apps:

```sh
./build-win32-apps.sh
```

The Linux script uses `clang` and the Emscripten SDK `lld` by default. Override
them with `CLANG=/path/to/clang` or `LLD=/path/to/lld` if needed. The Win32
script uses `clang -target i386-pc-windows-msvc` and `lld -flavor link`; set
`PE_CLANG`, `PE_LLD`, or `PE_STRIP` to override those tools.

The Win32 share binaries are PE console executables that Wine can launch like
normal `.exe` files, but they intentionally keep using Boxedwine's Linux syscall
path internally. That keeps the networking proof close to the already-tested
Linux apps and avoids a separate WinSock/Win32 filesystem port. Use the
`*-win32.zip` app zips and omit `linux=true` in the Boxedwine URL.

## Required Gateway

Most apps need the WebSocket gateway:

```sh
node network-gateway.mjs --port 8001
```

The default allowlist permits `127.0.0.1:*` and `localhost:*`.

## Gateway Dashboard Authentication

`network-gateway.mjs` can protect its dashboard and HTTP API with browser-native
Basic Auth. The WebSocket path remains unauthenticated so existing Boxedwine
clients can continue to connect with the normal `networkGateway=` URL.

Generate a password hash:

```sh
node network-apps/gateway-password-hash.mjs admin
```

Then export the printed value before starting the gateway:

```sh
export BOXEDWINE_GATEWAY_AUTH='admin:scrypt:v1:...'
node network-gateway.mjs --port 8001
```

For local scripting, the helper also accepts `--auth`, but interactive entry
is preferred so the password is not stored in shell history.

## Guest Apps

### `network-share-host`

Freestanding i386 Linux ELF binary. This is the first LAN share-agent proof.
Run it in the first browser peer with an overlay zip that creates test content.
By default, the host scans `/home/username/.wine/dosdevices/c:/host`, listens
on virtual TCP port `19200`, and keeps advertising a read-only `c-host` share
over UDP broadcast to `10.0.3.255:19201`. The host keeps running after each
sync so join clients can reconnect for later refreshes.

When a join client connects, the host recursively walks the share and sends a
simple archive stream:

```text
BW-SHARE-ARCHIVE/1
name=c-host
root=/home/username/.wine/dosdevices/c:/host
drive=c
path=host
mode=read-only
entries=recursive

dir path=SOUND
file path=DEMO.EXE size=...
<raw file bytes>
endfile
END
```

The join client also asks for a small manifest before each archive fetch:

```text
BW-SHARE-MANIFEST/1
name=c-host
hash=...
entries=recursive

file path=DEMO.EXE size=... hash=...
dir path=SOUND
END
```

After the first sync, the join client compares this hash with the previous poll
and skips downloads when the share tree has not changed. When the hash changes,
it compares the manifest entries with the local mirror, prunes deleted paths,
and fetches one-file archives only for new or modified files.

This proves the multiplayer-style shape using overlay-provided files: one
Boxedwine instance announces a share on the virtual LAN and another instance
discovers it, mirrors files and subdirectories, and the join side creates a
client-side Wine `Y:` drive mapping.

Use `storage=memory` for the host peer when serving overlay-provided files.
Use persistent storage for the join peer when you want the mirrored `Y:` drive
to remain available for a later Explorer session.

Host options can be supplied through the `args=` URL parameter. Both
`--key value` and `--key=value` are accepted:

```text
--root /home/username/.wine/dosdevices/c:/host
--name c-host
--drive c
--path host
--mode read-only
--port 19200
--beacon-port 19201
--broadcast 10.0.3.255
```

Successful output includes `network-share-host: sharing ...`,
`network-share-host: listening on 0.0.0.0:19200`,
`network-share-host: advertising share c-host`,
`network-share-host: client connected`, one or more
`network-share-host: sending file ...` lines, and
`network-share-host: archive sent`. Each subsequent join refresh should print
either `network-share-host: manifest sent` only when no files changed, or
`network-share-host: file archive sent` for each changed file.

Run URL:

```text
http://127.0.0.1:8000/boxedwine.html?app=network-apps/network-share-host&p=network-share-host&linux=true&storage=memory&overlay=home.zip&network=websocket&networkDebug=true&networkGateway=ws://127.0.0.1:8001/boxedwine-network
```

Win32 run URL:

```text
http://127.0.0.1:8000/boxedwine.html?app=network-apps/network-share-host-win32&p=network-share-host.exe&storage=memory&overlay=home.zip&network=websocket&networkDebug=true&networkGateway=ws://127.0.0.1:8001/boxedwine-network
```

Configured run URL:

```text
http://127.0.0.1:8000/boxedwine.html?app=network-apps/network-share-host&p=network-share-host&linux=true&storage=memory&overlay=home.zip&network=websocket&networkDebug=true&networkGateway=ws://127.0.0.1:8001/boxedwine-network&args=--root%20/home/username/.wine/dosdevices/c:/host%20--name%20c-host%20--drive%20c%20--path%20host%20--port%2019200%20--beacon-port%2019201
```

### `network-share-join`

Freestanding i386 Linux ELF binary. Run this in the second browser peer while
`network-share-host` is advertising. By default, it binds UDP port `19201`,
receives the share beacon, copies the source virtual IP into its TCP target
address, reads the host TCP port from the beacon, requests the archive, and
mirrors it into:

```text
C:\share-mirror
```

After the mirror completes, it refreshes Wine's `Y:` drive mapping so the same
files are visible as:

```text
Y:\
```

Successful output includes `network-share-join: beacon received`,
`network-share-join: connect ok`, `network-share-join: manifest hash ...`,
`network-share-join: archive request sent`, `network-share-join: archive
received`, `network-share-join: mirrored dir ...`, `network-share-join:
mirrored file ...`, and `network-share-join: mapped ...`. Polls where the
manifest hash has not changed print `network-share-join: no share changes` and
skip downloads. Polls where the manifest changed print
`network-share-join: fetching changed file ...` for each new or modified file,
then `network-share-join: delta changed files ... skipped ...`.

Use persistent storage for this browser peer when you want to launch Boxedwine
afterward, run Explorer, and browse `Y:`. In other words, omit
`storage=memory` for the join step. The host can still use `storage=memory`
because it only needs to serve the overlay-provided files during that session.
The current proof app buffers archives up to 8 MiB; larger shares report
`network-share-join: archive incomplete or too large` and do not remap `Y:`.

Join options can be supplied through `args=`:

```text
--mirror-root /home/username/.wine/dosdevices/c:/share-mirror
--drive y
--beacon-port 19201
--host 10.0.3.2
--port 19200
--poll-seconds 30
--once
```

`--host` skips UDP discovery and connects directly to the provided virtual IP.
When `--port` is omitted, the joiner uses the TCP port advertised in the share
beacon. By default, the joiner keeps running and refreshes the mirror every
30 seconds. Set `--poll-seconds` to change the interval, or pass `--once` to
perform a single sync and exit.
The first sync still uses the full archive path. Later changed polls use the
manifest entries for a simple delta refresh: deleted paths are pruned locally,
unchanged files are skipped by size/hash, and only new or modified files are
fetched. A failed delta falls back to the full archive path.

Run URL:

```text
http://127.0.0.1:8000/boxedwine.html?app=network-apps/network-share-join&p=network-share-join&linux=true&network=websocket&networkDebug=true&networkGateway=ws://127.0.0.1:8001/boxedwine-network
```

Win32 run URL:

```text
http://127.0.0.1:8000/boxedwine.html?app=network-apps/network-share-join-win32&p=network-share-join.exe&network=websocket&networkDebug=true&networkGateway=ws://127.0.0.1:8001/boxedwine-network
```

Configured run URL:

```text
http://127.0.0.1:8000/boxedwine.html?app=network-apps/network-share-join&p=network-share-join&linux=true&network=websocket&networkDebug=true&networkGateway=ws://127.0.0.1:8001/boxedwine-network&args=--mirror-root%20/home/username/.wine/dosdevices/c:/share-mirror%20--drive%20y%20--beacon-port%2019201
```

Configured live-polling run URL:

```text
http://127.0.0.1:8000/boxedwine.html?app=network-apps/network-share-join&p=network-share-join&linux=true&network=websocket&networkDebug=true&networkGateway=ws://127.0.0.1:8001/boxedwine-network&args=--mirror-root%20/home/username/.wine/dosdevices/c:/share-mirror%20--drive%20y%20--beacon-port%2019201%20--poll-seconds%2030
```

End-to-end manual test:

1. Start the web server that hosts `boxedwine.html`.
2. Start `network-gateway.mjs`.
3. Launch `network-share-host` with `overlay=home.zip`.
4. Launch `network-share-join` in a second browser without `storage=memory`.
   Keep this browser running if you want the mirror to refresh every poll
   interval.
5. Launch Boxedwine in the second browser with the default persistent storage,
   run `explorer.exe`, and open `Y:`.

### `network-share-agent`

Freestanding i386 Linux ELF binary. This is the symmetric read-only share
agent. Each running peer advertises and serves one local share while also
discovering remote shares, mirroring them, and assigning Wine drive letters.

By default, it serves `/home/username/.wine/dosdevices/c:/host` as `c-host`,
uses `/home/username/.wine/dosdevices/c:/share-mirror` as the mirror base,
polls every 30 seconds, and assigns remote shares from `Y:` backward:

```text
Y: first remote share
X: second remote share
W: third remote share
```

The allocator skips `Z:`, `C:`, `D:`, any letter already assigned to another
remote share, and any drive letter that already exists in Wine's `dosdevices`
folder. Remote mirrors use sanitized share names:

```text
C:\share-mirror\alice-games
C:\share-mirror\bob_tools
```

Share names are sanitized to `A-Z`, `a-z`, `0-9`, `_`, and `-`, with unsafe
characters converted to `_`. Empty, `.`/`..`, and reserved Windows device names
fall back to `share`; collisions append numeric suffixes. Use unique share
names for each peer, because the agent ignores beacons whose name matches its
own local share.

Agent options can be supplied through `args=`:

```text
--root /home/username/.wine/dosdevices/c:/host
--name alice-games
--drive c
--path host
--mode read-only
--mirror-base /home/username/.wine/dosdevices/c:/share-mirror
--port 19200
--beacon-port 19201
--broadcast 10.0.3.255
--poll-seconds 30
```

Two-peer manual test:

1. Start the web server that hosts `boxedwine.html`.
2. Start `network-gateway.mjs`.
3. Launch peer A with `network-share-agent`, a unique `--name`, and an overlay
   that populates `c:/host`.
4. Launch peer B with `network-share-agent`, a different unique `--name`, and
   its own overlay.
5. Wait for each peer to print `network-share-agent: discovered ...`,
   `network-share-agent: synced ...`, and `network-share-agent: mapped ...`.
6. Stop the agent for any peer whose mirrors you want to inspect, then launch
   Boxedwine with that peer's persistent storage and open Explorer. Remote
   shares should appear at `Y:`, then `X:`, then `W:` as more peers are
   discovered.

Use `storage=memory` for disposable peers. Omit `storage=memory` for the peer
whose mirrored drives you want to keep for a later Explorer session. Running
multiple active Boxedwine instances against the same persistent IndexedDB store
can still conflict, so use persistent storage selectively during manual tests.
The agent sync path is manifest-first. It reads a length-prefixed manifest,
creates directories, skips unchanged files by size/hash, and fetches only
changed files with `GET file-data path=...`. File-data responses include an
explicit byte count, so the receiver reads exactly the advertised number of
bytes into the mirror folder. The first sync does not prune existing files;
later refreshes prune deleted paths. If the manifest cannot be read, the agent
still has the older full archive refresh as a fallback.

If two agents discover each other at the same time, the peer with the
lexicographically later share name delays its first sync briefly so the other
peer can connect while it continues serving incoming requests.
Repeated beacons refresh the known peer address but do not force immediate
resyncs; normal refreshes happen on the `--poll-seconds` interval.

Peer A URL:

```text
http://127.0.0.1:8000/boxedwine.html?app=network-apps/network-share-agent&p=network-share-agent&linux=true&storage=memory&overlay=home.zip&network=websocket&networkDebug=true&networkGateway=ws://127.0.0.1:8001/boxedwine-network&args=--name%20alice-games%20--root%20/home/username/.wine/dosdevices/c:/host
```

Peer A Win32 URL:

```text
http://127.0.0.1:8000/boxedwine.html?app=network-apps/network-share-agent-win32&p=network-share-agent.exe&storage=memory&overlay=home.zip&network=websocket&networkDebug=true&networkGateway=ws://127.0.0.1:8001/boxedwine-network&args=--name%20alice-games%20--root%20/home/username/.wine/dosdevices/c:/host
```

Peer B URL:

```text
http://127.0.0.1:8000/boxedwine.html?app=network-apps/network-share-agent&p=network-share-agent&linux=true&storage=memory&overlay=home.zip&network=websocket&networkDebug=true&networkGateway=ws://127.0.0.1:8001/boxedwine-network&args=--name%20bob-tools%20--root%20/home/username/.wine/dosdevices/c:/host
```

Peer B Win32 URL:

```text
http://127.0.0.1:8000/boxedwine.html?app=network-apps/network-share-agent-win32&p=network-share-agent.exe&storage=memory&overlay=home.zip&network=websocket&networkDebug=true&networkGateway=ws://127.0.0.1:8001/boxedwine-network&args=--name%20bob-tools%20--root%20/home/username/.wine/dosdevices/c:/host
```
