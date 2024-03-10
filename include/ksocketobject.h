#ifndef __KSOCKETOBJECT_H__
#define __KSOCKETOBJECT_H__

class KFileDescriptor;

class KSocketObject : public KObject {
public:
    class KSockAddress {
    public:
        KSockAddress() = default;
        U16 family = 0;
        char data[256] = { 0 };
    };    

    KSocketObject(U32 objectType, U32 domain, U32 type, U32 protocol) : KObject(objectType), domain(domain), type(type), protocol(protocol) {}

    virtual U32 accept(KThread* thread, KFileDescriptor* fd, U32 address, U32 len, U32 flags) = 0;
    virtual U32 bind(KThread* thread, KFileDescriptor* fd, U32 address, U32 len) = 0;
    virtual U32 connect(KThread* thread, KFileDescriptor* fd, U32 address, U32 len) = 0;
    virtual U32 getpeername(KThread* thread, KFileDescriptor* fd, U32 address, U32 plen) = 0;
    virtual U32 getsockname(KThread* thread, KFileDescriptor* fd, U32 address, U32 plen) = 0;
    virtual U32 getsockopt(KThread* thread, KFileDescriptor* fd, U32 level, U32 name, U32 value, U32 len_address) = 0;
    virtual U32 listen(KThread* thread, KFileDescriptor* fd, U32 backlog) = 0;
    virtual U32 recvfrom(KThread* thread, KFileDescriptor* fd, U32 buffer, U32 length, U32 flags, U32 address, U32 address_len) = 0;
    virtual U32 recvmsg(KThread* thread, KFileDescriptor* fd, U32 address, U32 flags) = 0;
    virtual U32 sendmsg(KThread* thread, KFileDescriptor* fd, U32 address, U32 flags) = 0;
    virtual U32 sendto(KThread* thread, KFileDescriptor* fd, U32 message, U32 length, U32 flags, U32 dest_addr, U32 dest_len) = 0;
    virtual U32 setsockopt(KThread* thread, KFileDescriptor* fd, U32 level, U32 name, U32 value, U32 len) = 0;
    virtual U32 shutdown(KThread* thread, KFileDescriptor* fd, U32 how) = 0;

    U32 domain = 0;
    U32 type = 0;
    U32 protocol = 0;    
    U64 lastModifiedTime = 0;
    bool blocking = true;
    bool listening = false;
    
    bool connected = false; // this will be 0 and connection will be set while connect is blocking
    KSockAddress destAddress;
    U32 recvLen = 1048576;
    U32 sendLen = 1048576;

    bool inClosed = false;
    bool outClosed = false;

    int error = 0;

protected:
    class MsgHdr {
    public:
        U32 msg_name;
        U32 msg_namelen;
        U32 msg_iov;
        U32 msg_iovlen;
        U32 msg_control;
        U32 msg_controllen;
        U32 msg_flags;
    };

    class CMsgHdr {
    public:
        U32 cmsg_len;
        U32 cmsg_level;
        U32 cmsg_type;
    };

    void readMsgHdr(KThread* thread, U32 address, MsgHdr* hdr);
    void readCMsgHdr(KThread* thread, U32 address, CMsgHdr* hdr);
    void writeCMsgHdr(KThread* thread, U32 address, U32 len, U32 level, U32 type);
};

#endif