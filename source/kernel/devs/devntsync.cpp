/*
 *  Copyright (C) 2012-2026  The BoxedWine Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 */

#include "boxedwine.h"
#include "devntsync.h"

#include "../../io/fsvirtualopennode.h"

// Emulated backend for Wine's NTSYNC path (/dev/ntsync). The ABI mirrors the
// Linux uAPI in include/uapi/linux/ntsync.h. Each open of /dev/ntsync creates
// one NTSyncContext; objects created from a context can only be waited on
// together with objects from the same context.

class KNTSyncObject;

// One record per thread currently blocked inside a WAIT_ANY/WAIT_ALL on this
// context. The registry lets NTSYNC_IOC_EVENT_PULSE faithfully wake exactly the
// waiters that are blocked at pulse time (see KNTSyncObject::ioctl). Objects are
// held weakly so a registered waiter never keeps its objects (and, through them,
// the context) alive: a stale record can never form a reference cycle.
struct NTSyncWaiter {
    U32 threadId = 0;
    U32 owner = 0;
    bool waitAll = false;
    bool hasAlert = false;
    std::vector<std::weak_ptr<KNTSyncObject>> objs;
    std::weak_ptr<KNTSyncObject> alert;

    // Set when a pulse acquired this waiter's objects on its behalf. The blocked
    // thread reports the recorded index/result on its next wake without acquiring
    // anything again.
    bool pulseSatisfied = false;
    U32 pulseIndex = 0;
    U32 pulseResult = 0;
};

struct NTSyncContext {
    NTSyncContext() : cond(std::make_shared<BoxedWineCondition>(B("NTSyncContext::cond"))) {}

    // All object state, signaling and waiting is serialized through this single
    // per-context condition. State changing ioctls signal it; waiters block on
    // it and re-evaluate object state when woken.
    BOXEDWINE_CONDITION cond;

    // Active waiters, in arrival order. Only touched while the condition lock is
    // held.
    std::vector<std::shared_ptr<NTSyncWaiter>> waiters;
};

struct NTSyncSemArgs {
    U32 count;
    U32 max;
};

struct NTSyncMutexArgs {
    U32 owner;
    U32 count;
};

struct NTSyncEventArgs {
    U32 manual;
    U32 signaled;
};

enum class NTSyncObjectType {
    Semaphore,
    Mutex,
    Event
};

static constexpr U32 NTSYNC_IOC_CREATE_SEM = 0x40084e80;
static constexpr U32 NTSYNC_IOC_SEM_RELEASE = 0xc0044e81;
static constexpr U32 NTSYNC_IOC_WAIT_ANY = 0xc0284e82;
static constexpr U32 NTSYNC_IOC_WAIT_ALL = 0xc0284e83;
static constexpr U32 NTSYNC_IOC_CREATE_MUTEX = 0x40084e84;
static constexpr U32 NTSYNC_IOC_MUTEX_UNLOCK = 0xc0084e85;
static constexpr U32 NTSYNC_IOC_MUTEX_KILL = 0x40044e86;
static constexpr U32 NTSYNC_IOC_CREATE_EVENT = 0x40084e87;
static constexpr U32 NTSYNC_IOC_EVENT_SET = 0x80044e88;
static constexpr U32 NTSYNC_IOC_EVENT_RESET = 0x80044e89;
static constexpr U32 NTSYNC_IOC_EVENT_PULSE = 0x80044e8a;
static constexpr U32 NTSYNC_IOC_SEM_READ = 0x80084e8b;
static constexpr U32 NTSYNC_IOC_MUTEX_READ = 0x80084e8c;
static constexpr U32 NTSYNC_IOC_EVENT_READ = 0x80084e8d;

static constexpr U32 NTSYNC_MAX_WAIT_COUNT = 64;
static constexpr U32 NTSYNC_WAIT_REALTIME = 0x1;
static constexpr U64 NTSYNC_TIMEOUT_INFINITE = 0xFFFFFFFFFFFFFFFFULL;

// struct ntsync_wait_args layout (40 bytes)
static constexpr U32 NTSYNC_WAIT_OFF_TIMEOUT = 0;  // __u64
static constexpr U32 NTSYNC_WAIT_OFF_OBJS = 8;     // __u64 (guest pointer)
static constexpr U32 NTSYNC_WAIT_OFF_COUNT = 16;   // __u32
static constexpr U32 NTSYNC_WAIT_OFF_INDEX = 20;   // __u32 (out)
static constexpr U32 NTSYNC_WAIT_OFF_FLAGS = 24;   // __u32
static constexpr U32 NTSYNC_WAIT_OFF_OWNER = 28;   // __u32
static constexpr U32 NTSYNC_WAIT_OFF_ALERT = 32;   // __u32
static constexpr U32 NTSYNC_WAIT_OFF_PAD = 36;     // __u32
static constexpr U32 NTSYNC_WAIT_ARGS_SIZE = 40;

static bool canRead(KThread* thread, U32 address, U32 len) {
    return address && thread->memory->canRead(address, len);
}

static bool canWrite(KThread* thread, U32 address, U32 len) {
    return address && thread->memory->canWrite(address, len);
}

static NTSyncSemArgs readSemArgs(KThread* thread, U32 address) {
    return { thread->memory->readd(address), thread->memory->readd(address + 4) };
}

static NTSyncMutexArgs readMutexArgs(KThread* thread, U32 address) {
    return { thread->memory->readd(address), thread->memory->readd(address + 4) };
}

static NTSyncEventArgs readEventArgs(KThread* thread, U32 address) {
    return { thread->memory->readd(address), thread->memory->readd(address + 4) };
}

static void writeSemArgs(KThread* thread, U32 address, const NTSyncSemArgs& args) {
    thread->memory->writed(address, args.count);
    thread->memory->writed(address + 4, args.max);
}

static void writeMutexArgs(KThread* thread, U32 address, const NTSyncMutexArgs& args) {
    thread->memory->writed(address, args.owner);
    thread->memory->writed(address + 4, args.count);
}

static void writeEventArgs(KThread* thread, U32 address, const NTSyncEventArgs& args) {
    thread->memory->writed(address, args.manual);
    thread->memory->writed(address + 4, args.signaled);
}

class KNTSyncObject : public KObject {
public:
    KNTSyncObject(const std::shared_ptr<NTSyncContext>& context, const NTSyncSemArgs& args)
        : KObject(KTYPE_NTSYNC), context(context), objectType(NTSyncObjectType::Semaphore), sem(args) {}

    KNTSyncObject(const std::shared_ptr<NTSyncContext>& context, const NTSyncMutexArgs& args)
        : KObject(KTYPE_NTSYNC), context(context), objectType(NTSyncObjectType::Mutex), mutex(args) {}

    KNTSyncObject(const std::shared_ptr<NTSyncContext>& context, const NTSyncEventArgs& args)
        : KObject(KTYPE_NTSYNC), context(context), objectType(NTSyncObjectType::Event), event(args) {
        event.signaled = event.signaled ? 1 : 0;
        event.manual = event.manual ? 1 : 0;
    }

    U32 ioctl(KThread* thread, U32 request) override;
    S64 seek(S64 pos) override { return 0; }
    S64 length() override { return 0; }
    S64 getPos() override { return 0; }
    void setBlocking(bool blocking) override {}
    bool isBlocking() override { return false; }
    void setAsync(bool isAsync) override {}
    bool isAsync() override { return false; }
    KFileLock* getLock(KFileLock* lock) override { return nullptr; }
    U32 setLock(KFileLock* lock, bool wait) override { return -K_ENOLCK; }
    bool supportsLocks() override { return false; }
    bool isOpen() override { return true; }
    bool isReadReady() override { return false; }
    bool isWriteReady() override { return false; }
    void waitForEvents(BOXEDWINE_CONDITION& parentCondition, U32 events) override {}
    U32 writeNative(U8* buffer, U32 len) override { return -K_EINVAL; }
    U32 readNative(U8* buffer, U32 len) override { return -K_EINVAL; }
    U32 stat(KProcess* process, U32 address, bool is64) override { return -K_ENODEV; }
    U32 map(KThread* thread, U32 address, U32 len, S32 prot, S32 flags, U64 off) override { return -K_ENODEV; }
    bool canMap() override { return false; }
    BString selfFd() override;

    // The following helpers operate on object state and must be called while the
    // owning context's condition lock is held (see DevNTSync::wait).
    bool sameContext(NTSyncContext* other) const { return context.get() == other; }
    bool waitSignaled(U32 owner) const;
    U32 waitAcquire(U32 owner); // returns 0 or -K_EOWNERDEAD; only call when waitSignaled() is true

private:
    std::shared_ptr<NTSyncContext> context;
    NTSyncObjectType objectType;
    NTSyncSemArgs sem = {};
    NTSyncMutexArgs mutex = {};
    NTSyncEventArgs event = {};
    bool mutexAbandoned = false;
};

BString KNTSyncObject::selfFd() {
    switch (objectType) {
    case NTSyncObjectType::Semaphore: return B("/dev/ntsync/semaphore");
    case NTSyncObjectType::Mutex: return B("/dev/ntsync/mutex");
    case NTSyncObjectType::Event: return B("/dev/ntsync/event");
    }
    return B("/dev/ntsync/object");
}

bool KNTSyncObject::waitSignaled(U32 owner) const {
    switch (objectType) {
    case NTSyncObjectType::Semaphore:
        return sem.count > 0;
    case NTSyncObjectType::Mutex:
        // signaled if unowned or already owned by this owner, unless the
        // recursion count would overflow
        return (mutex.owner == 0 || mutex.owner == owner) && mutex.count < 0xFFFFFFFF;
    case NTSyncObjectType::Event:
        return event.signaled != 0;
    }
    return false;
}

U32 KNTSyncObject::waitAcquire(U32 owner) {
    switch (objectType) {
    case NTSyncObjectType::Semaphore:
        sem.count--;
        return 0;
    case NTSyncObjectType::Mutex:
        mutex.owner = owner;
        mutex.count++;
        if (mutexAbandoned) {
            mutexAbandoned = false;
            return -K_EOWNERDEAD;
        }
        return 0;
    case NTSyncObjectType::Event:
        if (event.signaled && !event.manual) {
            event.signaled = 0;
        }
        return 0;
    }
    return 0;
}

// Try to satisfy a wait over the given resolved objects. On success acquires the
// relevant object(s), stores the index to report and the ioctl result (0 or
// -K_EOWNERDEAD), and returns true. Does not touch guest memory. Must be called
// with the context lock held.
static bool ntsyncTryAcquire(const std::vector<std::shared_ptr<KNTSyncObject>>& objs,
                             const std::shared_ptr<KNTSyncObject>& alert,
                             U32 owner, bool waitAll, U32& index, U32& result) {
    U32 count = (U32)objs.size();

    if (waitAll) {
        bool all = true;
        for (auto& obj : objs) {
            if (!obj->waitSignaled(owner)) {
                all = false;
                break;
            }
        }
        if (all) {
            // Acquire every object atomically. EOWNERDEAD is reported if any of
            // the acquired mutexes was abandoned.
            result = 0;
            for (auto& obj : objs) {
                U32 r = obj->waitAcquire(owner);
                if (r) {
                    result = r;
                }
            }
            index = 0;
            return true;
        }
    } else {
        for (U32 i = 0; i < count; i++) {
            if (objs[i]->waitSignaled(owner)) {
                result = objs[i]->waitAcquire(owner);
                index = i;
                return true;
            }
        }
    }

    if (alert && alert->waitSignaled(owner)) {
        alert->waitAcquire(owner);
        index = count;
        result = 0;
        return true;
    }
    return false;
}

// Resolve a registered waiter's weak object references into strong ones.
// Returns false (waiter unsatisfiable) if any referenced object has been
// destroyed since it registered.
static bool lockWaiterObjs(const NTSyncWaiter& waiter, std::vector<std::shared_ptr<KNTSyncObject>>& objs,
                           std::shared_ptr<KNTSyncObject>& alert) {
    objs.clear();
    for (auto& weak : waiter.objs) {
        std::shared_ptr<KNTSyncObject> obj = weak.lock();
        if (!obj) {
            return false;
        }
        objs.push_back(std::move(obj));
    }
    if (waiter.hasAlert) {
        alert = waiter.alert.lock();
        if (!alert) {
            return false;
        }
    } else {
        alert = nullptr;
    }
    return true;
}

U32 KNTSyncObject::ioctl(KThread* thread, U32 request) {
    CPU* cpu = thread->cpu;
    U32 address = IOCTL_ARG1;

    BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(context->cond);

    switch (request) {
    case NTSYNC_IOC_SEM_RELEASE: {
        if (objectType != NTSyncObjectType::Semaphore) {
            return -K_EINVAL;
        }
        if (!canRead(thread, address, 4) || !canWrite(thread, address, 4)) {
            return -K_EFAULT;
        }
        U32 release = thread->memory->readd(address);
        U32 previous = sem.count;
        if (release > sem.max || sem.count > sem.max - release) {
            return -K_EOVERFLOW;
        }
        sem.count += release;
        thread->memory->writed(address, previous);
        BOXEDWINE_CONDITION_SIGNAL_ALL(context->cond);
        return 0;
    }
    case NTSYNC_IOC_SEM_READ:
        if (objectType != NTSyncObjectType::Semaphore) {
            return -K_EINVAL;
        }
        if (!canWrite(thread, address, 8)) {
            return -K_EFAULT;
        }
        writeSemArgs(thread, address, sem);
        return 0;
    case NTSYNC_IOC_MUTEX_UNLOCK: {
        if (objectType != NTSyncObjectType::Mutex) {
            return -K_EINVAL;
        }
        if (!canRead(thread, address, 8) || !canWrite(thread, address, 8)) {
            return -K_EFAULT;
        }
        NTSyncMutexArgs args = readMutexArgs(thread, address);
        if (!args.owner) {
            return -K_EINVAL;
        }
        if (mutex.owner != args.owner || !mutex.count) {
            return -K_EPERM;
        }
        U32 previousCount = mutex.count;
        mutex.count--;
        if (!mutex.count) {
            mutex.owner = 0;
            BOXEDWINE_CONDITION_SIGNAL_ALL(context->cond);
        }
        args.count = previousCount;
        writeMutexArgs(thread, address, args);
        return 0;
    }
    case NTSYNC_IOC_MUTEX_KILL: {
        if (objectType != NTSyncObjectType::Mutex) {
            return -K_EINVAL;
        }
        if (!canRead(thread, address, 4)) {
            return -K_EFAULT;
        }
        U32 owner = thread->memory->readd(address);
        if (!owner) {
            return -K_EINVAL;
        }
        if (mutex.owner != owner || !mutex.count) {
            return -K_EPERM;
        }
        mutex.owner = 0;
        mutex.count = 0;
        mutexAbandoned = true;
        BOXEDWINE_CONDITION_SIGNAL_ALL(context->cond);
        return 0;
    }
    case NTSYNC_IOC_MUTEX_READ:
        if (objectType != NTSyncObjectType::Mutex) {
            return -K_EINVAL;
        }
        if (!canWrite(thread, address, 8)) {
            return -K_EFAULT;
        }
        if (mutexAbandoned) {
            writeMutexArgs(thread, address, {});
            return -K_EOWNERDEAD;
        }
        writeMutexArgs(thread, address, mutex);
        return 0;
    case NTSYNC_IOC_EVENT_SET: {
        if (objectType != NTSyncObjectType::Event) {
            return -K_EINVAL;
        }
        if (!canWrite(thread, address, 4)) {
            return -K_EFAULT;
        }
        U32 previous = event.signaled;
        event.signaled = 1;
        thread->memory->writed(address, previous);
        BOXEDWINE_CONDITION_SIGNAL_ALL(context->cond);
        return 0;
    }
    case NTSYNC_IOC_EVENT_RESET: {
        if (objectType != NTSyncObjectType::Event) {
            return -K_EINVAL;
        }
        if (!canWrite(thread, address, 4)) {
            return -K_EFAULT;
        }
        U32 previous = event.signaled;
        event.signaled = 0;
        thread->memory->writed(address, previous);
        return 0;
    }
    case NTSYNC_IOC_EVENT_PULSE: {
        if (objectType != NTSyncObjectType::Event) {
            return -K_EINVAL;
        }
        if (!canWrite(thread, address, 4)) {
            return -K_EFAULT;
        }
        U32 previous = event.signaled;
        // A pulse momentarily signals the event, releases the waiters that are
        // eligible right now, and leaves the event unsignaled. We replicate the
        // kernel behavior by signaling the event and walking the registered
        // waiters in arrival order: each eligible waiter's objects are acquired
        // on its behalf and the wake result is recorded. Because acquiring an
        // auto-reset event designals it, only the first waiter wakes; a
        // manual-reset event stays signaled across the walk, so every eligible
        // waiter wakes. The event is then reset regardless of its prior state.
        event.signaled = 1;
        for (auto& waiter : context->waiters) {
            if (waiter->pulseSatisfied) {
                continue;
            }
            std::vector<std::shared_ptr<KNTSyncObject>> objs;
            std::shared_ptr<KNTSyncObject> alert;
            if (!lockWaiterObjs(*waiter, objs, alert)) {
                continue;
            }
            U32 index = 0;
            U32 result = 0;
            if (ntsyncTryAcquire(objs, alert, waiter->owner, waiter->waitAll, index, result)) {
                waiter->pulseSatisfied = true;
                waiter->pulseIndex = index;
                waiter->pulseResult = result;
            }
        }
        event.signaled = 0;
        thread->memory->writed(address, previous);
        BOXEDWINE_CONDITION_SIGNAL_ALL(context->cond);
        return 0;
    }
    case NTSYNC_IOC_EVENT_READ:
        if (objectType != NTSyncObjectType::Event) {
            return -K_EINVAL;
        }
        if (!canWrite(thread, address, 8)) {
            return -K_EFAULT;
        }
        writeEventArgs(thread, address, event);
        return 0;
    default:
        return -K_ENOTTY;
    }
}

class DevNTSync : public FsVirtualOpenNode {
public:
    DevNTSync(const std::shared_ptr<FsNode>& node, U32 flags) : FsVirtualOpenNode(node, flags), context(std::make_shared<NTSyncContext>()) {}

    U32 ioctl(KThread* thread, U32 request) override;
    U32 readNative(U8* buffer, U32 len) override { return -K_EINVAL; }
    U32 writeNative(U8* buffer, U32 len) override { return -K_EINVAL; }

private:
    U32 createSemaphore(KThread* thread, U32 address);
    U32 createMutex(KThread* thread, U32 address);
    U32 createEvent(KThread* thread, U32 address);
    U32 wait(KThread* thread, U32 address, bool waitAll);

    std::shared_ptr<KNTSyncObject> resolve(KThread* thread, S32 fd);

    // Waiter registry helpers. All must be called with the context lock held.
    std::shared_ptr<NTSyncWaiter> findWaiter(U32 threadId);
    void removeWaiter(U32 threadId);
    void registerWaiter(U32 threadId, const std::vector<std::shared_ptr<KNTSyncObject>>& objs,
                        const std::shared_ptr<KNTSyncObject>& alert, U32 owner, bool waitAll);

    std::shared_ptr<NTSyncContext> context;
};

U32 DevNTSync::createSemaphore(KThread* thread, U32 address) {
    if (!canRead(thread, address, 8)) {
        return -K_EFAULT;
    }
    NTSyncSemArgs args = readSemArgs(thread, address);
    if (args.count > args.max) {
        return -K_EINVAL;
    }
    std::shared_ptr<KObject> object = std::make_shared<KNTSyncObject>(context, args);
    return thread->process->allocFileDescriptor(object, K_O_RDWR, 0, -1, 0)->handle;
}

U32 DevNTSync::createMutex(KThread* thread, U32 address) {
    if (!canRead(thread, address, 8)) {
        return -K_EFAULT;
    }
    NTSyncMutexArgs args = readMutexArgs(thread, address);
    if ((!args.owner && args.count) || (args.owner && !args.count)) {
        return -K_EINVAL;
    }
    std::shared_ptr<KObject> object = std::make_shared<KNTSyncObject>(context, args);
    return thread->process->allocFileDescriptor(object, K_O_RDWR, 0, -1, 0)->handle;
}

U32 DevNTSync::createEvent(KThread* thread, U32 address) {
    if (!canRead(thread, address, 8)) {
        return -K_EFAULT;
    }
    NTSyncEventArgs args = readEventArgs(thread, address);
    std::shared_ptr<KObject> object = std::make_shared<KNTSyncObject>(context, args);
    return thread->process->allocFileDescriptor(object, K_O_RDWR, 0, -1, 0)->handle;
}

std::shared_ptr<KNTSyncObject> DevNTSync::resolve(KThread* thread, S32 fd) {
    KFileDescriptorPtr desc = thread->process->getFileDescriptor(fd);
    if (!desc || desc->kobject->type != KTYPE_NTSYNC) {
        return nullptr;
    }
    std::shared_ptr<KNTSyncObject> obj = std::dynamic_pointer_cast<KNTSyncObject>(desc->kobject);
    if (!obj || !obj->sameContext(context.get())) {
        return nullptr;
    }
    return obj;
}

std::shared_ptr<NTSyncWaiter> DevNTSync::findWaiter(U32 threadId) {
    for (auto& waiter : context->waiters) {
        if (waiter->threadId == threadId) {
            return waiter;
        }
    }
    return nullptr;
}

void DevNTSync::removeWaiter(U32 threadId) {
    std::vector<std::shared_ptr<NTSyncWaiter>>& waiters = context->waiters;
    for (size_t i = 0; i < waiters.size(); i++) {
        if (waiters[i]->threadId == threadId) {
            waiters.erase(waiters.begin() + i);
            return;
        }
    }
}

void DevNTSync::registerWaiter(U32 threadId, const std::vector<std::shared_ptr<KNTSyncObject>>& objs,
                               const std::shared_ptr<KNTSyncObject>& alert, U32 owner, bool waitAll) {
    if (findWaiter(threadId)) {
        // Already registered for this wait (a cooperative re-entry or another
        // loop iteration); the parameters do not change within one wait.
        return;
    }
    std::shared_ptr<NTSyncWaiter> waiter = std::make_shared<NTSyncWaiter>();
    waiter->threadId = threadId;
    waiter->owner = owner;
    waiter->waitAll = waitAll;
    waiter->objs.reserve(objs.size());
    for (auto& obj : objs) {
        waiter->objs.push_back(obj);
    }
    if (alert) {
        waiter->alert = alert;
        waiter->hasAlert = true;
    }
    context->waiters.push_back(waiter);
}

U32 DevNTSync::wait(KThread* thread, U32 address, bool waitAll) {
    KMemory* memory = thread->memory;

    BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(context->cond);

    // If a pulse already acquired this thread's objects, report the recorded
    // result without re-validating: the acquisition already happened, so the
    // result stands even if a referenced fd has since been closed.
    {
        std::shared_ptr<NTSyncWaiter> existing = findWaiter(thread->id);
        if (existing && existing->pulseSatisfied) {
            memory->writed(address + NTSYNC_WAIT_OFF_INDEX, existing->pulseIndex);
            U32 result = existing->pulseResult;
            removeWaiter(thread->id);
            return result;
        }
    }

    if (!canRead(thread, address, NTSYNC_WAIT_ARGS_SIZE) || !canWrite(thread, address + NTSYNC_WAIT_OFF_INDEX, 4)) {
        removeWaiter(thread->id);
        return -K_EFAULT;
    }

    U64 timeout = memory->readq(address + NTSYNC_WAIT_OFF_TIMEOUT);
    U32 objsPtr = memory->readd(address + NTSYNC_WAIT_OFF_OBJS);
    U32 count = memory->readd(address + NTSYNC_WAIT_OFF_COUNT);
    U32 flags = memory->readd(address + NTSYNC_WAIT_OFF_FLAGS);
    U32 owner = memory->readd(address + NTSYNC_WAIT_OFF_OWNER);
    U32 alertFd = memory->readd(address + NTSYNC_WAIT_OFF_ALERT);
    U32 pad = memory->readd(address + NTSYNC_WAIT_OFF_PAD);

    if (pad || (flags & ~NTSYNC_WAIT_REALTIME) || count > NTSYNC_MAX_WAIT_COUNT) {
        removeWaiter(thread->id);
        return -K_EINVAL;
    }
    if (count && !canRead(thread, objsPtr, count * 4)) {
        removeWaiter(thread->id);
        return -K_EFAULT;
    }

    // Resolve and validate every guest fd. Hold shared_ptr references so the
    // objects stay alive for the duration of the (possibly blocking) wait.
    std::vector<std::shared_ptr<KNTSyncObject>> objs;
    objs.reserve(count);
    for (U32 i = 0; i < count; i++) {
        S32 fd = (S32)memory->readd(objsPtr + i * 4);
        std::shared_ptr<KNTSyncObject> obj = resolve(thread, fd);
        if (!obj) {
            removeWaiter(thread->id);
            return -K_EINVAL;
        }
        if (waitAll) {
            // WAIT_ALL may not reference the same object more than once.
            for (auto& existing : objs) {
                if (existing == obj) {
                    removeWaiter(thread->id);
                    return -K_EINVAL;
                }
            }
        }
        objs.push_back(obj);
    }

    std::shared_ptr<KNTSyncObject> alert;
    if (alertFd) {
        alert = resolve(thread, (S32)alertFd);
        if (!alert) {
            removeWaiter(thread->id);
            return -K_EINVAL;
        }
        if (waitAll) {
            for (auto& existing : objs) {
                if (existing == alert) {
                    removeWaiter(thread->id);
                    return -K_EINVAL;
                }
            }
        }
    }

    // The NTSYNC timeout is an absolute value in nanoseconds, measured against
    // CLOCK_MONOTONIC unless NTSYNC_WAIT_REALTIME is set. U64_MAX means wait
    // forever. Convert to a host millisecond deadline; because the deadline is
    // absolute, recomputing it after a cooperative re-entry stays correct.
    bool infinite = (timeout == NTSYNC_TIMEOUT_INFINITE);
    U32 expireMillis = 0;
    if (!infinite) {
        U64 nowMicro = (flags & NTSYNC_WAIT_REALTIME) ? KSystem::getSystemTimeAsMicroSeconds() : KSystem::getMicroCounter();
        S64 remainingMicro = (S64)(timeout / 1000ULL) - (S64)nowMicro;
        S64 remainingMillis = remainingMicro / 1000;
        if (remainingMillis < 0) {
            remainingMillis = 0;
        }
        expireMillis = KSystem::getMilliesSinceStart() + (U32)remainingMillis;
    }

    while (true) {
        // A pulse may have satisfied us while we were blocked (multi-threaded
        // build) or between cooperative re-entries.
        std::shared_ptr<NTSyncWaiter> existing = findWaiter(thread->id);
        if (existing && existing->pulseSatisfied) {
            memory->writed(address + NTSYNC_WAIT_OFF_INDEX, existing->pulseIndex);
            U32 result = existing->pulseResult;
            removeWaiter(thread->id);
            return result;
        }

        if (thread->pendingSignals) {
            if (thread->runSignals()) {
                removeWaiter(thread->id);
                return -K_CONTINUE;
            }
        }

        U32 index = 0;
        U32 result = 0;
        if (ntsyncTryAcquire(objs, alert, owner, waitAll, index, result)) {
            memory->writed(address + NTSYNC_WAIT_OFF_INDEX, index);
            removeWaiter(thread->id);
            return result;
        }

        // Register before sleeping so a concurrent pulse can release us.
        registerWaiter(thread->id, objs, alert, owner, waitAll);

        if (!infinite) {
            S32 diff = (S32)(expireMillis - KSystem::getMilliesSinceStart());
            if (diff <= 0) {
                removeWaiter(thread->id);
                return -K_ETIMEDOUT;
            }
            BOXEDWINE_CONDITION_WAIT_TIMEOUT(context->cond, (U32)diff);
        } else {
            BOXEDWINE_CONDITION_WAIT(context->cond);
        }
#ifdef BOXEDWINE_MULTI_THREADED
        if (thread->terminating) {
            removeWaiter(thread->id);
            return -K_EINTR;
        }
        if (thread->startSignal) {
            thread->startSignal = false;
            removeWaiter(thread->id);
            return -K_CONTINUE;
        }
#endif
    }
}

U32 DevNTSync::ioctl(KThread* thread, U32 request) {
    CPU* cpu = thread->cpu;
    U32 address = IOCTL_ARG1;

    switch (request) {
    case NTSYNC_IOC_CREATE_SEM:
        return createSemaphore(thread, address);
    case NTSYNC_IOC_CREATE_MUTEX:
        return createMutex(thread, address);
    case NTSYNC_IOC_CREATE_EVENT:
        return createEvent(thread, address);
    case NTSYNC_IOC_WAIT_ANY:
        return wait(thread, address, false);
    case NTSYNC_IOC_WAIT_ALL:
        return wait(thread, address, true);
    default:
        return -K_ENOTTY;
    }
}

FsOpenNode* openDevNTSync(const std::shared_ptr<FsNode>& node, U32 flags, U32 data) {
    return new DevNTSync(node, flags);
}
