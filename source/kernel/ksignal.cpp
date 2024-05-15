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

void KSigAction::writeSigAction(KMemory* memory, U32 address, U32 sigsetSize) {
    memory->writed(address, this->handlerAndSigAction);
    memory->writed(address + 4, this->flags);
    memory->writed(address + 8, this->restorer);
    if (sigsetSize==4)
        memory->writed(address + 12, (U32)this->mask);
    else if (sigsetSize==8)
        memory->writeq(address + 12, this->mask);
    else
        klog("writeSigAction: can't handle sigsetSize=%d", sigsetSize);
}

void KSigAction::readSigAction(KMemory* memory, U32 address, U32 sigsetSize) {
    this->handlerAndSigAction = memory->readd(address);
    this->flags = memory->readd(address + 4);
    this->restorer = memory->readd(address + 8);
    if (sigsetSize==4)
        this->mask = memory->readd(address + 12);
    else if (sigsetSize==8)
        this->mask = memory->readq(address + 12);
    else
        klog("readSigAction: can't handle sigsetSize=%d", sigsetSize);
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
    kdebug("KSignal::getLock not implemented yet");
    return nullptr;
}

U32 KSignal::setLock(KFileLock* lock, bool wait) {
    kdebug("KSignal::setLock not implemented yet");
    return -1;
}

bool KSignal::isOpen() {
    return true;
}

void KSignal::waitForEvents(BOXEDWINE_CONDITION& parentCondition, U32 events) {
    if (events & K_POLLIN) {
        BOXEDWINE_CONDITION_ADD_PARENT(this->lockCond, parentCondition);
    } else {
        BOXEDWINE_CONDITION_REMOVE_PARENT(this->lockCond, parentCondition);
    }
    if (events & K_POLLOUT) {
        kpanic("waiting on a signal not implemented yet");
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

void writeSignal(U8* buffer, U32 signal, U32 signalingPid, U32 signalingUid, KSigAction* action) {
    U32* b = (U32*)buffer;
    b[0] = signal;
    b[1] = 0; // ssi_errno;    /* Error number (unused) */
    b[2] = action?action->sigInfo[2]:0; // ssi_code;     /* Signal code */
    b[3] = signalingPid;    
    b[4] = signalingUid;
    b[5] = 0; // ssi_fd;       /* File descriptor (SIGIO) */
    b[6] = 0; // ssi_tid;      /* Kernel timer ID (POSIX timers)
    b[7] = 0; // ssi_band;     /* Band event (SIGIO) */
    b[8] = 0; // ssi_overrun;  /* POSIX timer overrun count */
    b[9] = 0; // ssi_trapno;   /* Trap number that caused signal */
    b[10] = 0; // ssi_status;   /* Exit status or signal (SIGCHLD) */    
    b[11] = 0; // ssi_int;      /* Integer sent by sigqueue(3) */
    b[12] = 0; // ssi_ptr;      /* Pointer sent by sigqueue(3) */
    b[13] = 0; 
    b[14] = 0; // ssi_utime;    /* User CPU time consumed (SIGCHLD) */
    b[15] = 0;
    b[16] = 0; // ssi_stime;    /* System CPU time consumed (SIGCHLD) */
    b[17] = 0;
    b[18] = 0; // ssi_addr;     /* Address that generated signal (for hardware-generated signals) */
    b[19] = 0;
    b[20] = 0; // ssi_addr_lsb; /* Least significant bit of address (SIGBUS; since Linux 2.6.37)

    if (action && action->sigInfo[0] == K_SIGCHLD) {
        b[10] = action->sigInfo[5];
    }
    if (action && action->sigInfo[0] == K_SIGIO) {
        b[5] = action->sigInfo[4];
        b[7] = action->sigInfo[3];
    }
}

U32 KSignal::readNative(U8* buffer, U32 len) {
    if (len<128) {
        return -K_EINVAL; // :TODO: couldn't not find documentation on what is returned
    }
    KThread* thread = KThread::currentThread();
    while (true) {
        U32 result = 0;
        {
            BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(thread->pendingSignalsMutex);        
            if (thread->pendingSignals & this->mask) {
                U64 todo = thread->pendingSignals & this->mask;
                for (U32 i=0;i<32;i++) {
                    if ((todo & ((U64)1 << i))!=0) {
                        thread->pendingSignals &= ~(1 << i);                
                        writeSignal(buffer, i, this->signalingPid, this->signalingUid, (this->sigAction.sigInfo[0]==i)?&this->sigAction: nullptr);
                        result+=128;
                        len-=128;
                        buffer+=128;
                        if (len<128) {
                            return result;
                        }
                    }
                }
            }
        }
        {
            BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(thread->process->pendingSignalsMutex);        
            if (thread->process->pendingSignals & this->mask) {
                U64 todo = thread->process->pendingSignals & this->mask;
                for (U32 i=0;i<32;i++) {
                    if ((todo & ((U64)1 << i))!=0) {
                        thread->process->pendingSignals &= ~(1 << i);                
                        writeSignal(buffer, i, this->signalingPid, this->signalingUid, (this->sigAction.sigInfo[0]==i)?&this->sigAction: nullptr);
                        result+=128;
                        len-=128;
                        buffer+=128;
                        if (len<128) {
                            return result;
                        }
                    }
                }
            }
        }
        if (result) {
            return result;
        }
        if (!this->blocking) {
            break;
        }
        {
            BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(this->lockCond);
            BOXEDWINE_CONDITION_WAIT(this->lockCond);
        }

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
    return 0;
}

U32 KSignal::stat(KProcess* process, U32 address, bool is64) {
    kpanic("KSignal::stat not implemented yet");
    return 0;
}

U32 KSignal::map(KThread* thread, U32 address, U32 len, S32 prot, S32 flags, U64 off) {
    return 0;
}

bool KSignal::canMap() {
    return false;
}

BString KSignal::selfFd() {
    return B("anon_inode:[signal]");
}

S64 KSignal::seek(S64 pos) {
    return -K_ESPIPE; // :TODO: is this right?
}

S64 KSignal::getPos() {
    return 0;
}

U32 KSignal::ioctl(KThread* thread, U32 request) {
    return -K_ENOTTY;
}

bool KSignal::supportsLocks() {
    return false;
}

S64 KSignal::length() {
    return -1;
}

U32 syscall_signalfd4(KThread* thread, S32 fildes, U32 mask, U32 maskSize, U32 flags) {
    KFileDescriptor* fd = nullptr;
    KMemory* memory = thread->memory;

    if (fildes>=0) {
        fd = thread->process->getFileDescriptor(fildes);
        if (!fd)
            return -K_EBADF;
        if (fd->kobject->type!=KTYPE_SIGNAL)
            return -K_EINVAL;
    } else {
        std::shared_ptr<KSignal> o = std::make_shared<KSignal>();
        fd =  thread->process->allocFileDescriptor(o, K_O_RDONLY, 0, -1, 0);
    }    
    if (flags & K_O_CLOEXEC) {
        fd->descriptorFlags|=FD_CLOEXEC;
    }
    if (flags & K_O_NONBLOCK) {
        fd->accessFlags |= K_O_NONBLOCK;
    }
    std::shared_ptr<KSignal> s = std::dynamic_pointer_cast<KSignal>(fd->kobject);
    if (maskSize==4) {
        s->mask = memory->readd(mask);
    } else if (maskSize==8) {
        s->mask = memory->readq(mask);
    } else {
        kpanic("syscall_signalfd4 unknown mask size: %d", maskSize);
    }
    s->blocking = (fd->accessFlags & K_O_NONBLOCK) == 0;
    return fd->handle;
}
