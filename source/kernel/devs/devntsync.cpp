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

struct NTSyncContext {
    NTSyncContext() : cond(std::make_shared<BoxedWineCondition>(B("NTSyncContext::cond"))) {}

    BOXEDWINE_MUTEX mutex;
    BOXEDWINE_CONDITION cond;
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
    BString selfFd() override { return B("/dev/ntsync/object"); }

private:
    std::shared_ptr<NTSyncContext> context;
    NTSyncObjectType objectType;
    NTSyncSemArgs sem = {};
    NTSyncMutexArgs mutex = {};
    NTSyncEventArgs event = {};
    bool mutexAbandoned = false;
};

U32 KNTSyncObject::ioctl(KThread* thread, U32 request) {
    U32 address = IOCTL_ARG1;

    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(context->mutex);

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

U32 DevNTSync::ioctl(KThread* thread, U32 request) {
    U32 address = IOCTL_ARG1;

    switch (request) {
    case NTSYNC_IOC_CREATE_SEM:
        return createSemaphore(thread, address);
    case NTSYNC_IOC_CREATE_MUTEX:
        return createMutex(thread, address);
    case NTSYNC_IOC_CREATE_EVENT:
        return createEvent(thread, address);
    case NTSYNC_IOC_WAIT_ANY:
    case NTSYNC_IOC_WAIT_ALL:
        return -K_ENOSYS;
    default:
        return -K_ENOTTY;
    }
}

FsOpenNode* openDevNTSync(const std::shared_ptr<FsNode>& node, U32 flags, U32 data) {
    return new DevNTSync(node, flags);
}
