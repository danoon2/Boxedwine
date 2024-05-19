#include "boxedwine.h"

#include "ksocketobject.h"
#include "kscheduler.h"

void KSocketObject::readMsgHdr(KThread* thread, U32 address, MsgHdr* hdr) {
    KMemory* memory = thread->memory;

    hdr->msg_name = memory->readd(address);address+=4;
    hdr->msg_namelen = memory->readd(address); address += 4;
    hdr->msg_iov = memory->readd(address); address += 4;
    hdr->msg_iovlen = memory->readd(address); address += 4;
    hdr->msg_control = memory->readd(address); address += 4;
    hdr->msg_controllen = memory->readd(address); address += 4;
    hdr->msg_flags = memory->readd(address);
}

void KSocketObject::readCMsgHdr(KThread* thread, U32 address, CMsgHdr* hdr) {
    KMemory* memory = thread->memory;

    hdr->cmsg_len = memory->readd(address); address += 4;
    hdr->cmsg_level = memory->readd(address); address += 4;
    hdr->cmsg_type = memory->readd(address);
}

void KSocketObject::writeCMsgHdr(KThread* thread, U32 address, U32 len, U32 level, U32 type) {
    KMemory* memory = thread->memory;

    memory->writed(address, len); address += 4;
    memory->writed(address, level); address += 4;
    memory->writed(address, type);
}
