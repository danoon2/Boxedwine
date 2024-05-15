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

#ifndef __KOBJECT_H__
#define __KOBJECT_H__

#define KTYPE_FILE 0
#define KTYPE_UNIX_SOCKET 1
#define KTYPE_NATIVE_SOCKET 2
#define KTYPE_EPOLL 3
#define KTYPE_SIGNAL 4
#define KTYPE_TIMER 5
#define KTYPE_EVENT 6

// can be shared between processes (see kunixsocket sendmsg/recvmsg) but in each process they will have their own file descriptor
class KObject : public std::enable_shared_from_this<KObject> {
protected:
    KObject(U32 type);
public:
    virtual ~KObject() {};    
    virtual U32 ioctl(KThread* thread, U32 request)=0;
    virtual S64 seek(S64 pos)=0;
    virtual S64 length()=0;
    virtual S64 getPos()=0;
    virtual void setBlocking(bool blocking)=0;
    virtual bool isBlocking()=0;
    virtual void setAsync(bool isAsync)=0;
    virtual bool isAsync()=0;
    virtual KFileLock* getLock(KFileLock* lock)=0;
    virtual U32 setLock(KFileLock* lock, bool wait)=0;
    virtual void unlockAll(U32 pid) {}
    virtual bool supportsLocks()=0;
    virtual bool isOpen()=0;
    virtual bool isReadReady()=0;
    virtual bool isPriorityReadReady() { return isReadReady(); }
    virtual bool isWriteReady()=0;
    virtual void waitForEvents(BOXEDWINE_CONDITION& parentCondition, U32 events)=0;
    virtual U32 write(KThread* thread, U32 buffer, U32 len);
    virtual U32 writeNative(U8* buffer, U32 len)=0;
    virtual U32 writev(KThread* thread, U32 iov, S32 iovcnt);
    virtual U32 read(KThread* thread, U32 buffer, U32 len);
    virtual U32 readNative(U8* buffer, U32 len)=0;
    virtual U32 stat(KProcess* process, U32 address, bool is64)=0;
    virtual U32 map(KThread* thread, U32 address, U32 len, S32 prot, S32 flags, U64 off)=0;
    virtual bool canMap()=0;
    virtual BString selfFd()=0;

    U32 type;
};

#endif