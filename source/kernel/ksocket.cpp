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

#include "boxedwine.h"

#include "ksocket.h"
#include "kstat.h"
#include "kscheduler.h"
#include "kunixsocket.h"
#include "knativesocket.h"
#include "knetlink.h"

#include <string.h>
#include <stdio.h>


BString socketAddressName(KMemory* memory, U32 address, U32 len) {
    if (!address) {
        return B("");
    }
    U16 family = memory->readw(address);
    if (family == K_AF_UNIX) {
        U32 sLen = (U32)memory->strlen(address + 2);
        BString result(sLen + 1, 0);
        memory->memcpy(result.str(), address + 2, sLen+1);
        return result;
    } else if (family == K_AF_NETLINK) {
        BString result;
        result.sprintf("port %u", memory->readd(address + 4));
        return result;
    } else if (family == K_AF_INET) {
        BString result;
        result.sprintf("AF_INET %u.%u.%u.%u:%u", memory->readb(address + 4), memory->readb(address + 5), memory->readb(address + 6), memory->readb(address + 7), memory->readb(address + 3) | (((U32)memory->readb(address + 2)) << 8));
        return result;
    }
    return B("Unknown address family");
}

U32 ksocket(U32 domain, U32 type, U32 protocol) {
    if (domain==K_AF_UNIX) {
        std::shared_ptr<KUnixSocketObject> kSocket = std::make_shared<KUnixSocketObject>(domain, type, protocol);
        KFileDescriptor* result = KThread::currentThread()->process->allocFileDescriptor(kSocket, K_O_RDWR, 0, -1, 0);
        return result->handle;
    } else if (domain == K_AF_NETLINK) {       
        std::shared_ptr<KNetLinkObject> kSocket = std::make_shared<KNetLinkObject>(domain, type, protocol);
        KFileDescriptor* result = KThread::currentThread()->process->allocFileDescriptor(kSocket, K_O_RDWR, 0, -1, 0);
        return result->handle;
    } else if (domain == K_AF_INET) {
        std::shared_ptr<KNativeSocketObject> s = std::make_shared<KNativeSocketObject>(domain, type, protocol);
        if (s->error) {
            return s->error;
        } else {
            KFileDescriptor* result = KThread::currentThread()->process->allocFileDescriptor(s, K_O_RDWR, 0, -1, 0);
            return result->handle;
        }
    }
    return -K_EAFNOSUPPORT;
}

#define IS_NOT_SOCKET(fd) (fd->kobject->type!=KTYPE_NATIVE_SOCKET && fd->kobject->type!=KTYPE_UNIX_SOCKET)

U32 kbind(KThread* thread, U32 socket, U32 address, U32 len) {
    KFileDescriptor* fd = thread->process->getFileDescriptor(socket);

    if (!fd) {
        return -K_EBADF;
    }
    if (IS_NOT_SOCKET(fd)) {
        return -K_ENOTSOCK;
    }
    std::shared_ptr<KSocketObject> s = std::dynamic_pointer_cast<KSocketObject>(fd->kobject);
    return s->bind(thread, fd, address, len);
}

U32 kconnect(KThread* thread, U32 socket, U32 address, U32 len) {
    KFileDescriptor* fd = thread->process->getFileDescriptor(socket);

    if (!fd) {
        return -K_EBADF;
    }
    if (IS_NOT_SOCKET(fd)) {
        return -K_ENOTSOCK;
    }
    std::shared_ptr<KSocketObject> s = std::dynamic_pointer_cast<KSocketObject>(fd->kobject);
    if (s->connected) {
        return -K_EISCONN;
    }		
    return s->connect(thread, fd, address, len);        
}

U32 klisten(KThread* thread, U32 socket, U32 backlog) {
    KFileDescriptor* fd = thread->process->getFileDescriptor(socket);

    if (!fd) {
        return -K_EBADF;
    }
    if (IS_NOT_SOCKET(fd)) {
        return -K_ENOTSOCK;
    }
    std::shared_ptr<KSocketObject> s = std::dynamic_pointer_cast<KSocketObject>(fd->kobject);
    return s->listen(thread, fd, backlog);
}

U32 kaccept(KThread* thread, U32 socket, U32 address, U32 len, U32 flags) {
    KFileDescriptor* fd = thread->process->getFileDescriptor(socket);

    if (!fd) {
        return -K_EBADF;
    }
    if (IS_NOT_SOCKET(fd)) {
        return -K_ENOTSOCK;
    }
    std::shared_ptr<KSocketObject> s = std::dynamic_pointer_cast<KSocketObject>(fd->kobject);
    if (!s->listening) {
        return -K_EINVAL;
    }
    return s->accept(thread, fd, address, len, flags);
}

U32 kgetsockname(KThread* thread, U32 socket, U32 address, U32 plen) {
    KFileDescriptor* fd = thread->process->getFileDescriptor(socket);
    
    if (!fd) {
        return -K_EBADF;
    }
    if (IS_NOT_SOCKET(fd)) {
        return -K_ENOTSOCK;
    }
    std::shared_ptr<KSocketObject> s = std::dynamic_pointer_cast<KSocketObject>(fd->kobject);
    return s->getsockname(thread, fd, address, plen);
}

U32 kgetpeername(KThread* thread, U32 socket, U32 address, U32 plen) {
    KFileDescriptor* fd = thread->process->getFileDescriptor(socket);
    
    if (!fd) {
        return -K_EBADF;
    }
    if (IS_NOT_SOCKET(fd)) {
        return -K_ENOTSOCK;
    }
    std::shared_ptr<KSocketObject> s = std::dynamic_pointer_cast<KSocketObject>(fd->kobject);
    return s->getpeername(thread, fd, address, plen);
}

U32 ksocketpair(KThread* thread, U32 af, U32 type, U32 protocol, U32 socks, U32 flags) {
    FD fd1 = 0;
    FD fd2 = 0;
    KFileDescriptor* f1 = nullptr;
    KFileDescriptor* f2 = nullptr;
    std::shared_ptr<KUnixSocketObject> s1;
    std::shared_ptr<KUnixSocketObject> s2;
    KMemory* memory = thread->memory;

    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(thread->process->fdsMutex);

    if (af!=K_AF_UNIX) {
        kwarn("socketpair with adress family %d not implemented", af);
        return -1;
    }
    if (type!=K_SOCK_DGRAM && type!=K_SOCK_STREAM) {
        kwarn("socketpair with type %d not implemented", type);
        return -1;
    }
    fd1 = ksocket(af, type, protocol);
    fd2 = ksocket(af, type, protocol);
    f1 = thread->process->getFileDescriptor(fd1);
    f2 = thread->process->getFileDescriptor(fd2);
    s1 = std::dynamic_pointer_cast<KUnixSocketObject>(f1->kobject);
    s2 = std::dynamic_pointer_cast<KUnixSocketObject>(f2->kobject);
    
    s1->connection = s2;
    s2->connection = s1;
    s1->connected = true;
    s2->connected = true;
    f1->accessFlags = K_O_RDWR;
    f2->accessFlags = K_O_RDWR;
    memory->writed(socks, fd1);
    memory->writed(socks + 4, fd2);

    if ((flags & K_O_CLOEXEC)!=0) {
        thread->process->fcntrl(thread, fd1, K_F_SETFD, FD_CLOEXEC);
        thread->process->fcntrl(thread, fd2, K_F_SETFD, FD_CLOEXEC);
    }
    if ((flags & K_O_NONBLOCK)!=0) {
        thread->process->fcntrl(thread, fd1, K_F_SETFL, K_O_NONBLOCK);
        thread->process->fcntrl(thread, fd2, K_F_SETFL, K_O_NONBLOCK);
    }
    if (flags & ~(K_O_CLOEXEC|K_O_NONBLOCK)) {
        kwarn("Unknow flags sent to pipe2: %X", flags);
    }
    return 0;
}

U32 ksend(KThread* thread, U32 socket, U32 buffer, U32 len, U32 flags) {
    KFileDescriptor* fd = thread->process->getFileDescriptor(socket);

    if (!fd) {
        return -K_EBADF;
    }
    if (IS_NOT_SOCKET(fd)) {
        return -K_ENOTSOCK;
    }
    std::shared_ptr<KSocketObject> s = std::dynamic_pointer_cast<KSocketObject>(fd->kobject);
    return s->sendto(thread, fd, buffer, len, flags, 0, 0);
}

U32 krecv(KThread* thread, U32 socket, U32 buffer, U32 len, U32 flags) {
    KFileDescriptor* fd = thread->process->getFileDescriptor(socket);

    if (!fd) {
        return -K_EBADF;
    }
    if (IS_NOT_SOCKET(fd)) {
        return -K_ENOTSOCK;
    }
    std::shared_ptr<KSocketObject> s = std::dynamic_pointer_cast<KSocketObject>(fd->kobject);
    return s->recvfrom(thread, fd, buffer, len, flags, 0, 0);
}

U32 kshutdown(KThread* thread, U32 socket, U32 how) {
    KFileDescriptor* fd = thread->process->getFileDescriptor(socket);

    if (!fd) {
        return -K_EBADF;
    }
    if (IS_NOT_SOCKET(fd)) {
        return -K_ENOTSOCK;
    }
    std::shared_ptr<KSocketObject> s = std::dynamic_pointer_cast<KSocketObject>(fd->kobject);
    return s->shutdown(thread, fd, how);
}

U32 ksetsockopt(KThread* thread, U32 socket, U32 level, U32 name, U32 value, U32 len) {
    KFileDescriptor* fd = thread->process->getFileDescriptor(socket);

    if (!fd) {
        return -K_EBADF;
    }
    if (IS_NOT_SOCKET(fd)) {
        return -K_ENOTSOCK;
    }
    std::shared_ptr<KSocketObject> s = std::dynamic_pointer_cast<KSocketObject>(fd->kobject);
    return s->setsockopt(thread, fd, level, name, value, len);
}

U32 kgetsockopt(KThread* thread, U32 socket, U32 level, U32 name, U32 value, U32 len_address) {
    KFileDescriptor* fd = thread->process->getFileDescriptor(socket);

    if (!fd) {
        return -K_EBADF;
    }
    if (IS_NOT_SOCKET(fd)) {
        return -K_ENOTSOCK;
    }
    std::shared_ptr<KSocketObject> s = std::dynamic_pointer_cast<KSocketObject>(fd->kobject);
    return s->getsockopt(thread, fd, level, name, value, len_address);    
}

#define K_SOL_SOCKET 1
#define K_SCM_RIGHTS 1

U32 ksendmmsg(KThread* thread, U32 socket, U32 address, U32 vlen, U32 flags) {
    U32 i;

    for (i=0;i<vlen;i++) {
        S32 result = (S32)ksendmsg(thread, socket, address+i*32, flags);
        if (result>=0) {
            thread->memory->writed(address+i*32+28, result);
        } else {
            return i;
        }
    }
    return vlen;
}

U32 ksendmsg(KThread* thread, U32 socket, U32 address, U32 flags) {
    KFileDescriptor* fd = thread->process->getFileDescriptor(socket);

    if (!fd) {
        return -K_EBADF;
    }
    if (IS_NOT_SOCKET(fd)) {
        return -K_ENOTSOCK;
    }
    std::shared_ptr<KSocketObject> s = std::dynamic_pointer_cast<KSocketObject>(fd->kobject);
    return s->sendmsg(thread, fd, address, flags);    
}

U32 krecvmsg(KThread* thread, U32 socket, U32 address, U32 flags) {
    KFileDescriptor* fd = thread->process->getFileDescriptor(socket);

    if (!fd) {
        return -K_EBADF;
    }
    if (IS_NOT_SOCKET(fd)) {
        return -K_ENOTSOCK;
    }
    std::shared_ptr<KSocketObject> s = std::dynamic_pointer_cast<KSocketObject>(fd->kobject);
    return s->recvmsg(thread, fd, address, flags);
}

// ssize_t sendto(int socket, const void *message, size_t length, int flags, const struct sockaddr *dest_addr, socklen_t dest_len);
U32 ksendto(KThread* thread, U32 socket, U32 message, U32 length, U32 flags, U32 dest_addr, U32 dest_len) {
    KFileDescriptor* fd = thread->process->getFileDescriptor(socket);

    if (!fd) {
        return -K_EBADF;
    }
    if (IS_NOT_SOCKET(fd)) {
        return -K_ENOTSOCK;
    }
    std::shared_ptr<KSocketObject> s = std::dynamic_pointer_cast<KSocketObject>(fd->kobject);
    return s->sendto(thread, fd, message, length, flags, dest_addr, dest_len);        
}

// ssize_t recvfrom(int socket, void *restrict buffer, size_t length, int flags, struct sockaddr *restrict address, socklen_t *restrict address_len);
U32 krecvfrom(KThread* thread, U32 socket, U32 buffer, U32 length, U32 flags, U32 address, U32 address_len) {
    KFileDescriptor* fd = thread->process->getFileDescriptor(socket);

    if (!fd) {
        return -K_EBADF;
    }
    if (IS_NOT_SOCKET(fd)) {
        return -K_ENOTSOCK;
    }
    std::shared_ptr<KSocketObject> s = std::dynamic_pointer_cast<KSocketObject>(fd->kobject);
    return s->recvfrom(thread, fd, buffer, length, flags, address, address_len);    
}

bool isNativeSocket(KThread* thread, FD desc) {
    KFileDescriptor* fd = thread->process->getFileDescriptor(desc);

    if (!fd) {
        return false;
    }
    if (fd->kobject->type == KTYPE_NATIVE_SOCKET) {
        return true;
    }
    return false;
}
