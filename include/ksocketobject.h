#ifndef __KSOCKETOBJECT_H__
#define __KSOCKETOBJECT_H__

class KFileDescriptor;

class KSocketObject : public KObject {
public:
    class KSockAddress {
    public:
        U16 family;
        char data[256];
    };    

    KSocketObject(U32 objectType, U32 domain, U32 type, U32 protocol);

    virtual U32 accept(KFileDescriptor* fd, U32 address, U32 len, U32 flags) = 0;
    virtual U32 bind(KFileDescriptor* fd, U32 address, U32 len) = 0;
    virtual U32 connect(KFileDescriptor* fd, U32 address, U32 len) = 0;
    virtual U32 getpeername(KFileDescriptor* fd, U32 address, U32 plen) = 0;
    virtual U32 getsockname(KFileDescriptor* fd, U32 address, U32 plen) = 0;
    virtual U32 getsockopt(KFileDescriptor* fd, U32 level, U32 name, U32 value, U32 len_address) = 0;
    virtual U32 listen(KFileDescriptor* fd, U32 backlog) = 0;
    virtual U32 recvfrom(KFileDescriptor* fd, U32 buffer, U32 length, U32 flags, U32 address, U32 address_len) = 0;
    virtual U32 recvmsg(KFileDescriptor* fd, U32 address, U32 flags) = 0;
    virtual U32 sendmsg(KFileDescriptor* fd, U32 address, U32 flags) = 0;
    virtual U32 sendto(KFileDescriptor* fd, U32 message, U32 length, U32 flags, U32 dest_addr, U32 dest_len) = 0;
    virtual U32 setsockopt(KFileDescriptor* fd, U32 level, U32 name, U32 value, U32 len) = 0;
    virtual U32 shutdown(KFileDescriptor* fd, U32 how) = 0;

    U32 domain;
    U32 type;
    U32 protocol;    
    U64 lastModifiedTime;
    bool blocking;
    bool listening;
    U32 nl_port;    
    
    bool connected; // this will be 0 and connection will be set while connect is blocking
    KSockAddress destAddress;
    U32 recvLen;
    U32 sendLen;

    bool inClosed;
    bool outClosed;

    U32 flags;
    int error;

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

    void readMsgHdr(U32 address, MsgHdr* hdr);
    void readCMsgHdr(U32 address, CMsgHdr* hdr);
    void writeCMsgHdr(U32 address, U32 len, U32 level, U32 type);
};

#endif