#include "boxedwine.h"

#include "ksocketobject.h"
#include "kscheduler.h"

KSocketObject::KSocketObject(U32 objectType, U32 domain, U32 type, U32 protocol) : 
        KObject(objectType), 
        domain(domain),
        type(type),
        protocol(protocol),
        blocking(true), 
        listening(false), 
        recvLen(1048576), 
        sendLen(1048576), 
        inClosed(false), 
        outClosed(false),
        lastModifiedTime(0),
        nl_port(0),
        connected(false),
        flags(0),
        error(0)
{
}    

void KSocketObject::waitOnSocketRead(KThread* thread) {
    this->waitingOnReadThread.addToBack(thread->getWaitNofiyNode());
}

void KSocketObject::waitOnSocketWrite(KThread* thread) {
    this->waitingOnWriteThread.addToBack(thread->getWaitNofiyNode());
}

void KSocketObject::waitOnSocketConnect(KThread* thread) {
    this->waitingOnConnectThread.addToBack(thread->getWaitNofiyNode());
}

U32 KSocketObject::wakeAndResetWaitingOnReadThreads() {
    U32 result = this->waitingOnReadThread.size();
    if (result) {
        this->waitingOnReadThread.for_each([](KListNode<KThread*>* node) {
            wakeThread(node->data);
        }); 
    }
    return result;
}

U32 KSocketObject::wakeAndResetWaitingOnWriteThreads() {
    U32 result = this->waitingOnWriteThread.size();

    if (result) {
        this->waitingOnWriteThread.for_each([](KListNode<KThread*>* node) {
            wakeThread(node->data);
        });
    }

    return result;
}

U32 KSocketObject::wakeAndResetWaitingOnConnectionThreads() {
    U32 result = this->waitingOnConnectThread.size();

    if (result) {
        this->waitingOnConnectThread.for_each([](KListNode<KThread*>* node) {
            wakeThread(node->data);
        });
    }

    return result;
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