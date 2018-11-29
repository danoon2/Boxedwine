#include "boxedwine.h"

#include "ksocketobject.h"
#include "kscheduler.h"

KSocketObject::KSocketObject(U32 objectType, U32 domain, U32 type, U32 protocol) : 
        KObject(objectType), 
        domain(domain),
        type(type),
        protocol(protocol),
        lastModifiedTime(0),
        blocking(true), 
        listening(false),
        nl_port(0),
        connected(false), 
        recvLen(1048576), 
        sendLen(1048576), 
        inClosed(false), 
        outClosed(false),
        flags(0),
        error(0)
{
}    

void KSocketObject::readMsgHdr(U32 address, MsgHdr* hdr) {
    hdr->msg_name = readd(address);address+=4;
    hdr->msg_namelen = readd(address); address += 4;
    hdr->msg_iov = readd(address); address += 4;
    hdr->msg_iovlen = readd(address); address += 4;
    hdr->msg_control = readd(address); address += 4;
    hdr->msg_controllen = readd(address); address += 4;
    hdr->msg_flags = readd(address);
}

void KSocketObject::readCMsgHdr(U32 address, CMsgHdr* hdr) {
    hdr->cmsg_len = readd(address); address += 4;
    hdr->cmsg_level = readd(address); address += 4;
    hdr->cmsg_type = readd(address);
}

void KSocketObject::writeCMsgHdr(U32 address, U32 len, U32 level, U32 type) {
    writed(address, len); address += 4;
    writed(address, level); address += 4;
    writed(address, type);
}
