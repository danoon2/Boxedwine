# Local server

`-localServer <port>` starts an HTTP and WebSocket server on 127.0.0.1. Tools and agents use it to
see and hear a running program without a host window, focus, or a platform capture API. It works
with `-novideo` and `-nosound`.

```
boxedwine -novideo -localServer 8086 -zip TinyCore15Wine11.0.zip /bin/wine notepad
```

Port 0 picks a free port. The log has a `Local server listening on http://127.0.0.1:<port>/` line
either way. The server only listens on the loopback address, and answers 403 to a request whose
`Host` or `Origin` header names another host, so a web page open in a browser can't read the
screen through it. `GET /` lists the endpoints.

Each request gets its own connection, and the server closes it after the response.

## Screen

The server keeps a copy of the guest's screen, composited the same way the Boxedwine window shows
it: the Wine desktop, every X window Wine draws (GDI, DirectDraw through GDI), and OpenGL output
(including Direct3D through wined3d) laid over the window it belongs to. The mouse cursor is not
part of it.

OpenGL frames come from reading the back buffer before each swap. That stalls the game's GL
pipeline, so it only happens while a consumer asked for the screen in the last two seconds, and at
most about 60 times a second. A request for the screen while a GL window is presenting waits up to
250 ms for a frame read after the request, so the answer is never older than the request.

### `GET /screenshot`

The current picture as a PNG (RGB, 8 bits a channel). `x`, `y`, `w` and `h` crop it; a crop
outside the screen is a 400. Before anything has been presented the answer is a 503.

The `X-Frame-Hash` and `X-Frame-Change-Seq` headers identify the picture, with the same values
`/screen` reports.

```
curl -o screen.png http://127.0.0.1:8086/screenshot
curl -o button.png "http://127.0.0.1:8086/screenshot?x=300&y=200&w=200&h=50"
```

### `GET /screen`

The picture's description without its pixels:

```json
{"width":800,"height":600,"hash":"8f0c2a6d1e4b7c39","change_seq":42,"changed_ms_ago":1250,"presents":3120,"gl_presenting":false}
```

- `hash` is a 64-bit FNV-1a hash of the RGB pixels and the size, in hex.
- `change_seq` counts changes to the picture. A present that leaves every pixel the same does not
  count.
- `changed_ms_ago` is the time since the last change.
- `presents` counts frames presented (desktop redraws plus GL swaps), changed or not.
- `gl_presenting` is true while an OpenGL window swapped in the last second.

`since=<hash>&wait=<ms>` turns it into a long poll: the answer comes when the picture's hash
differs from `since`, or after `wait` ms (at most 60000), with `"changed"` saying which. A script
waits for the screen to move on without transferring pictures:

```
curl "http://127.0.0.1:8086/screen?since=8f0c2a6d1e4b7c39&wait=10000"
```

### `GET /ws/screen` (WebSocket)

Binary messages, each one PNG. `fps` sets the most frames a second (default 5, 0.1 to 60).
`mode=change` (the default) sends a frame only when the picture changed since the previous one;
`mode=all` sends one every tick, for recordings that need a constant frame rate.

## Audio

Every `/dev/dsp` voice the guest writes (Wine's OSS driver opens one per stream) is converted to
44100 Hz stereo and mixed at wall-clock rate, as the host's mixer would play them. MIDI is not
included.

### `GET /audio`

```json
{"voices":1,"peak":12011,"frames_mixed":441000,"sound_frames":220500,"ms_since_sound":35,"sample_rate":44100,"channels":2,"format":"s16le"}
```

- `voices` is the number of open DSP voices.
- `peak` is the loudest sample (0 to 32767) since the previous `/audio` request. 0 means silence.
- `sound_frames` counts mixed frames with any non-zero sample, out of `frames_mixed`.
- `ms_since_sound` is the time since the last non-zero sample, or `null` if there was none.

To check that a game plays sound at all, request `/audio` once, let the game run, and request it
again: a non-zero `peak` means it played something in between.

### `GET /ws/audio` (WebSocket)

A text message with the format, `{"type":"audio_format","sample_rate":44100,"channels":2,"format":"s16le"}`,
then binary messages of interleaved s16le stereo PCM, about every 20 ms. A client that falls more
than a second behind loses the oldest chunks.
