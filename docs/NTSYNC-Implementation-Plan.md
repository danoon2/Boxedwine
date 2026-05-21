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

- [ ] `NTSYNC_IOC_CREATE_SEM`
- [ ] `NTSYNC_IOC_CREATE_MUTEX`
- [ ] `NTSYNC_IOC_CREATE_EVENT`
- [ ] `NTSYNC_IOC_SEM_RELEASE`
- [ ] `NTSYNC_IOC_SEM_READ`
- [ ] `NTSYNC_IOC_MUTEX_UNLOCK`
- [ ] `NTSYNC_IOC_MUTEX_KILL`
- [ ] `NTSYNC_IOC_MUTEX_READ`
- [ ] `NTSYNC_IOC_EVENT_SET`
- [ ] `NTSYNC_IOC_EVENT_RESET`
- [ ] `NTSYNC_IOC_EVENT_PULSE`
- [ ] `NTSYNC_IOC_EVENT_READ`
- [ ] `NTSYNC_IOC_WAIT_ANY`
- [ ] `NTSYNC_IOC_WAIT_ALL`

## Semantic Requirements

- [ ] Semaphore count must never exceed max; overflow returns `EOVERFLOW`.
- [ ] Acquiring a semaphore decrements its count.
- [ ] Mutex owner zero means unowned.
- [ ] Mutex recursion count and owner must be consistent at create time.
- [ ] Unlock by owner zero returns `EINVAL`.
- [ ] Unlock by non-owner returns `EPERM`.
- [ ] Killed mutexes become abandoned and unowned.
- [ ] Acquiring an abandoned mutex returns `EOWNERDEAD` while still acquiring it.
- [ ] Auto-reset events become unsignaled when acquired.
- [ ] Manual-reset events remain signaled when acquired.
- [ ] Pulse wakes eligible waiters and leaves the event unsignaled.
- [ ] `WAIT_ANY` acquires at most one object and writes the selected index.
- [ ] `WAIT_ALL` acquires all objects atomically or modifies none.
- [ ] `WAIT_ALL` rejects duplicate objects as the kernel API specifies.
- [ ] Timeouts use absolute nanoseconds and honor `NTSYNC_WAIT_REALTIME`.
- [ ] Signals and thread termination return appropriate interrupt behavior.

## Implementation Phases

### Phase 1: Device Discovery

- [ ] Add `/dev/ntsync` registration.
- [ ] Add placeholder device implementation.
- [ ] Add ioctl constant definitions.
- [ ] Make unsupported ioctls return Linux-compatible errors.
- [ ] Verify Wine opens `/dev/ntsync`.

Acceptance:

- [ ] A small guest program can `stat()` and `open()` `/dev/ntsync`.
- [ ] Wine 11 attempts NTSYNC ioctls instead of immediately falling back.

### Phase 2: Object Creation

- [ ] Implement create semaphore.
- [ ] Implement create mutex.
- [ ] Implement create event.
- [ ] Return valid guest FDs for created objects.
- [ ] Reject invalid create arguments.

Acceptance:

- [ ] Guest test can create each object type and close returned FDs.
- [ ] Invalid create inputs return expected errors.

### Phase 3: Read and State Mutation

- [ ] Implement semaphore release/read.
- [ ] Implement mutex unlock/kill/read.
- [ ] Implement event set/reset/pulse/read.
- [ ] Wake waiters when object state changes.

Acceptance:

- [ ] Unit tests cover state transitions for every object type.
- [ ] Error returns match Linux NTSYNC behavior.

### Phase 4: Wait Semantics

- [ ] Implement `WAIT_ANY`.
- [ ] Implement `WAIT_ALL`.
- [ ] Implement alert event handling.
- [ ] Implement timeout handling.
- [ ] Implement abandoned mutex results during waits.

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
