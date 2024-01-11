#ifndef __KNATIVESOCKET_H__
#define __KNATIVESOCKET_H__

#include "ksocketobject.h"

class KNativeSocketObject : public KSocketObject {
public:
    KNativeSocketObject(U32 domain, U32 type, U32 protocol);

    virtual ~KNativeSocketObject();    

    // from KObject
    virtual U32 ioctl(KThread* thread, U32 request) override;
    virtual S64 seek(S64 pos) override;
    virtual S64 length() override;
    virtual S64 getPos() override;
    virtual void setBlocking(bool blocking) override;
    virtual bool isBlocking() override;
    virtual void setAsync(bool isAsync) override;
    virtual bool isAsync() override;
    virtual KFileLock* getLock(KFileLock* lock) override;
    virtual U32 setLock(KFileLock* lock, bool wait) override;
    virtual bool supportsLocks() override;
    virtual bool isOpen() override;
    virtual bool isReadReady() override;
    virtual bool isPriorityReadReady() override;
    virtual bool isWriteReady() override;
    virtual void waitForEvents(BOXEDWINE_CONDITION& parentCondition, U32 events) override;
    virtual U32 writeNative(U8* buffer, U32 len) override;
    virtual U32 readNative(U8* buffer, U32 len) override;
    virtual U32 stat(KProcess* process, U32 address, bool is64) override;
    virtual U32 map(KThread* thread, U32 address, U32 len, S32 prot, S32 flags, U64 off) override;
    virtual bool canMap() override;

    // from KSocketObject
    virtual U32 accept(KThread* thread, KFileDescriptor* fd, U32 address, U32 len, U32 flags) override;
    virtual U32 bind(KThread* thread, KFileDescriptor* fd, U32 address, U32 len) override;
    virtual U32 connect(KThread* thread, KFileDescriptor* fd, U32 address, U32 len) override;
    virtual U32 getpeername(KThread* thread, KFileDescriptor* fd, U32 address, U32 plen) override;
    virtual U32 getsockname(KThread* thread, KFileDescriptor* fd, U32 address, U32 plen) override;
    virtual U32 getsockopt(KThread* thread, KFileDescriptor* fd, U32 level, U32 name, U32 value, U32 len_address) override;
    virtual U32 listen(KThread* thread, KFileDescriptor* fd, U32 backlog) override;
    virtual U32 recvfrom(KThread* thread, KFileDescriptor* fd, U32 buffer, U32 length, U32 flags, U32 address, U32 address_len) override;
    virtual U32 recvmsg(KThread* thread, KFileDescriptor* fd, U32 address, U32 flags) override;
    virtual U32 sendmsg(KThread* thread, KFileDescriptor* fd, U32 address, U32 flags) override;
    virtual U32 sendto(KThread* thread, KFileDescriptor* fd, U32 message, U32 length, U32 flags, U32 dest_addr, U32 dest_len) override;
    virtual U32 setsockopt(KThread* thread, KFileDescriptor* fd, U32 level, U32 name, U32 value, U32 len) override;
    virtual U32 shutdown(KThread* thread, KFileDescriptor* fd, U32 how) override;

    S32 nativeSocket;
    bool connecting;

    BOXEDWINE_CONDITION readingCond;
    BOXEDWINE_CONDITION writingCond;
};

bool checkWaitingNativeSockets(int timeout);
FsOpenNode* openHosts(const BoxedPtr<FsNode>& node, U32 flags, U32 data);
FsOpenNode* openHostname(const BoxedPtr<FsNode>& node, U32 flags, U32 data);

#endif