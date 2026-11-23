# Clipping callbacks and message queues

The v45 captured-input patch used `SendMessageW` to ask its input thread to
change the X11 pointer grab. This introduced two unwanted effects inside the
driver callback: application window procedures could run, and a pending
message-queue wait could lose its wake masks.

In Wine 11, `process_driver_events()` sets the outer wait's masks before
processing internal hardware messages. Internal `WM_WINE_CLIPCURSOR` handling
can call the driver at that point. `SendMessageW` changes the masks to wait for
its reply, leaving the outer wait unable to wake for posted application
messages. `SendMessageTimeoutW(SMTO_BLOCK)` prevents window-procedure reentry
but still changes those masks. Real Deal's installer consequently waited on
the Choose Destination Location page with an unprocessed posted COM request.

The updated patch posts a request to the helper and waits on a completion
event. The request and arguments stay alive until the helper finishes; a call
already on the helper executes directly. The caller does not enter a nested
message wait or dispatch application messages.

## Run

```sh
i686-w64-mingw32-gcc -O2 -Wall -Wextra -Werror \
  clipcursor-reentrancy.c -luser32 -o clipcursor-reentrancy.exe
```

Run the executable with Wine in an isolated prefix, or inside Boxedwine using
a disposable root. The probe creates a visible foreground window and clips
the pointer briefly, so use a private X server or `-novideo` in Boxedwine.

It queues 16 cross-thread sent messages while polling queue status and
changing clipping. It then repeats a second scenario four times: a worker
changes clipping while the foreground thread waits for a posted message,
then posts that message. Each wake has a two-second deadline.

Expected output:

```text
CLIP_REENTRANCY sent=16 clip=0 poll=0
CLIP_QUEUE_WAKE waits=4 missed=0 failures=0
```

On 2026-11-23, with the same Windows x64 Boxedwine engine:

| Wine driver | Sent-message reentry | Missed posted-message wakes |
| --- | --- | --- |
| v44, before the input helper | 0 | 0/4 |
| v45, synchronous message handoff | Observed during queue polling | 4/4 |
| Event-based handoff | 0 | 0/4 |

The original Real Deal installer recording also passes with the updated
driver, whereas v45 times out. This probe complements the XTest-based
[`dinput-capture-thread`](dinput-capture-thread.md) checks; both must pass to
cover installer message waits and captured input with a blocked window thread.
