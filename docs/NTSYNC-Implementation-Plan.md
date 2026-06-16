# NTSYNC Implementation Plan

This document tracks the plan for making Boxedwine compatible with Wine's NTSYNC path. NTSYNC is exposed to Linux userspace as a character device, `/dev/ntsync`, controlled through `ioctl()` calls. Boxedwine already has virtual device nodes and ioctl dispatch, so the work should fit into the existing syscall and filesystem model.

References:

- Linux kernel NTSYNC userspace API: https://docs.kernel.org/userspace-api/ntsync.html
- Linux NTSYNC uAPI header: https://raw.githubusercontent.com/torvalds/linux/master/include/uapi/linux/ntsync.h

## Goals

- [ ] Let Wine detect `/dev/ntsync` inside Boxedwine.
- [ ] Implement the NTSYNC ioctl ABI closely enough for Wine 11 and newer to use it.
- [ ] Use host Linux `/dev/ntsync` when available for real kernel-level synchronization benefits.
- [ ] Provide a portable emulated backend for non-Linux hosts and for systems without host NTSYNC.
- [ ] Add focused tests for NTSYNC object semantics and Wine-facing behavior.

## Non-Goals

- Do not add a new Linux syscall for NTSYNC. Wine uses `/dev/ntsync` plus `ioctl()`.
- Do not expose NTSYNC as a general Boxedwine synchronization API outside of the Linux compatibility layer.
- Do not require host NTSYNC for basic compatibility; native acceleration should be optional.

## Current Boxedwine Touchpoints

- Virtual `/dev` nodes are registered in `source/sdl/startupArgs.cpp`.
- Device implementations live under `source/kernel/devs/`.
- Device headers live under `include/`.
- `ioctl()` dispatch flows through `source/kernel/syscall.cpp`, `KProcess::ioctl()`, `KFile::ioctl()`, and then `FsOpenNode::ioctl()`.
- Returned NTSYNC objects should be represented as guest file descriptors using `KProcess::allocFileDescriptor()`.
- Existing condition and wait helpers live in `source/util/synchronization.*`.
- Existing futex behavior lives in `source/kernel/kthread.cpp` and can inform fallback wait behavior.

## Proposed Architecture

### Device and Context

- [ ] Add `include/devntsync.h`.
- [ ] Add `source/kernel/devs/devntsync.cpp`.
- [ ] Register `/dev/ntsync` in `StartUpArgs::buildVirtualFileSystem()`.
- [ ] Opening `/dev/ntsync` creates one `NTSyncContext`.
- [ ] Objects created from one context must not be waitable with objects from another context.

### Object Model

- [ ] Add an NTSYNC object representation for:
  - [ ] semaphore
  - [ ] mutex
  - [ ] event
- [ ] Represent created objects as normal guest file descriptors.
- [ ] Ensure object lifetime follows FD lifetime.
- [ ] Implement useful `selfFd()` labels for `/proc/self/fd`.

### Backend Model

- [ ] Add a backend boundary so native and emulated implementations share the ioctl-facing ABI.
- [ ] Native Linux backend:
  - [ ] Open host `/dev/ntsync`.
  - [ ] Create host NTSYNC objects and store host FDs in guest object wrappers.
  - [ ] Translate guest FD arrays for wait ioctls into host FD arrays.
  - [ ] Copy output fields back to guest memory.
- [ ] Emulated backend:
  - [ ] Use a context-level lock and condition variable.
  - [ ] Implement atomic wait-any and wait-all semantics.
  - [ ] Wake waiters after state-changing operations.

## ioctl Surface

Implement the ioctl constants and structures from `linux/ntsync.h`.

- [x] `NTSYNC_IOC_CREATE_SEM`
- [x] `NTSYNC_IOC_CREATE_MUTEX`
- [x] `NTSYNC_IOC_CREATE_EVENT`
- [x] `NTSYNC_IOC_SEM_RELEASE`
- [x] `NTSYNC_IOC_SEM_READ`
- [x] `NTSYNC_IOC_MUTEX_UNLOCK`
- [x] `NTSYNC_IOC_MUTEX_KILL`
- [x] `NTSYNC_IOC_MUTEX_READ`
- [x] `NTSYNC_IOC_EVENT_SET`
- [x] `NTSYNC_IOC_EVENT_RESET`
- [x] `NTSYNC_IOC_EVENT_PULSE`
- [x] `NTSYNC_IOC_EVENT_READ`
- [x] `NTSYNC_IOC_WAIT_ANY`
- [x] `NTSYNC_IOC_WAIT_ALL`

All ioctls are implemented in the emulated backend (`source/kernel/devs/devntsync.cpp`).

## Semantic Requirements

- [x] Semaphore count must never exceed max; overflow returns `EOVERFLOW`.
- [x] Acquiring a semaphore decrements its count.
- [x] Mutex owner zero means unowned.
- [x] Mutex recursion count and owner must be consistent at create time.
- [x] Unlock by owner zero returns `EINVAL`.
- [x] Unlock by non-owner returns `EPERM`.
- [x] Killed mutexes become abandoned and unowned.
- [x] Acquiring an abandoned mutex returns `EOWNERDEAD` while still acquiring it.
- [x] Auto-reset events become unsignaled when acquired.
- [x] Manual-reset events remain signaled when acquired.
- [x] Pulse wakes eligible waiters and leaves the event unsignaled. Each context
      keeps a registry of blocked waiters (`NTSyncContext::waiters`, objects held
      weakly to avoid reference cycles). A pulse momentarily signals the event and
      walks the registry in arrival order, acquiring objects on behalf of each
      eligible waiter and recording the wake result. Acquiring an auto-reset event
      designals it, so only the first waiter wakes; a manual-reset event stays
      signaled across the walk, so all eligible waiters wake. The event is then
      reset and no state lingers, so waiters that arrive after the pulse are not
      affected. A pulse with no blocked waiters wakes no one, as expected.
- [x] `WAIT_ANY` acquires at most one object and writes the selected index.
- [x] `WAIT_ALL` acquires all objects atomically or modifies none.
- [x] `WAIT_ALL` rejects duplicate objects (in `objs`, or shared with `alert`).
- [x] Timeouts use absolute nanoseconds and honor `NTSYNC_WAIT_REALTIME`
      (`U64_MAX` waits forever, a past deadline returns `ETIMEDOUT`).
- [x] Alert event terminates a wait with `index == count`.
- [x] Signals and thread termination return appropriate interrupt behavior
      (`EINTR`/`CONTINUE`), mirroring the futex path in `kthread.cpp`.

## Implementation Phases

### Phase 1: Device Discovery — done

- [x] Add `/dev/ntsync` registration.
- [x] Add placeholder device implementation.
- [x] Add ioctl constant definitions.
- [x] Make unsupported ioctls return Linux-compatible errors.
- [ ] Verify Wine opens `/dev/ntsync`. (Pending Phase 6 Wine validation.)

Acceptance:

- [ ] A small guest program can `stat()` and `open()` `/dev/ntsync`.
- [ ] Wine 11 attempts NTSYNC ioctls instead of immediately falling back.

### Phase 2: Object Creation — done

- [x] Implement create semaphore.
- [x] Implement create mutex.
- [x] Implement create event.
- [x] Return valid guest FDs for created objects.
- [x] Reject invalid create arguments.

Acceptance:

- [ ] Guest test can create each object type and close returned FDs.
- [ ] Invalid create inputs return expected errors.

### Phase 3: Read and State Mutation — done

- [x] Implement semaphore release/read.
- [x] Implement mutex unlock/kill/read.
- [x] Implement event set/reset/pulse/read.
- [x] Wake waiters when object state changes (single per-context condition).

Acceptance:

- [ ] Unit tests cover state transitions for every object type.
- [ ] Error returns match Linux NTSYNC behavior.

### Phase 4: Wait Semantics — done (emulated backend)

- [x] Implement `WAIT_ANY` (lowest signaled index wins).
- [x] Implement `WAIT_ALL` (atomic acquire-all, duplicate rejection).
- [x] Implement alert event handling (`index == count`).
- [x] Implement timeout handling (absolute ns, monotonic/realtime, infinite).
- [x] Implement abandoned mutex results during waits (`EOWNERDEAD`).

The wait loop mirrors the futex implementation in `source/kernel/kthread.cpp`:
it blocks on the per-context condition and works in both the multi-threaded and
the cooperative/single-threaded (wasm) builds. Because the NTSYNC timeout is
absolute, recomputing the deadline after a cooperative re-entry stays correct.

Acceptance:

- [ ] Tests verify single-object waits.
- [ ] Tests verify multi-object wait-any index selection.
- [ ] Tests verify wait-all atomicity.
- [ ] Tests verify alert event behavior.
- [ ] Tests verify timeout behavior.

### Phase 5: Native Linux Acceleration

- [ ] Detect host `/dev/ntsync` availability.
- [ ] Forward create and mutation ioctls to host backend.
- [ ] Translate guest wait FD arrays to host wait FD arrays.
- [ ] Fall back to emulated backend if host support is unavailable or disabled.

Acceptance:

- [ ] On Linux with NTSYNC, Boxedwine uses host NTSYNC.
- [ ] On hosts without NTSYNC, Wine still sees a working emulated device.
- [ ] Native and emulated backends pass the same behavioral tests.

### Phase 6: Wine Validation and Benchmarking

- [ ] Build or install a Wine version with NTSYNC support.
- [ ] Add logging around `/dev/ntsync` open and ioctl paths.
- [ ] Confirm Wine uses NTSYNC under Boxedwine.
- [ ] Run at least one sync-heavy app or game workload.
- [ ] Compare behavior and performance against the pre-NTSYNC path.

Acceptance:

- [ ] Wine does not fall back because of missing or broken NTSYNC.
- [ ] No regressions in existing Wine startup or common app workflows.
- [ ] Performance results are recorded in `docs/Performance.md` or a linked benchmark note.

## Build and Project Updates

- [ ] Linux makefile should pick up new `source/**/*.cpp` files automatically.
- [ ] Update Visual Studio project files for new `.cpp` and `.h` files.
- [ ] Check macOS/Xcode project handling if NTSYNC files are not auto-discovered there.
- [ ] Ensure native Linux backend code is guarded so non-Linux builds compile cleanly.

## Open Questions

- [ ] Should native NTSYNC be enabled automatically, or controlled by a startup flag?
- [ ] What major/minor device number should Boxedwine expose for virtual `/dev/ntsync`?
- [ ] Should emulated NTSYNC be advertised by default on every host, or only when Wine probes for it?
- [ ] How should diagnostic logging expose native versus emulated backend selection?
- [ ] Which Wine build should be used as the compatibility baseline?

## Progress Log

- 2026-05-21: Initial plan recorded after reviewing Boxedwine syscall/device structure and Linux NTSYNC uAPI.
- 2026-06-15: Completed the emulated backend (Phases 1–4).
  - Fixed a build break in the WIP device: the ioctl handlers used `IOCTL_ARG1`
    (`EDX`) without a `CPU* cpu` in scope.
  - Unified all object state access, signaling and waiting under the single
    per-context `BoxedWineCondition` to remove the lost-wakeup race between the
    state-changing ioctls and waiters (previously a separate `context->mutex`
    was used for mutation while waits would have needed the condition mutex).
  - Implemented `NTSYNC_IOC_WAIT_ANY` and `NTSYNC_IOC_WAIT_ALL` including alert
    events, absolute timeouts (monotonic/realtime/infinite), abandoned-mutex
    `EOWNERDEAD`, duplicate rejection for `WAIT_ALL`, and signal/termination
    interrupt handling. Works in both threading models.
  - Added per-type `selfFd()` labels for `/proc/self/fd`.
- 2026-06-15 (later): Reworked `EVENT_PULSE` from the initial bounded grant model
  to an exact, kernel-faithful implementation. Each context now keeps a registry
  of blocked waiters; a pulse walks it, momentarily signaling the event, and
  acquires objects on behalf of each eligible waiter (auto-reset wakes one,
  manual-reset wakes all). This removes the stale-grant edge cases where a later
  waiter could be wrongly satisfied. The registry holds objects weakly so a
  lingering record can never keep a context/object alive.
  - Verified the full Linux release build compiles and links; the device file
    also syntax-checks under the single-threaded (non-`BOXEDWINE_MULTI_THREADED`)
    configuration.
  - Not yet done: Phase 5 (native host `/dev/ntsync` acceleration — the dev host
    runs kernel 6.8 without `/dev/ntsync`, so it cannot be exercised here) and
    Phase 6 (Wine 11 validation/benchmarking). Automated tests are still pending;
    Boxedwine's test harness is CPU/MMU-focused and has no guest-syscall
    fixture, so a guest-side NTSYNC test program is the likely vehicle.
