#ifndef __KNATIVESOCKET_H__
#define __KNATIVESOCKET_H__

#include "ksocketobject.h"

class KNativeSocketObject : public KSocketObject {
public:
    KNativeSocketObject(U32 domain, U32 type, U32 protocol);

    virtual ~KNativeSocketObject();    
    virtual U32  ioctl(U32 request);
    virtual S64  seek(S64 pos);
    virtual S64  length();
    virtual S64  getPos();
    virtual void setBlocking(bool blocking);
    virtual bool isBlocking();
    virtual void setAsync(bool isAsync);
    virtual bool isAsync();
    virtual KFileLock* getLock(KFileLock* lock);
    virtual U32 setLock(KFileLock* lock, bool wait);
    virtual bool supportsLocks();
    virtual bool isOpen();
    virtual bool isReadReady();
    virtual bool isPriorityReadReady();
    virtual bool isWriteReady();
    virtual void waitForEvents(BOXEDWINE_CONDITION& parentCondition, U32 events);
    virtual U32  writeNative(U8* buffer, U32 len);
    virtual U32  readNative(U8* buffer, U32 len);
    virtual U32  stat(U32 address, bool is64);
    virtual U32  map(U32 address, U32 len, S32 prot, S32 flags, U64 off);
    virtual bool canMap();

    virtual U32 accept(KFileDescriptor* fd, U32 address, U32 len, U32 flags);
    virtual U32 bind(KFileDescriptor* fd, U32 address, U32 len);
    virtual U32 connect(KFileDescriptor* fd, U32 address, U32 len);
    virtual U32 getpeername(KFileDescriptor* fd, U32 address, U32 plen);
    virtual U32 getsockname(KFileDescriptor* fd, U32 address, U32 plen);
    virtual U32 getsockopt(KFileDescriptor* fd, U32 level, U32 name, U32 value, U32 len_address);
    virtual U32 listen(KFileDescriptor* fd, U32 backlog);
    virtual U32 recvfrom(KFileDescriptor* fd, U32 buffer, U32 length, U32 flags, U32 address, U32 address_len);
    virtual U32 recvmsg(KFileDescriptor* fd, U32 address, U32 flags);
    virtual U32 sendmsg(KFileDescriptor* fd, U32 address, U32 flags);
    virtual U32 sendto(KFileDescriptor* fd, U32 message, U32 length, U32 flags, U32 dest_addr, U32 dest_len);
    virtual U32 setsockopt(KFileDescriptor* fd, U32 level, U32 name, U32 value, U32 len);
    virtual U32 shutdown(KFileDescriptor* fd, U32 how);

    S32 nativeSocket;
    bool connecting;

    BOXEDWINE_CONDITION readingCond;
    BOXEDWINE_CONDITION writingCond;
};

bool checkWaitingNativeSockets(int timeout);

#endif