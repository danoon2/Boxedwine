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

#include "kepoll.h"
#include "kscheduler.h"

#include <string.h>

KEPoll::KEPoll() : KObject(KTYPE_EPOLL) {
}

KEPoll::~KEPoll() {
     for( const auto& n : this->data ) {
         delete n.second;
    }
}

void KEPoll::setBlocking(bool blocking) {
    if (blocking)
        kpanic("KEPoll::setBlocking not implemented yet");
}

bool KEPoll::isBlocking() {
    return false;
}

void KEPoll::setAsync(bool isAsync) {
    if (isAsync)
        kpanic("KEPoll::setAsync not implemented yet");
}

bool KEPoll::isAsync() {
    return false;
}

KFileLock* KEPoll::getLock(KFileLock* lock) {
    kwarn("KEPoll::getLock not implemented yet");
    return 0;
}

U32 KEPoll::setLock(KFileLock* lock, bool wait) {
    kwarn("KEPoll::setLock not implemented yet");
    return -1;
}

bool KEPoll::isOpen() {
    kpanic("KEPoll::isOpen not implemented yet");
    return false;
}

void KEPoll::waitForEvents(BOXEDWINE_CONDITION& parentCondition, U32 events) {
    kpanic("KEPoll::waitForEvents not implemented yet");
}

bool KEPoll::isReadReady() {
    kpanic("KEPoll::isReadReady not implemented yet");
    return false;
}

bool KEPoll::isWriteReady() {
    kpanic("KEPoll::isWriteReady not implemented yet");
    return false;
}

U32 KEPoll::writeNative(U8* buffer, U32 len) {
    kpanic("KEPoll::writeNative not implemented yet");
    return 0;
}

U32 KEPoll::readNative(U8* buffer, U32 len) {
    kpanic("KEPoll::readNative not implemented yet");
    return 0;
}

U32 KEPoll::stat(U32 address, bool is64) {
    kpanic("KEPoll::stat not implemented yet");
    return 0;
}

U32 KEPoll::map(U32 address, U32 len, S32 prot, S32 flags, U64 off) {
    return 0;
}

bool KEPoll::canMap() {
    return false;
}

S64 KEPoll::seek(S64 pos) {
    return -K_ESPIPE; // :TODO: is this right?
}

S64 KEPoll::getPos() {
    return 0;
}

U32 KEPoll::ioctl(U32 request) {
    return -K_ENOTTY;
}

bool KEPoll::supportsLocks() {
    return false;
}

S64 KEPoll::length() {
    return -1;
}

void KEPoll::close() {
}


#define K_EPOLL_CTL_ADD 1
#define K_EPOLL_CTL_DEL 2
#define K_EPOLL_CTL_MOD 3

U32 KEPoll::ctl(U32 op, FD fd, U32 address) {
    KFileDescriptor* targetFD = KThread::currentThread()->process->getFileDescriptor(fd);
    Data* existing = NULL;

    if (!targetFD) {
        return -K_EBADF;
    }

    if (this->data.count(fd))
        existing = this->data[fd];

    switch (op) {
        case K_EPOLL_CTL_ADD:
            if (existing) {
                return -K_EEXIST;
            }
            existing = new Data();
            existing->fd = fd;
            existing->events = readd(address);
            existing->data = readq(address + 4);
            this->data[fd] = existing;
            break;
        case K_EPOLL_CTL_DEL:
            if (!existing)
                return -K_ENOENT;
            this->data.erase(fd);
            delete existing;
            break;
        case K_EPOLL_CTL_MOD:
            if (!existing)
                return -K_ENOENT;
            existing->events = readd(address);
            existing->data = readq(address + 4);
            break;
        default:
            return -K_EINVAL;
    }
    return 0;
}

U32 KEPoll::wait(U32 events, U32 maxevents, U32 timeout) {    
    S32 result = 0;
    U32 i;
    KThread* thread = KThread::currentThread();
    static KPollData pollData[256];	
    U32 pollCount=0;

    for( const auto& n : this->data ) {
        if (pollCount>=256) {
            kpanic("Wasn't expect a poll count of more than 256");
            break;
        }
        Data* next = n.second;
        pollData[pollCount].events = next->events;
        pollData[pollCount].fd = next->fd;
        pollData[pollCount].data = next->data;
        pollCount++;	
    }
    result = internal_poll(pollData, pollCount, timeout);
    if (result >= 0) {
        result = 0;
        for (i=0;i<pollCount;i++) {
            if (pollData[i].revents!=0) {
                writed(events + result * 12, pollData[i].revents);        
                writeq(events + result * 12 + 4, pollData[i].data);
                result++;
                if (result>=(S32)maxevents) {
                    kwarn("possible starvation in epoll, more events are ready than can be received.");
                    break;
                }
            }        
        }
    }
    return result;
}