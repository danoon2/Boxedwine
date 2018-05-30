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

#include "ksignal.h"
#include "kscheduler.h"

void KSigAction::writeSigAction(U32 address, U32 sigsetSize) {
    writed(address, this->handlerAndSigAction);
    writed(address + 4, this->flags);
    writed(address + 8, this->restorer);
    if (sigsetSize==4)
        writed(address + 12, (U32)this->mask);
    else if (sigsetSize==8)
        writeq(address + 12, this->mask);
    else
        klog("writeSigAction: can't handle sigsetSize=%d", sigsetSize);
}

void KSigAction::readSigAction(U32 address, U32 sigsetSize) {
    this->handlerAndSigAction = readd(address);
    this->flags = readd(address + 4);
    this->restorer = readd(address + 8);
    if (sigsetSize==4)
        this->mask = readd(address + 12);
    else if (sigsetSize==8)
        this->mask = readq(address + 12);
    else
        klog("readSigAction: can't handle sigsetSize=%d", sigsetSize);
}

KSignal::KSignal() : KObject(KTYPE_SIGNAL), blocking(false), mask(0) {
}

void KSignal::setBlocking(bool blocking) {
    this->blocking = blocking;
}

bool KSignal::isBlocking() {
    return this->blocking;
}

void KSignal::setAsync(bool isAsync) {
    if (isAsync)
        kpanic("KSignal::setAsync not implemented yet");
}

bool KSignal::isAsync() {
    return false;
}

KFileLock* KSignal::getLock(KFileLock* lock) {
    kwarn("KSignal::getLock not implemented yet");
    return 0;
}

U32 KSignal::setLock(KFileLock* lock, bool wait) {
    kwarn("KSignal::setLock not implemented yet");
    return -1;
}

bool KSignal::isOpen() {
    return true;
}

void KSignal::waitForEvents(U32 events) {
    if (events & K_POLLIN) {
		this->waitingThreads.addToBack(KThread::currentThread()->getWaitNofiyNode());
    }
}

bool KSignal::isReadReady() {
    KThread* thread = KThread::currentThread();
    if ((thread->process->pendingSignals & this->mask) || (thread->pendingSignals & this->mask))
        return true;
    return false;
}

bool KSignal::isWriteReady() {
    kpanic("KSignal::isWriteReady not implemented yet");
    return false;
}

U32 KSignal::writeNative(U8* buffer, U32 len) {
    kpanic("KSignal::writeNative not implemented yet");
    return 0;
}

U32 KSignal::readNative(U8* buffer, U32 len) {
    kpanic("KSignal::readNative not implemented yet");
    return 0;
}

U32 KSignal::stat(U32 address, bool is64) {
    kpanic("KSignal::stat not implemented yet");
    return 0;
}

U32 KSignal::map(U32 address, U32 len, S32 prot, S32 flags, U64 off) {
    return 0;
}

bool KSignal::canMap() {
    return false;
}

S64 KSignal::seek(S64 pos) {
    return -K_ESPIPE; // :TODO: is this right?
}

S64 KSignal::getPos() {
    return 0;
}

U32 KSignal::ioctl(U32 request) {
    return -K_ENOTTY;
}

bool KSignal::supportsLocks() {
    return false;
}

S64 KSignal::length() {
    return -1;
}

U32 syscall_signalfd4(S32 fildes, U32 mask, U32 flags) {
    KFileDescriptor* fd;
    KThread* thread = KThread::currentThread();

    if (fildes>=0) {
        fd = thread->process->getFileDescriptor(fildes);
        if (!fd)
            return -K_EBADF;
        if (fd->kobject->type!=KTYPE_SIGNAL)
            return -K_EINVAL;
    } else {
        BoxedPtr<KSignal> o = new KSignal();
        fd =  thread->process->allocFileDescriptor(o, K_O_RDONLY, 0, -1, 0);
    }    
    if (flags & K_O_CLOEXEC) {
        fd->descriptorFlags|=FD_CLOEXEC;
    }
    if (flags & K_O_NONBLOCK) {
        fd->accessFlags |= K_O_NONBLOCK;
    }
    BoxedPtr<KSignal> s = (KSignal*)fd->kobject.get();
    s->mask = readd(mask);
    s->blocking = (fd->accessFlags & K_O_NONBLOCK)!=0;
    return fd->handle;
}