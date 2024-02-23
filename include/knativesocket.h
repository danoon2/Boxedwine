#ifndef __KNATIVESOCKET_H__
#define __KNATIVESOCKET_H__

#include "ksocketobject.h"

class KNativeSocketObject : public KSocketObject {
public:
    KNativeSocketObject(U32 domain, U32 type, U32 protocol);

    virtual ~KNativeSocketObject();    

    // from KObject
    U32 ioctl(KThread* thread, U32 request) override;
    S64 seek(S64 pos) override;
    S64 length() override;
    S64 getPos() override;
    void setBlocking(bool blocking) override;
    bool isBlocking() override;
    void setAsync(bool isAsync) override;
    bool isAsync() override;
    KFileLock* getLock(KFileLock* lock) override;
    U32 setLock(KFileLock* lock, bool wait) override;
    bool supportsLocks() override;
    bool isOpen() override;
    bool isReadReady() override;
    bool isPriorityReadReady() override;
    bool isWriteReady() override;
    void waitForEvents(BOXEDWINE_CONDITION& parentCondition, U32 events) override;
    U32 writeNative(U8* buffer, U32 len) override;
    U32 readNative(U8* buffer, U32 len) override;
    U32 stat(KProcess* process, U32 address, bool is64) override;
    U32 map(KThread* thread, U32 address, U32 len, S32 prot, S32 flags, U64 off) override;
    bool canMap() override;
    BString selfFd() override;

    // from KSocketObject
    U32 accept(KThread* thread, KFileDescriptor* fd, U32 address, U32 len, U32 flags) override;
    U32 bind(KThread* thread, KFileDescriptor* fd, U32 address, U32 len) override;
    U32 connect(KThread* thread, KFileDescriptor* fd, U32 address, U32 len) override;
    U32 getpeername(KThread* thread, KFileDescriptor* fd, U32 address, U32 plen) override;
    U32 getsockname(KThread* thread, KFileDescriptor* fd, U32 address, U32 plen) override;
    U32 getsockopt(KThread* thread, KFileDescriptor* fd, U32 level, U32 name, U32 value, U32 len_address) override;
    U32 listen(KThread* thread, KFileDescriptor* fd, U32 backlog) override;
    U32 recvfrom(KThread* thread, KFileDescriptor* fd, U32 buffer, U32 length, U32 flags, U32 address, U32 address_len) override;
    U32 recvmsg(KThread* thread, KFileDescriptor* fd, U32 address, U32 flags) override;
    U32 sendmsg(KThread* thread, KFileDescriptor* fd, U32 address, U32 flags) override;
    U32 sendto(KThread* thread, KFileDescriptor* fd, U32 message, U32 length, U32 flags, U32 dest_addr, U32 dest_len) override;
    U32 setsockopt(KThread* thread, KFileDescriptor* fd, U32 level, U32 name, U32 value, U32 len) override;
    U32 shutdown(KThread* thread, KFileDescriptor* fd, U32 how) override;

    S32 nativeSocket;
    bool connecting;

    BOXEDWINE_CONDITION readingCond;
    BOXEDWINE_CONDITION writingCond;
};

bool checkWaitingNativeSockets(int timeout);
FsOpenNode* openHosts(const std::shared_ptr<FsNode>& node, U32 flags, U32 data);
FsOpenNode* openHostname(const std::shared_ptr<FsNode>& node, U32 flags, U32 data);

#endif