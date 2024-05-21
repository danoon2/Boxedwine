#include "boxedwine.h"

#include "knetlink.h"
#include "ksocket.h"
#include "kstat.h"
#include "ksignal.h"

KNetLinkObject::KNetLinkObject(U32 domain, U32 type, U32 protocol) : KSocketObject(KTYPE_UNIX_SOCKET, domain, type, protocol),
lockCond(std::make_shared<BoxedWineCondition>(B("KNetLinkObject::lockCond")))
{
}

KNetLinkObject::~KNetLinkObject() {
    BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(this->lockCond);
    BOXEDWINE_CONDITION_SIGNAL_ALL(this->lockCond);
}

void KNetLinkObject::setBlocking(bool blocking) {
    BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(this->lockCond);
    this->blocking = blocking;
}

bool KNetLinkObject::isBlocking() {
    return this->blocking;
}

void KNetLinkObject::setAsync(bool isAsync) {
    if (isAsync)
        kpanic("KNetLinkObject::setAsync not implemented yet");
}

bool KNetLinkObject::isAsync() {
    return false;
}

KFileLock* KNetLinkObject::getLock(KFileLock* lock) {
    kdebug("KNetLinkObject::getLock not implemented yet");
    return nullptr;
}

U32 KNetLinkObject::setLock(KFileLock* lock, bool wait) {
    kdebug(" UnixSocketObject::setLock not implemented yet");
    return -1;
}

bool KNetLinkObject::isOpen() {
    return this->listening || this->afBound;
}

bool KNetLinkObject::isReadReady() {
    //BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(this->lockCond);
    return this->inClosed || this->recvBuffer.size();
}

bool KNetLinkObject::isWriteReady() {
    return true;
}

void KNetLinkObject::waitForEvents(BOXEDWINE_CONDITION& parentCondition, U32 events) {
    bool addedLock = false;

    if (events & K_POLLIN) {
        BOXEDWINE_CONDITION_ADD_PARENT(this->lockCond, parentCondition);
        addedLock = true;
    }
    if (events & K_POLLOUT) {
        if (!addedLock) {
            BOXEDWINE_CONDITION_ADD_PARENT(this->lockCond, parentCondition);
            addedLock = true;
        }
    }
    if (events && ((events & ~(K_POLLIN | K_POLLOUT)) || this->listening)) {
        if (!addedLock) {
            BOXEDWINE_CONDITION_ADD_PARENT(this->lockCond, parentCondition);
        }
    }
    if (events == 0) {
        BOXEDWINE_CONDITION_REMOVE_PARENT(this->lockCond, parentCondition);
    }
}

U32 KNetLinkObject::writev(KThread* thread, U32 iov, S32 iovcnt) {
    U32 len = 0;
    KMemory* memory = thread->memory;


    for (S32 i = 0; i < iovcnt; i++) {
        U32 buf = memory->readd(iov + i * 8);
        U32 toWrite = memory->readd(iov + i * 8 + 4);
        S32 result;

        if (toWrite) {
            result = this->write(thread, buf, toWrite);
            if (result < 0) {
                if (i > 0) {
                    return len;
                }
                return result;
            }
            len += result;
        }
    }
    return len;
}

U32 KNetLinkObject::write(KThread* thread, U32 buffer, U32 len) {
    BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(lockCond);
    thread->memory->performOnMemory(buffer, len, true, [this](U8* ram, U32 len) {
        this->recvBuffer.insert(this->recvBuffer.end(), ram, ram + len);
        return true;
        });
    BOXEDWINE_CONDITION_SIGNAL_ALL(lockCond);
    return len;
}

U32 KNetLinkObject::writeNative(U8* buffer, U32 len) {
    BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(lockCond);
    recvBuffer.insert(recvBuffer.end(), buffer, buffer + len);
    BOXEDWINE_CONDITION_SIGNAL_ALL(lockCond);
    return len;
}

U32 KNetLinkObject::readNative(U8* buffer, U32 len) {
    BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(this->lockCond);
    while (this->recvBuffer.size() == 0) {
        if (this->inClosed) {
            return 0;
        }
        if (!this->blocking) {
            return -K_EWOULDBLOCK;
        }
        BOXEDWINE_CONDITION_WAIT(this->lockCond);
#ifdef BOXEDWINE_MULTI_THREADED
        if (KThread::currentThread()->terminating) {
            return -K_EINTR;
        }
        if (KThread::currentThread()->startSignal) {
            KThread::currentThread()->startSignal = false;
            return -K_CONTINUE;
        }
#endif
    }
    //printf("readNative: %0.8X size=%d capacity=%d writeLen=%d", (int)&this->recvBuffer, (int)this->recvBuffer.size(), (int)this->recvBuffer.capacity(), len);
    if (len > this->recvBuffer.size()) {
        len = (U32)this->recvBuffer.size();
    }
    std::copy(this->recvBuffer.begin(), this->recvBuffer.begin() + len, buffer);
    this->recvBuffer.erase(this->recvBuffer.begin(), this->recvBuffer.begin() + len);
    BOXEDWINE_CONDITION_SIGNAL_ALL(this->lockCond);
    //printf("    readNative: %0.8X size=%d capacity=%d writeLen=%d", (int)&this->recvBuffer, (int)this->recvBuffer.size(), (int)this->recvBuffer.capacity(), len);
    return len;
}

U32 KNetLinkObject::read(KThread* thread, U32 buffer, U32 len) {
    U32 count = 0;
    KMemory* memory = thread->memory;

    if (!memory->canWrite(buffer, len)) {
        return -K_EFAULT;
    }
    BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(this->lockCond);
    while (this->recvBuffer.size() == 0) {
        if (this->inClosed) {
            return 0;
        }
        if (!this->blocking) {
            return -K_EWOULDBLOCK;
        }
        BOXEDWINE_CONDITION_WAIT(this->lockCond);
#ifdef BOXEDWINE_MULTI_THREADED
        if (thread->terminating) {
            return -K_EINTR;
        }
        if (thread->startSignal) {
            thread->startSignal = false;
            return -K_CONTINUE;
        }
#endif
    }
    count = std::min(len, (U32)this->recvBuffer.size());
    memory->performOnMemory(buffer, count, false, [this](U8* ram, U32 len) {
        std::copy(this->recvBuffer.begin(), this->recvBuffer.begin() + len, ram);
        this->recvBuffer.erase(this->recvBuffer.begin(), this->recvBuffer.begin() + len);
        return true;
        });

    BOXEDWINE_CONDITION_SIGNAL_ALL(this->lockCond);
    return count;
}

U32 KNetLinkObject::stat(KProcess* process, U32 address, bool is64) {
    KSystem::writeStat(process, B(""), address, is64, true, 0, K_S_IFSOCK | K__S_IWRITE | K__S_IREAD, 0, 0, 4096, 0, this->lastModifiedTime, 1);
    return 0;
}

U32 KNetLinkObject::map(KThread* thread, U32 address, U32 len, S32 prot, S32 flags, U64 off) {
    return 0;
}

bool KNetLinkObject::canMap() {
    return false;
}

BString KNetLinkObject::selfFd() {
    return B("anon_inode:[pipe]");
}

S64 KNetLinkObject::seek(S64 pos) {
    return -K_ESPIPE;
}

S64 KNetLinkObject::getPos() {
    return 0;
}

U32 KNetLinkObject::ioctl(KThread* thread, U32 request) {
    return -K_ENOTTY;
}

bool KNetLinkObject::supportsLocks() {
    return false;
}

S64 KNetLinkObject::length() {
    return -1;
}

U32 KNetLinkObject::bind(KThread* thread, KFileDescriptor* fd, U32 address, U32 len) {
    KMemory* memory = thread->memory;

    U32 family = memory->readw(address);

    U32 port = memory->readd(address + 4);
    if (port == 0) {
        port = thread->process->id;
    }
    this->afBound = std::make_shared<k_sockaddr_nl>();
    this->afBound->nl_family = memory->readw(address);
    this->afBound->nl_pid = port;
    this->afBound->nl_groups = memory->readd(address + 8);
    this->listening = 1;
    return 0;
}

U32 KNetLinkObject::connect(KThread* thread, KFileDescriptor* fd, U32 address, U32 len) {
    return -K_EOPNOTSUPP;
}

U32 KNetLinkObject::listen(KThread* thread, KFileDescriptor* fd, U32 backlog) {
    return -K_EOPNOTSUPP;
}

U32 KNetLinkObject::accept(KThread* thread, KFileDescriptor* fd, U32 address, U32 len, U32 flags) {
    return -K_EOPNOTSUPP;
}

U32 KNetLinkObject::getsockname(KThread* thread, KFileDescriptor* fd, U32 address, U32 plen) {
    KMemory* memory = thread->memory;

    if (afBound) {
        memory->writew(address, this->afBound->nl_family);
        memory->writew(address + 2, 0);
        memory->writed(address + 4, this->afBound->nl_pid);
        memory->writed(address + 8, this->afBound->nl_groups);
        memory->writed(plen, 12);
        return 0;
    }
    return -K_ENOTCONN;
}

U32 KNetLinkObject::getpeername(KThread* thread, KFileDescriptor* fd, U32 address, U32 plen) {
    return -K_EOPNOTSUPP;
}

U32 KNetLinkObject::shutdown(KThread* thread, KFileDescriptor* fd, U32 how) {
    return -K_EOPNOTSUPP;
}

U32 KNetLinkObject::setsockopt(KThread* thread, KFileDescriptor* fd, U32 level, U32 name, U32 value, U32 len) {
    KMemory* memory = thread->memory;

    if (level == K_SOL_SOCKET) {
        switch (name) {
        case K_SO_RCVBUFFORCE:
        case K_SO_RCVBUF:
            if (len != 4)
                kpanic("KUnixSocketObject::setsockopt SO_RCVBUF expecting len of 4");
            this->recvLen = memory->readd(value);
            break;
        case K_SO_SNDBUFFORCE:
        case K_SO_SNDBUF:
            if (len != 4)
                kpanic("KUnixSocketObject::setsockopt SO_SNDBUF expecting len of 4");
            this->sendLen = memory->readd(value);
            break;
        case K_SO_PASSCRED:
            break;
        case K_SO_ATTACH_FILTER:
            break;
        default:
            kwarn("KUnixSocketObject::setsockopt name %d not implemented", name);
        }
    } else {
        kwarn("KUnixSocketObject::setsockopt level %d not implemented", level);
    }
    return 0;
}

U32 KNetLinkObject::getsockopt(KThread* thread, KFileDescriptor* fd, U32 level, U32 name, U32 value, U32 len_address) {
    KMemory* memory = thread->memory;

    U32 len = memory->readd(len_address);
    if (level == K_SOL_SOCKET) {
        if (name == K_SO_RCVBUF) {
            if (len != 4)
                kpanic("KNetLinkObject::getsockopt SO_RCVBUF expecting len of 4");
            memory->writed(value, this->recvLen);
        } else if (name == K_SO_SNDBUF) {
            if (len != 4)
                kpanic("KNetLinkObject::getsockopt SO_SNDBUF expecting len of 4");
            memory->writed(value, this->sendLen);
        } else if (name == K_SO_ERROR) {
            if (len != 4)
                kpanic("KNetLinkObject::getsockopt SO_ERROR expecting len of 4");
            memory->writed(value, this->error);
        } else if (name == K_SO_TYPE) {
            if (len != 4)
                kpanic("KNetLinkObject::getsockopt K_SO_TYPE expecting len of 4");
            memory->writed(value, this->type);
        } else {
            kwarn("KUnixSocketObject::getsockopt name %d not implemented", name);
            return -K_EINVAL;
        }
    } else {
        kwarn("KUnixSocketObject::getsockopt level %d not implemented", level);
        return -K_EINVAL;
    }
    return 0;
}

U32 KNetLinkObject::sendmsg(KThread* thread, KFileDescriptor* fd, U32 address, U32 flags) {
    kpanic("KNetLinkObject::sendmsg not implemented");
    return 0;
}

U32 KNetLinkObject::recvmsg(KThread* thread, KFileDescriptor* fd, U32 address, U32 flags) {
    MsgHdr hdr = {};
    U32 result = 0;
    KMemory* memory = thread->memory;

    BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(this->lockCond);
    while (1) {
        if (this->recvBuffer.size()) {
            readMsgHdr(thread, address, &hdr);
            for (U32 i = 0; i < hdr.msg_iovlen; i++) {
                U32 p = memory->readd(hdr.msg_iov + 8 * i);
                U32 len = memory->readd(hdr.msg_iov + 8 * i + 4);

                U32 count = std::min(len, (U32)this->recvBuffer.size());
                memory->performOnMemory(p, count, false, [this](U8* ram, U32 len) {
                    std::copy(this->recvBuffer.begin(), this->recvBuffer.begin() + len, ram);
                    this->recvBuffer.erase(this->recvBuffer.begin(), this->recvBuffer.begin() + len);
                    return true;
                    });
                result += count;
            }
            if (this->type == K_SOCK_STREAM)
                memory->writed(address + 4, 0); // msg_namelen, set to 0 for connected sockets
            memory->writed(address + 20, 0); // msg_controllen
            return result;
        }
        if (this->inClosed) {
            return 0;
        }
        if (!this->blocking) {
            return -K_EWOULDBLOCK;
        }
        // :TODO: what about a time out
        BOXEDWINE_CONDITION_WAIT(this->lockCond);
#ifdef BOXEDWINE_MULTI_THREADED
        if (thread->terminating) {
            return -K_EINTR;
        }
        if (thread->startSignal) {
            thread->startSignal = false;
            return -K_CONTINUE;
        }
#endif
    }
    BOXEDWINE_CONDITION_SIGNAL_ALL(this->lockCond);
    return result;
}

 struct boxed_nlmsghdr
 {
    U32                nlmsg_len;        /* Length of message including header */
    U16                nlmsg_type;        /* Message content */
    U16                nlmsg_flags;        /* Additional flags */
    U32                nlmsg_seq;        /* Sequence number */
    U32                nlmsg_pid;        /* Sending process port ID */
};

struct boxed_ifinfomsg {
    U8 ifi_family;
    U8 __ifi_pad;
    U16 ifi_type;		/* ARPHRD_* */
    S32 ifi_index;		/* Link index	*/
    U32 ifi_flags;		/* IFF_* flags	*/
    U32 ifi_change;		/* IFF_* change mask */
};
U32 KNetLinkObject::sendto(KThread* thread, KFileDescriptor* fd, U32 message, U32 length, U32 flags, U32 dest_addr, U32 dest_len) {
    if (length >= 12) {
        U32 len = thread->memory->readd(message);
        U16 type = thread->memory->readw(message + 4);
        U16 flags = thread->memory->readw(message + 6);
        U32 seq = thread->memory->readd(message + 8);

        // start in Wine 7 (with Tiny Core Linux), this function will crash if I don't return something.  I'm not sure if what I'm returning is correct, but it stops the crash
        //
        // dlls\nsiproxy.sys\ndis.c
        // static unsigned int update_if_table( void )
        // {
        //    struct if_nameindex* indices = if_nameindex(), * entry;
        //    unsigned int append_count = 0;
        //
        //    for (entry = indices; entry->if_index; entry++)
        if (type == 0x12) { // RTM_GETLINK
            boxed_nlmsghdr hdr;
            boxed_ifinfomsg msg;
            hdr.nlmsg_len = sizeof(hdr) + sizeof(msg);
            hdr.nlmsg_type = type;
            hdr.nlmsg_flags = 301; // NLM_F_REQUEST | NLM_F_DUMP
            hdr.nlmsg_seq = seq;
            hdr.nlmsg_pid = 0;
            msg.ifi_family = 0;
            msg.ifi_type = 1;
            msg.ifi_index = 1;
            msg.ifi_flags = 0;
            msg.ifi_change = 0xffffffff;
            BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(lockCond);
            recvBuffer.insert(recvBuffer.end(), (U8*)&hdr, ((U8*)&hdr) + sizeof(hdr));
            recvBuffer.insert(recvBuffer.end(), (U8*)&msg, ((U8*)&msg) + sizeof(msg));
            BOXEDWINE_CONDITION_SIGNAL_ALL(lockCond);
            return sizeof(hdr) + sizeof(msg);
        }
    }
    return -1; // if we return 0 here and pretend it succeeded, then some library might call recvfrom on a block thread to get the response and hang the app
}

U32 KNetLinkObject::recvfrom(KThread* thread, KFileDescriptor* fd, U32 buffer, U32 length, U32 flags, U32 address, U32 address_len) {
    if (address == 0) {
        if (flags) {
            kpanic("KUnixSocketObject::recvfrom unhandled flags=%x", flags);
        }
        return read(thread, buffer, length);
    }
    return 0;
}
