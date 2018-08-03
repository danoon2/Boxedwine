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

#include <string.h>
#include <stdio.h>


const char* socketAddressName(U32 address, U32 len, char* result, U32 cbResult) {
    U16 family = readw(address);
    if (family == K_AF_UNIX) {
        return getNativeString(address + 2, result, cbResult);
    } else if (family == K_AF_NETLINK) {
        sprintf(result, "port %u", readd(address + 4));
        return result;
    } else if (family == K_AF_INET) {
        sprintf(result, "AF_INET %u.%u.%u.%u:%u", readb(address + 4), readb(address + 5), readb(address + 6), readb(address + 7), readb(address + 3) | (((U32)readb(address + 2)) << 8));
        return result;
    }
    return "Unknown address family";
}

U32 ksocket(U32 domain, U32 type, U32 protocol) {
    if (domain==K_AF_UNIX || domain==K_AF_NETLINK) {
        KUnixSocketObject* p = new KUnixSocketObject(KThread::currentThread()->process->id, domain, type, protocol);
        BoxedPtr<KSocketObject> kSocket = p;
        KFileDescriptor* result = KThread::currentThread()->process->allocFileDescriptor(kSocket, K_O_RDWR, 0, -1, 0);
        return result->handle;
    } else if (domain == K_AF_INET) {   
        BoxedPtr<KNativeSocketObject> s = new KNativeSocketObject(domain, type, protocol);
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

U32 kbind(U32 socket, U32 address, U32 len) {
    KThread* thread = KThread::currentThread();
    KFileDescriptor* fd = thread->process->getFileDescriptor(socket);

    if (!fd) {
        return -K_EBADF;
    }
    if (IS_NOT_SOCKET(fd)) {
        return -K_ENOTSOCK;
    }
    BoxedPtr<KSocketObject> s = (KSocketObject*)fd->kobject.get();
    return s->bind(fd, address, len);
}

U32 kconnect(U32 socket, U32 address, U32 len) {
    KThread* thread = KThread::currentThread();
    KFileDescriptor* fd = thread->process->getFileDescriptor(socket);

    if (!fd) {
        return -K_EBADF;
    }
    if (IS_NOT_SOCKET(fd)) {
        return -K_ENOTSOCK;
    }
    BoxedPtr<KSocketObject> s = (KSocketObject*)fd->kobject.get();
    if (s->connected) {
        return -K_EISCONN;
    }		
    return s->connect(fd, address, len);        
}

U32 klisten(U32 socket, U32 backlog) {
    KThread* thread = KThread::currentThread();
    KFileDescriptor* fd = thread->process->getFileDescriptor(socket);

    if (!fd) {
        return -K_EBADF;
    }
    if (IS_NOT_SOCKET(fd)) {
        return -K_ENOTSOCK;
    }
    BoxedPtr<KSocketObject> s = (KSocketObject*)fd->kobject.get();
    return s->listen(fd, backlog);
}

U32 kaccept(U32 socket, U32 address, U32 len) {
    KThread* thread = KThread::currentThread();
    KFileDescriptor* fd = thread->process->getFileDescriptor(socket);

    if (!fd) {
        return -K_EBADF;
    }
    if (IS_NOT_SOCKET(fd)) {
        return -K_ENOTSOCK;
    }
    BoxedPtr<KSocketObject> s = (KSocketObject*)fd->kobject.get();
    if (!s->listening) {
        return -K_EINVAL;
    }
    return s->accept(fd, address, len);
}

U32 kgetsockname( U32 socket, U32 address, U32 plen) {
    KThread* thread = KThread::currentThread();
    KFileDescriptor* fd = thread->process->getFileDescriptor(socket);
    
    if (!fd) {
        return -K_EBADF;
    }
    if (IS_NOT_SOCKET(fd)) {
        return -K_ENOTSOCK;
    }
    BoxedPtr<KSocketObject> s = (KSocketObject*)fd->kobject.get();
    return s->getsockname(fd, address, plen);
}

U32 kgetpeername(U32 socket, U32 address, U32 plen) {
    KThread* thread = KThread::currentThread();
    KFileDescriptor* fd = thread->process->getFileDescriptor(socket);
    
    if (!fd) {
        return -K_EBADF;
    }
    if (IS_NOT_SOCKET(fd)) {
        return -K_ENOTSOCK;
    }
    BoxedPtr<KSocketObject> s = (KSocketObject*)fd->kobject.get();
    return s->getpeername(fd, address, plen);
}

U32 ksocketpair(U32 af, U32 type, U32 protocol, U32 socks, U32 flags) {
    FD fd1;
    FD fd2;
    KFileDescriptor* f1;
    KFileDescriptor* f2;
    BoxedPtr<KUnixSocketObject> s1;
    BoxedPtr<KUnixSocketObject> s2;
    KThread* thread = KThread::currentThread();

    if (af!=K_AF_UNIX) {
        kpanic("socketpair with adress family %d not implemented", af);
    }
    if (type!=K_SOCK_DGRAM && type!=K_SOCK_STREAM) {
        kpanic("socketpair with type %d not implemented", type);
    }
    fd1 = ksocket(af, type, protocol);
    fd2 = ksocket(af, type, protocol);
    f1 = thread->process->getFileDescriptor(fd1);
    f2 = thread->process->getFileDescriptor(fd2);
    s1 = (KUnixSocketObject*)f1->kobject.get();
    s2 = (KUnixSocketObject*)f2->kobject.get();
    
    s1->connection = s2.get();
    s2->connection = s1.get();
    s1->connected = true;
    s2->connected = true;
    f1->accessFlags = K_O_RDWR;
    f2->accessFlags = K_O_RDWR;
    writed(socks, fd1);
    writed(socks + 4, fd2);

    if ((flags & K_O_CLOEXEC)!=0) {
        thread->process->fcntrl(fd1, K_F_SETFD, FD_CLOEXEC);
        thread->process->fcntrl(fd2, K_F_SETFD, FD_CLOEXEC);
    }
    if ((flags & K_O_NONBLOCK)!=0) {
        thread->process->fcntrl(fd1, K_F_SETFL, K_O_NONBLOCK);
        thread->process->fcntrl(fd2, K_F_SETFL, K_O_NONBLOCK);
    }
    if (flags & ~(K_O_CLOEXEC|K_O_NONBLOCK)) {
        kwarn("Unknow flags sent to pipe2: %X", flags);
    }
    return 0;
}

U32 ksend(U32 socket, U32 buffer, U32 len, U32 flags) {
    KThread* thread = KThread::currentThread();
    KFileDescriptor* fd = thread->process->getFileDescriptor(socket);

    if (!fd) {
        return -K_EBADF;
    }
    if (IS_NOT_SOCKET(fd)) {
        return -K_ENOTSOCK;
    }
    BoxedPtr<KSocketObject> s = (KSocketObject*)fd->kobject.get();
    s->flags = 0;
    if (flags == 0x4000) {
        //  MSG_NOSIGNAL
    }
    if (flags & 1) {
        s->flags|=K_MSG_OOB;
    } 
    U32 result = thread->process->write(socket, buffer, len);
    s->flags = 0;
    return result;
}

U32 krecv(U32 socket, U32 buffer, U32 len, U32 flags) {
    KThread* thread = KThread::currentThread();
    KFileDescriptor* fd = thread->process->getFileDescriptor(socket);

    if (!fd) {
        return -K_EBADF;
    }
    if (IS_NOT_SOCKET(fd)) {
        return -K_ENOTSOCK;
    }
    BoxedPtr<KSocketObject> s = (KSocketObject*)fd->kobject.get();
    s->flags = 0;
    if (flags == 0x4000) {
        //  MSG_NOSIGNAL
    } 
    if (flags & 1) {
        s->flags|=K_MSG_OOB;
    } 
    if (flags & 2) {
        s->flags|=K_MSG_PEEK;
    } 
    U32 result = thread->process->read(socket, buffer, len);
    s->flags = 0;
    return result;
}

U32 kshutdown(U32 socket, U32 how) {
    KThread* thread = KThread::currentThread();
    KFileDescriptor* fd = thread->process->getFileDescriptor(socket);

    if (!fd) {
        return -K_EBADF;
    }
    if (IS_NOT_SOCKET(fd)) {
        return -K_ENOTSOCK;
    }
    BoxedPtr<KSocketObject> s = (KSocketObject*)fd->kobject.get();
    return s->shutdown(fd, how);
}

U32 ksetsockopt(U32 socket, U32 level, U32 name, U32 value, U32 len) {
    KThread* thread = KThread::currentThread();
    KFileDescriptor* fd = thread->process->getFileDescriptor(socket);

    if (!fd) {
        return -K_EBADF;
    }
    if (IS_NOT_SOCKET(fd)) {
        return -K_ENOTSOCK;
    }
    BoxedPtr<KSocketObject> s = (KSocketObject*)fd->kobject.get();
    return s->setsockopt(fd, level, name, value, len);
}

U32 kgetsockopt( U32 socket, U32 level, U32 name, U32 value, U32 len_address) {
    KThread* thread = KThread::currentThread();
    KFileDescriptor* fd = thread->process->getFileDescriptor(socket);

    if (!fd) {
        return -K_EBADF;
    }
    if (IS_NOT_SOCKET(fd)) {
        return -K_ENOTSOCK;
    }
    BoxedPtr<KSocketObject> s = (KSocketObject*)fd->kobject.get();
    return s->getsockopt(fd, level, name, value, len_address);    
}

#define K_SOL_SOCKET 1
#define K_SCM_RIGHTS 1

U32 ksendmmsg(U32 socket, U32 address, U32 vlen, U32 flags) {
    U32 i;

    for (i=0;i<vlen;i++) {
        S32 result = (S32)ksendmsg(socket, address+i*32, flags);
        if (result>=0) {
            writed(address+i*32+28, result);
        } else {
            return i;
        }
    }
    return vlen;
}

U32 ksendmsg(U32 socket, U32 address, U32 flags) {
    KThread* thread = KThread::currentThread();
    KFileDescriptor* fd = thread->process->getFileDescriptor(socket);

    if (!fd) {
        return -K_EBADF;
    }
    if (IS_NOT_SOCKET(fd)) {
        return -K_ENOTSOCK;
    }
    BoxedPtr<KSocketObject> s = (KSocketObject*)fd->kobject.get();
    return s->sendmsg(fd, address, flags);    
}

U32 krecvmsg(U32 socket, U32 address, U32 flags) {
    KThread* thread = KThread::currentThread();
    KFileDescriptor* fd = thread->process->getFileDescriptor(socket);

    if (!fd) {
        return -K_EBADF;
    }
    if (IS_NOT_SOCKET(fd)) {
        return -K_ENOTSOCK;
    }
    BoxedPtr<KSocketObject> s = (KSocketObject*)fd->kobject.get();
    return s->recvmsg(fd, address, flags);
}

// ssize_t sendto(int socket, const void *message, size_t length, int flags, const struct sockaddr *dest_addr, socklen_t dest_len);
U32 ksendto(U32 socket, U32 message, U32 length, U32 flags, U32 dest_addr, U32 dest_len) {
    KThread* thread = KThread::currentThread();
    KFileDescriptor* fd = thread->process->getFileDescriptor(socket);

    if (!fd) {
        return -K_EBADF;
    }
    if (IS_NOT_SOCKET(fd)) {
        return -K_ENOTSOCK;
    }
    BoxedPtr<KSocketObject> s = (KSocketObject*)fd->kobject.get();
    return s->sendto(fd, message, length, flags, dest_addr, dest_len);        
}

// ssize_t recvfrom(int socket, void *restrict buffer, size_t length, int flags, struct sockaddr *restrict address, socklen_t *restrict address_len);
U32 krecvfrom(U32 socket, U32 buffer, U32 length, U32 flags, U32 address, U32 address_len) {
    KThread* thread = KThread::currentThread();
    KFileDescriptor* fd = thread->process->getFileDescriptor(socket);

    if (!fd) {
        return -K_EBADF;
    }
    if (IS_NOT_SOCKET(fd)) {
        return -K_ENOTSOCK;
    }
    BoxedPtr<KSocketObject> s = (KSocketObject*)fd->kobject.get();
    return s->recvfrom(fd, buffer, length, flags, address, address_len);    
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
