/*
 *  Copyright (C) 2012-2026  The BoxedWine Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 */

#include "boxedwine.h"

#include "kinotify.h"
#include "kstat.h"

namespace {

BOXEDWINE_MUTEX gInotifyMutex;
std::vector<std::weak_ptr<KInotifyObject> > gInotifyObjects;

U32 align4(U32 value) {
    return (value + 3) & ~3;
}

U32 readDequeU32(const std::deque<U8>& buffer, U32 offset) {
    return (U32)buffer[offset] | ((U32)buffer[offset + 1] << 8) | ((U32)buffer[offset + 2] << 16) | ((U32)buffer[offset + 3] << 24);
}

void appendU32(std::deque<U8>& buffer, U32 value) {
    buffer.push_back((U8)value);
    buffer.push_back((U8)(value >> 8));
    buffer.push_back((U8)(value >> 16));
    buffer.push_back((U8)(value >> 24));
}

void registerInotifyObject(const std::shared_ptr<KInotifyObject>& object) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(gInotifyMutex);
    gInotifyObjects.push_back(object);
}

} // namespace

KInotifyObject::KInotifyObject() : KObject(KTYPE_INOTIFY),
lockCond(std::make_shared<BoxedWineCondition>(B("KInotifyObject::lockCond"))),
createdTime(time(nullptr)) {
}

KInotifyObject::~KInotifyObject() {
    {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(gInotifyMutex);
        gInotifyObjects.erase(std::remove_if(gInotifyObjects.begin(), gInotifyObjects.end(), [this](const std::weak_ptr<KInotifyObject>& item) {
            std::shared_ptr<KInotifyObject> object = item.lock();
            return !object || object.get() == this;
            }), gInotifyObjects.end());
    }
    BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(this->lockCond);
    BOXEDWINE_CONDITION_SIGNAL_ALL(this->lockCond);
}

U32 KInotifyObject::create(KThread* thread, U32 flags) {
    if (flags & ~(K_O_NONBLOCK | K_O_CLOEXEC)) {
        return -K_EINVAL;
    }

    std::shared_ptr<KInotifyObject> object = std::make_shared<KInotifyObject>();
    KFileDescriptorPtr fd = thread->process->allocFileDescriptor(object, K_O_RDONLY, 0, -1, 0);
    if (flags & K_O_CLOEXEC) {
        fd->descriptorFlags |= FD_CLOEXEC;
    }
    if (flags & K_O_NONBLOCK) {
        fd->accessFlags |= K_O_NONBLOCK;
        object->setBlocking(false);
    }
    registerInotifyObject(object);
    return fd->handle;
}

U32 KInotifyObject::addWatch(KThread* thread, FD fd, BString path, U32 mask) {
    KFileDescriptorPtr descriptor = thread->process->getFileDescriptor(fd);
    if (!descriptor) {
        return -K_EBADF;
    }
    if (descriptor->kobject->type != KTYPE_INOTIFY) {
        return -K_EINVAL;
    }
    std::shared_ptr<KInotifyObject> object = std::dynamic_pointer_cast<KInotifyObject>(descriptor->kobject);
    return object->addWatch(thread->process.get(), path, mask);
}

U32 KInotifyObject::removeWatch(KThread* thread, FD fd, S32 wd) {
    KFileDescriptorPtr descriptor = thread->process->getFileDescriptor(fd);
    if (!descriptor) {
        return -K_EBADF;
    }
    if (descriptor->kobject->type != KTYPE_INOTIFY) {
        return -K_EINVAL;
    }
    std::shared_ptr<KInotifyObject> object = std::dynamic_pointer_cast<KInotifyObject>(descriptor->kobject);
    return object->removeWatch(wd);
}

void KInotifyObject::notifyPath(const BString& fullPath, U32 mask) {
    BString parentPath = Fs::getParentPath(fullPath);
    BString name = Fs::getFileNameFromPath(fullPath);
    std::vector<std::shared_ptr<KInotifyObject> > objects;

    {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(gInotifyMutex);
        for (auto it = gInotifyObjects.begin(); it != gInotifyObjects.end();) {
            std::shared_ptr<KInotifyObject> object = it->lock();
            if (object) {
                objects.push_back(object);
                ++it;
            } else {
                it = gInotifyObjects.erase(it);
            }
        }
    }

    for (auto& object : objects) {
        object->queueEvent(parentPath, name, mask);
    }
}

U32 KInotifyObject::addWatch(KProcess* process, const BString& path, U32 mask) {
    if ((mask & K_IN_ALL_EVENTS) == 0) {
        return -K_EINVAL;
    }

    BString fullPath = Fs::getFullPath(process->currentDirectory, path);
    std::shared_ptr<FsNode> node = Fs::getNodeFromLocalPath(B(""), fullPath, (mask & K_IN_DONT_FOLLOW) == 0);
    if (!node) {
        return -K_ENOENT;
    }
    if ((mask & K_IN_ONLYDIR) && !node->isDirectory()) {
        return -K_ENOTDIR;
    }

    BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(this->lockCond);
    for (Watch& watch : this->watches) {
        if (watch.path == node->path) {
            if (mask & K_IN_MASK_ADD) {
                watch.mask |= mask;
            } else {
                watch.mask = mask;
            }
            return watch.wd;
        }
    }

    Watch watch;
    watch.wd = this->nextWatch++;
    watch.path = node->path;
    watch.mask = mask;
    this->watches.push_back(watch);
    return watch.wd;
}

U32 KInotifyObject::removeWatch(S32 wd) {
    BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(this->lockCond);
    for (auto it = this->watches.begin(); it != this->watches.end(); ++it) {
        if (it->wd == wd) {
            this->watches.erase(it);
            return 0;
        }
    }
    return -K_EINVAL;
}

void KInotifyObject::queueEvent(const BString& parentPath, const BString& name, U32 mask) {
    U32 eventMask = mask & K_IN_ALL_EVENTS;
    bool queued = false;

    BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(this->lockCond);
    for (const Watch& watch : this->watches) {
        if (watch.path == parentPath && (watch.mask & eventMask)) {
            this->appendEvent(watch.wd, mask, name);
            queued = true;
        }
    }
    if (queued) {
        BOXEDWINE_CONDITION_SIGNAL_ALL(this->lockCond);
    }
}

void KInotifyObject::appendEvent(S32 wd, U32 mask, const BString& name) {
    U32 rawNameLen = (U32)name.length();
    U32 nameLen = rawNameLen ? align4(rawNameLen + 1) : 0;

    appendU32(this->recvBuffer, (U32)wd);
    appendU32(this->recvBuffer, mask);
    appendU32(this->recvBuffer, 0);
    appendU32(this->recvBuffer, nameLen);
    if (nameLen) {
        for (U32 i = 0; i < rawNameLen; ++i) {
            this->recvBuffer.push_back((U8)name.charAt(i));
        }
        for (U32 i = rawNameLen; i < nameLen; ++i) {
            this->recvBuffer.push_back(0);
        }
    }
}

U32 KInotifyObject::eventLengthAt(U32 offset) {
    if (this->recvBuffer.size() < offset + 16) {
        return 0;
    }
    return 16 + readDequeU32(this->recvBuffer, offset + 12);
}

U32 KInotifyObject::bytesAvailableForRead(U32 len) {
    U32 result = 0;
    while (result < this->recvBuffer.size()) {
        U32 eventLen = this->eventLengthAt(result);
        if (!eventLen || result + eventLen > this->recvBuffer.size()) {
            break;
        }
        if (result + eventLen > len) {
            break;
        }
        result += eventLen;
    }
    return result;
}

void KInotifyObject::setBlocking(bool blocking) {
    BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(this->lockCond);
    this->blocking = blocking;
}

bool KInotifyObject::isBlocking() {
    return this->blocking;
}

void KInotifyObject::setAsync(bool isAsync) {
    this->async = isAsync;
}

bool KInotifyObject::isAsync() {
    return this->async;
}

KFileLock* KInotifyObject::getLock(KFileLock* lock) {
    return nullptr;
}

U32 KInotifyObject::setLock(KFileLock* lock, bool wait) {
    return -K_EINVAL;
}

bool KInotifyObject::supportsLocks() {
    return false;
}

bool KInotifyObject::isOpen() {
    return true;
}

bool KInotifyObject::isReadReady() {
    return this->recvBuffer.size() != 0;
}

bool KInotifyObject::isWriteReady() {
    return true;
}

void KInotifyObject::waitForEvents(BOXEDWINE_CONDITION& parentCondition, U32 events) {
    bool addedLock = false;

    if (events & K_POLLIN) {
        BOXEDWINE_CONDITION_ADD_PARENT(this->lockCond, parentCondition);
        addedLock = true;
    }
    if (events & K_POLLOUT) {
        if (!addedLock) {
            BOXEDWINE_CONDITION_ADD_PARENT(this->lockCond, parentCondition);
            addedLock = true;
        }
    }
    if (events && (events & ~(K_POLLIN | K_POLLOUT))) {
        if (!addedLock) {
            BOXEDWINE_CONDITION_ADD_PARENT(this->lockCond, parentCondition);
        }
    }
    if (events == 0) {
        BOXEDWINE_CONDITION_REMOVE_PARENT(this->lockCond, parentCondition);
    }
}

U32 KInotifyObject::write(KThread* thread, U32 buffer, U32 len) {
    return -K_EINVAL;
}

U32 KInotifyObject::writeNative(U8* buffer, U32 len) {
    return -K_EINVAL;
}

U32 KInotifyObject::read(KThread* thread, U32 buffer, U32 len) {
    if (!thread->memory->canWrite(buffer, len)) {
        return -K_EFAULT;
    }

    BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(this->lockCond);
    while (this->recvBuffer.size() == 0) {
        if (!this->blocking) {
            return -K_EWOULDBLOCK;
        }
        BOXEDWINE_CONDITION_WAIT(this->lockCond);
#ifdef BOXEDWINE_MULTI_THREADED
        if (thread->terminating) {
            return -K_EINTR;
        }
        if (thread->startSignal) {
            thread->startSignal = false;
            return -K_CONTINUE;
        }
#endif
    }

    U32 count = this->bytesAvailableForRead(len);
    if (!count) {
        return -K_EINVAL;
    }
    thread->memory->performOnMemory(buffer, count, false, [this](U8* ram, U32 len) {
        std::copy(this->recvBuffer.begin(), this->recvBuffer.begin() + len, ram);
        this->recvBuffer.erase(this->recvBuffer.begin(), this->recvBuffer.begin() + len);
        return true;
        });
    return count;
}

U32 KInotifyObject::readNative(U8* buffer, U32 len) {
    BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(this->lockCond);
    while (this->recvBuffer.size() == 0) {
        if (!this->blocking) {
            return -K_EWOULDBLOCK;
        }
        BOXEDWINE_CONDITION_WAIT(this->lockCond);
#ifdef BOXEDWINE_MULTI_THREADED
        if (KThread::currentThread()->terminating) {
            return -K_EINTR;
        }
        if (KThread::currentThread()->startSignal) {
            KThread::currentThread()->startSignal = false;
            return -K_CONTINUE;
        }
#endif
    }

    U32 count = this->bytesAvailableForRead(len);
    if (!count) {
        return -K_EINVAL;
    }
    std::copy(this->recvBuffer.begin(), this->recvBuffer.begin() + count, buffer);
    this->recvBuffer.erase(this->recvBuffer.begin(), this->recvBuffer.begin() + count);
    return count;
}

U32 KInotifyObject::stat(KProcess* process, U32 address, bool is64) {
    KSystem::writeStat(process, B(""), address, is64, true, 0, K_S_IFSOCK | K__S_IWRITE | K__S_IREAD, 0, 0, 4096, 0, this->createdTime, 1);
    return 0;
}

U32 KInotifyObject::map(KThread* thread, U32 address, U32 len, S32 prot, S32 flags, U64 off) {
    return 0;
}

bool KInotifyObject::canMap() {
    return false;
}

BString KInotifyObject::selfFd() {
    return B("anon_inode:inotify");
}

S64 KInotifyObject::seek(S64 pos) {
    return -K_ESPIPE;
}

S64 KInotifyObject::getPos() {
    return 0;
}

U32 KInotifyObject::ioctl(KThread* thread, U32 request) {
    return -K_ENOTTY;
}

S64 KInotifyObject::length() {
    return -1;
}
