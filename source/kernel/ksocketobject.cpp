/*
 *  Copyright (C) 2012-2025  The BoxedWine Team
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
