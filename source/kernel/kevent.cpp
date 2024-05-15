/*
 *  Copyright (C) 2016  The BoxedWine Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "boxedwine.h"
#include "kevent.h"

void KEvent::setBlocking(bool blocking) {
    this->blocking = blocking;
}

bool KEvent::isBlocking() {
    return this->blocking;
}

void KEvent::setAsync(bool isAsync) {
    if (isAsync)
        kpanic("KEvent::setAsync not implemented yet");
}

bool KEvent::isAsync() {
    return false;
}

KFileLock* KEvent::getLock(KFileLock* lock) {
    kdebug("KEvent::getLock not implemented yet");
    return nullptr;
}

U32 KEvent::setLock(KFileLock* lock, bool wait) {
    kdebug("KEvent::setLock not implemented yet");
    return -1;
}

bool KEvent::isOpen() {
    return true;
}

void KEvent::waitForEvents(BOXEDWINE_CONDITION& parentCondition, U32 events) {
    if ((events & K_POLLIN) || (events & K_POLLOUT)) {
        BOXEDWINE_CONDITION_ADD_PARENT(this->lockCond, parentCondition);
    } else {
        BOXEDWINE_CONDITION_REMOVE_PARENT(this->lockCond, parentCondition);
    }
}

bool KEvent::isReadReady() {
    KThread* thread = KThread::currentThread();
    if (counter) {
        return true;
    }
    return false;
}

bool KEvent::isWriteReady() {
    return counter != 0xffffffffffffffffl;
}

U32 KEvent::writeNative(U8* buffer, U32 len) {
    if (len < 8) {
        return -K_EINVAL;
    }
    U64 value = *(U64*)buffer;
    BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(lockCond);
    while (true) {
        U64 result = counter + value;
        if (result < counter || result < value) {
            if (!blocking) {
                return -K_EAGAIN;
            }
            BOXEDWINE_CONDITION_WAIT(lockCond);
        } else {
            counter = result;
            BOXEDWINE_CONDITION_SIGNAL(lockCond);
            break;
        }
    }
    return 8;
}

U32 KEvent::readNative(U8* buffer, U32 len) {
    if (len < 8) {
        return -K_EINVAL;
    }
    BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(lockCond);
    while (true) {
        if (counter) {
            *((U64*)buffer) = counter;
            counter = 0;
            BOXEDWINE_CONDITION_SIGNAL(lockCond);
            return 8;
        }
        if (blocking) {
            BOXEDWINE_CONDITION_WAIT(lockCond);
        } else {
            return -K_EAGAIN;
        }
    }
}

U32 KEvent::stat(KProcess* process, U32 address, bool is64) {
    kpanic("KEvent::stat not implemented yet");
    return 0;
}

U32 KEvent::map(KThread* thread, U32 address, U32 len, S32 prot, S32 flags, U64 off) {
    return 0;
}

bool KEvent::canMap() {
    return false;
}

BString KEvent::selfFd() {
    return B("anon_inode:[event]");
}

S64 KEvent::seek(S64 pos) {
    return -K_ESPIPE; // :TODO: is this right?
}

S64 KEvent::getPos() {
    return 0;
}

U32 KEvent::ioctl(KThread* thread, U32 request) {
    return -K_ENOTTY;
}

bool KEvent::supportsLocks() {
    return false;
}

S64 KEvent::length() {
    return -1;
}

U32 syscall_eventfd2(KThread* thread, U32 initialValue, U32 flags) {
    KFileDescriptor* fd = nullptr;
    KMemory* memory = thread->memory;

    std::shared_ptr<KEvent> o = std::make_shared<KEvent>();
    o->counter = initialValue;
    fd = thread->process->allocFileDescriptor(o, K_O_RDWR, 0, -1, 0);

    if (flags & K_O_CLOEXEC) {
        fd->descriptorFlags |= FD_CLOEXEC;
    }
    if (flags & K_O_NONBLOCK) {
        fd->accessFlags |= K_O_NONBLOCK;
    }
    U32 unusedFlags = flags;
    unusedFlags &= ~K_O_NONBLOCK;
    unusedFlags &= ~K_O_CLOEXEC;
    if (unusedFlags) {
        kwarn("syscall_eventfd2 unhandled flags=%X", unusedFlags);
    }
    o->blocking = (fd->accessFlags & K_O_NONBLOCK) == 0;
    return fd->handle;
}