#include "boxedwine.h"

#include "kunixsocket.h"
#include "ksocket.h"
#include "kstat.h"

KUnixSocketObject::KUnixSocketObject(U32 pid, U32 domain, U32 type, U32 protocol) : KSocketObject(KTYPE_UNIX_SOCKET, domain, type, protocol), recvBuffer(1024*1024), pid(0), connecting(NULL), connection(NULL), pendingConnectionNode(this) {
}

KUnixSocketObject::~KUnixSocketObject() {
    if (this->node) {
        this->node->getParent()->removeChildByName(this->node->name); 
    }

    if (this->connection) {
        this->connection->connection = NULL;
        this->connection->inClosed = true;
        this->connection->outClosed = true;
        this->connection->wakeAndResetWaitingOnReadThreads();
        this->connection->wakeAndResetWaitingOnWriteThreads();
        this->connection->wakeAndResetWaitingOnConnectionThreads();
    }    
    if (this->connecting) {	
        this->pendingConnectionNode.remove();
        this->connecting = NULL;
    }
    this->pendingConnections.for_each([](KListNode<KUnixSocketObject*>* node) {
        node->data->connecting = NULL;
        node->data->wakeAndResetWaitingOnConnectionThreads();
    });
    this->wakeAndResetWaitingOnReadThreads();
    this->wakeAndResetWaitingOnWriteThreads();
    this->wakeAndResetWaitingOnConnectionThreads();
}

void KUnixSocketObject::setBlocking(bool blocking) {
    this->blocking = blocking;
}

bool KUnixSocketObject::isBlocking() {
    return this->blocking;
}

void  KUnixSocketObject::setAsync(bool isAsync) {
    if (isAsync)
        kpanic(" UnixSocketObject::setAsync not implemented yet");
}

bool  KUnixSocketObject::isAsync() {
    return false;
}

KFileLock*  KUnixSocketObject::getLock(KFileLock* lock) {
    kwarn("UnixSocketObject::getLock not implemented yet");
    return NULL;
}

U32  KUnixSocketObject::setLock(KFileLock* lock, bool wait) {
    kwarn(" UnixSocketObject::setLock not implemented yet");
    return -1;
}

bool KUnixSocketObject::isOpen() {
    return this->listening || this->connection;
}

bool KUnixSocketObject::isReadReady() {
    return this->inClosed || this->recvBuffer.getOccupied() || this->pendingConnections.size() || this->msgs.size();
}

bool KUnixSocketObject::isWriteReady() {
    return this->connection!=NULL;
}

void KUnixSocketObject::waitForEvents(U32 events) {
    KThread* thread = KThread::currentThread();

    if (events & K_POLLIN) {
        this->waitOnSocketRead(thread);
    }
    if (events & K_POLLOUT) {
        this->waitOnSocketWrite(thread);
    }
    if ((events & ~(K_POLLIN | K_POLLOUT)) || this->listening) {
        this->waitOnSocketConnect(thread);
    }
}

U32 KUnixSocketObject::internal_write(U32 buffer, U32 len) {
    U32 count=0;

    if (this->type == K_SOCK_DGRAM) {
        if (!strcmp(this->destAddress.data, "/dev/log")) {
            char tmp[MAX_FILEPATH_LEN];
            printf("%s\n", getNativeString(buffer, tmp, sizeof(tmp)));
        }
        return len;
    }
    if (this->outClosed || !this->connection)
        return -K_EPIPE;

    if (this->connection->recvBuffer.getFree()<len) {
        if (!this->blocking) {
            return -K_EWOULDBLOCK;
        }
        this->waitOnSocketWrite(KThread::currentThread());
        return -K_WAIT;                
    }   
    
    //printf("internal_write: %0.8X size=%d capacity=%d writeLen=%d", (int)&this->connection->recvBuffer, (int)this->connection->recvBuffer.size(), (int)this->connection->recvBuffer.capacity(), len);

    if (len>this->connection->recvBuffer.getFree())
        len = this->connection->recvBuffer.getFree();
    while (len) {
        S8 tmp[4096];
        U32 todo = len;

        if (todo>4096)
            todo = 4096;
        memcopyToNative(buffer, tmp, todo);
        this->connection->recvBuffer.write(tmp, todo);
        buffer+=todo;
        len-=todo;
        count+=todo;
    }     
    return count;
}

U32 KUnixSocketObject::writev(U32 iov, S32 iovcnt) {
    U32 len=0;
    S32 i;

    for (i=0;i<iovcnt;i++) {
        U32 buf = readd(iov + i * 8);
        U32 toWrite = readd(iov + i * 8 + 4);
        S32 result;

        result = this->internal_write(buf, toWrite);
        if (result<0) {
            if (i>0) {
                return len;
            }
            return result;
        }
        len+=result;
    }    
    if (this->connection) {
        this->connection->wakeAndResetWaitingOnReadThreads();
    }
    return len;
}

U32 KUnixSocketObject::write(U32 buffer, U32 len) {
    U32 result = this->internal_write(buffer, len);    
    if (this->connection) {
        this->connection->wakeAndResetWaitingOnReadThreads();
    }
    return result;
}

U32 KUnixSocketObject::writeNative(U8* buffer, U32 len) {
    if (this->type == K_SOCK_DGRAM) {
        if (!strcmp(this->destAddress.data, "/dev/log")) {
            printf("%s\n", buffer);
        }
        return len;
    }
    if (this->outClosed || !this->connection)
        return -K_EPIPE;

    //printf("writeNative: %0.8X size=%d capacity=%d writeLen=%d", (int)&this->connection->recvBuffer, (int)this->connection->recvBuffer.size(), (int)this->connection->recvBuffer.capacity(), len);
    if (this->connection->recvBuffer.getFree()<len) {
        if (!this->blocking) {
            return -K_EWOULDBLOCK;
        }
        this->waitOnSocketWrite(KThread::currentThread());
        return -K_WAIT;                
    }   
    if (len>this->connection->recvBuffer.getFree())
        len = this->connection->recvBuffer.getFree();
    this->connection->recvBuffer.write((S8*)buffer, len);
    //printf("    writeNative: %0.8X size=%d capacity=%d writeLen=%d", (int)&this->connection->recvBuffer, (int)this->connection->recvBuffer.size(), (int)this->connection->recvBuffer.capacity(), len);
    return len;
}

U32 unixsocket_write_native_nowait(const BoxedPtr<KObject>& obj, U8* value, int len) {
    if (obj->type!=KTYPE_UNIX_SOCKET)
        return 0;
    BoxedPtr<KUnixSocketObject> s = (KUnixSocketObject*)obj.get();

    if (s->type == K_SOCK_DGRAM) {
        return 0;
    }
    if (s->outClosed || !s->connection)
        return -K_EPIPE;
    if (s->connection->recvBuffer.getFree()<(U32)len) {
        return -K_EWOULDBLOCK;
    }
    //printf("SOCKET write len=%d bufferSize=%d pos=%d\n", len, s->connection->recvBufferLen, s->connection->recvBufferWritePos);
    s->connection->recvBuffer.write((S8*)value, len);

    if (s->connection) {
        s->connection->wakeAndResetWaitingOnReadThreads();
    }

    return len;
}

U32 KUnixSocketObject::readNative(U8* buffer, U32 len) {
    U32 count = 0;
    if (!this->inClosed && !this->connection)
        return -K_EPIPE;
    if (this->recvBuffer.getOccupied()==0) {
        if (this->inClosed) {
            return 0;
        }
        if (!this->blocking) {
            return -K_EWOULDBLOCK;
        }
        this->waitOnSocketRead(KThread::currentThread());
        return -K_WAIT;
    }
    //printf("readNative: %0.8X size=%d capacity=%d writeLen=%d", (int)&this->recvBuffer, (int)this->recvBuffer.size(), (int)this->recvBuffer.capacity(), len);
    if (len>this->recvBuffer.getOccupied())
        len = this->recvBuffer.getOccupied();
    this->recvBuffer.read((S8*)buffer, len);
    if (this->connection)
        this->connection->wakeAndResetWaitingOnWriteThreads();
    //printf("    readNative: %0.8X size=%d capacity=%d writeLen=%d", (int)&this->recvBuffer, (int)this->recvBuffer.size(), (int)this->recvBuffer.capacity(), len);
    return len;
}

U32 KUnixSocketObject::read(U32 buffer, U32 len) {
    U32 count = 0;
    if (!this->inClosed && !this->connection)
        return -K_EPIPE;
    if (this->recvBuffer.getOccupied()==0) {
        if (this->inClosed) {
            return 0;
        }
        if (!this->blocking) {
            return -K_EWOULDBLOCK;
        }
        this->waitOnSocketRead(KThread::currentThread());
        return -K_WAIT;
    }
    // :TODO: remove extra copy
    while (len && this->recvBuffer.getOccupied()!=0) {
        S8 tmp[4096];
        U32 todo = len;

        if (todo > 4096)
            todo = 4096;
        if (todo > this->recvBuffer.getOccupied())
            todo = (U32)this->recvBuffer.getOccupied();

        this->recvBuffer.read(tmp, len);
        
        memcopyFromNative(buffer, tmp, todo);

        buffer += todo;
        count += todo;
        len -= todo;
    }      
    if (this->connection)
        this->connection->wakeAndResetWaitingOnWriteThreads();

    return count;
}

U32 KUnixSocketObject::stat(U32 address, bool is64) {
    KSystem::writeStat("", address, is64, true, (this->node?this->node->id:0), K_S_IFSOCK|K__S_IWRITE|K__S_IREAD, (this->node?this->node->rdev:0), 0, 4096, 0, this->lastModifiedTime, 1);
    return 0;
}

U32 KUnixSocketObject::map(U32 address, U32 len, S32 prot, S32 flags, U64 off) {
    return 0;
}

bool KUnixSocketObject::canMap() {
    return false;
}

S64 KUnixSocketObject::seek(S64 pos) {
    return -K_ESPIPE;
}

S64 KUnixSocketObject::getPos() {
    return 0;
}

U32 KUnixSocketObject::ioctl( U32 request) {
    return -K_ENOTTY;
}

bool KUnixSocketObject::supportsLocks() {
    return false;
}

S64 KUnixSocketObject::length() {
    return -1;
}

class UnixSocketNode : public FsNode {
public:
    UnixSocketNode(U32 id, U32 rdev, const std::string& path, BoxedPtr<FsNode> parent) : FsNode(Socket, id, rdev, path, "", false, parent) {}
    U32 rename(const std::string& path) {return -K_EIO;}
    bool remove() {return false;}
    U64 lastModified() {return 0;}
    U64 length() {return 0;}
    FsOpenNode* open(U32 flags) {kwarn("unixsocket_open was called, this shouldn't happen.  syscall_open should detect we have a kobject already"); return NULL;}
    U32 getType(bool checkForLink) {return 12;} // DT_SOCK
    U32 getMode() {return K__S_IREAD | K__S_IWRITE | K_S_IFSOCK;}
    U32 removeDir() {kpanic("UnixSocket::removeDir not implemented"); return 0;}
    U32 setTimes(U64 lastAccessTime, U32 lastAccessTimeNano, U64 lastModifiedTime, U32 lastModifiedTimeNano) {klog("UnixSocket::setTimes not implemented"); return 0;}
};

U32 KUnixSocketObject::bind(KFileDescriptor* fd, U32 address, U32 len) {
    U32 family = readw(address);
    if (family==K_AF_UNIX) {
        char tmp[MAX_FILEPATH_LEN];
        const char* name = socketAddressName(address, len, tmp, sizeof(tmp));

        if (!name || !name[0]) {
            return 0; // :TODO: why does XOrg need this
        }
        BoxedPtr<FsNode> node = Fs::getNodeFromLocalPath(KThread::currentThread()->process->currentDirectory, name, true);
        if (node) {
            return -K_EADDRINUSE;
        }
        BoxedPtr<FsNode> parentNode = Fs::getNodeFromLocalPath("", KThread::currentThread()->process->currentDirectory, true);
        std::string fullpath = Fs::getFullPath(KThread::currentThread()->process->currentDirectory, name);
        BoxedPtr<UnixSocketNode> socketNode = new UnixSocketNode(0, 2, fullpath, parentNode);
        parentNode->addChild(socketNode);
        BoxedPtr<KUnixSocketObject> s = (KUnixSocketObject*)fd->kobject.get();
        socketNode->kobject = fd->kobject;
        s->node = socketNode;
        return 0;
    } else if (family == K_AF_NETLINK) {
        BoxedPtr<KUnixSocketObject> s = (KUnixSocketObject*)fd->kobject.get();
        U32 port = readd(address + 4);
        if (port == 0) {
            port = KThread::currentThread()->process->id;
        }
        s->nl_port = port;
        s->listening = 1;
        return 0;
    }
    return -K_EAFNOSUPPORT;
}

U32 KUnixSocketObject::connect(KFileDescriptor* fd, U32 address, U32 len) {
    if (len-2>sizeof(this->destAddress.data)) {
        kpanic("Socket address is too big");
    }
    this->destAddress.family = readw(address);
    memcopyToNative(address + 2, this->destAddress.data, len - 2);
    if (this->type==K_SOCK_DGRAM) {
        this->connected = 1;		
        return 0;
    } else if (this->type==K_SOCK_STREAM) {
        KThread* thread = KThread::currentThread();
        if (this->destAddress.data[0]==0) {
            return -K_ENOENT;
        }
        if (this->domain==K_AF_UNIX) {
            BoxedPtr<FsNode> node = Fs::getNodeFromLocalPath(thread->process->currentDirectory, this->destAddress.data, true);

            if (!node || !node->kobject || node->kobject->type!=KTYPE_UNIX_SOCKET) {
                this->destAddress.family = 0;
                return -K_ECONNREFUSED;
            }            
            if (this->connection) {
                this->connected = 1;
                return 0;
            }            
            if (this->connecting) {
                if (this->blocking) {
                    waitOnSocketConnect(thread);
                    return -K_WAIT;
                } else {
                    return -K_EINPROGRESS;
                }
            } else {
                BoxedPtr<KUnixSocketObject> destination = (KUnixSocketObject*)node->kobject.get();

                destination->pendingConnections.addToBack(&this->pendingConnectionNode);
                destination->wakeAndResetWaitingOnConnectionThreads();

                if (!this->blocking) {
                    return -K_EINPROGRESS;
                }
                // :TODO: what about a time out
                
                waitOnSocketConnect(thread);
                return -K_WAIT;
            }
        } else {
            kpanic("connect not implemented for domain %d", this->domain);
        }
    } else {
        kpanic("connect not implemented for type %d", this->type);
    }
    // should never get here
    return 0;
}

U32 KUnixSocketObject::listen(KFileDescriptor* fd, U32 backlog) {
    if (!this->node) {
        return -K_EDESTADDRREQ;
    }
    if (this->connection || this->connecting) {
        return -K_EINVAL;
    }
    this->listening = true;
    return 0;
}

U32 KUnixSocketObject::accept(KFileDescriptor* fd, U32 address, U32 len) {
    if (!this->pendingConnections.size()) {
        if (!this->blocking)
            return -K_EWOULDBLOCK;
        this->waitOnSocketConnect(KThread::currentThread());
        return -K_WAIT;
    }
    
    KListNode<KUnixSocketObject*>* connectionNode = this->pendingConnections.front();
    connectionNode->remove();

    BoxedPtr<KUnixSocketObject> resultSocket = new KUnixSocketObject(this->pid, domain, type, protocol);
    KFileDescriptor* result = KThread::currentThread()->process->allocFileDescriptor(resultSocket, K_O_RDWR, 0, -1, 0);

    connection = connectionNode->data; // weak reference
    connection->connection = resultSocket.get();
    //connection->connected = true; this will be handled when the connecting thread is unblocked    

    resultSocket->connected = true;
    resultSocket->connection = connectionNode->data; // weak reference
    
    if (connection->waitingOnConnectThread.size()) {
        connection->wakeAndResetWaitingOnConnectionThreads();
    }

    return result->handle;
}

U32 KUnixSocketObject::getsockname(KFileDescriptor* fd, U32 address, U32 plen) {
    U32 len = readd( plen);
    if (this->domain == K_AF_NETLINK) {
        if (len>0 && len<12)
            kpanic("getsocketname: AF_NETLINK wrong address size");
        writew(address, this->domain);
        writew(address + 2, 0);
        writed(address + 4, this->nl_port);
        writed(address + 8, 0);
        writed(plen, 12);
        return 0;
    } else if (this->domain == K_AF_UNIX) {
        writew(address, this->destAddress.family);
        len-=2;
        if (len>sizeof(this->destAddress.data))
            len = sizeof(this->destAddress.data);
        memcopyFromNative(address + 2, this->destAddress.data, len);
        writed(plen, 2 + (U32)strlen(this->destAddress.data) + 1);
        return 0;
    }
    kwarn("KUnixSocketObject::getsockname not implemented for domain %d", this->domain);
    return 0;
}

U32 KUnixSocketObject::getpeername(KFileDescriptor* fd, U32 address, U32 plen) {
    if (!this->connection)
        return -K_ENOTCONN;
    U32 len = readd( plen);
    writew(address, this->destAddress.family);
    len-=2;
    if (len>sizeof(this->destAddress.data))
        len = sizeof(this->destAddress.data);
    memcopyFromNative(address + 2, this->destAddress.data, len);
    writed(plen, 2 + (U32)strlen(this->destAddress.data) + 1);
    return 0;
}

U32 KUnixSocketObject::shutdown(KFileDescriptor* fd, U32 how) {
    if (this->type == K_SOCK_DGRAM) {
        kpanic("shutdown on SOCK_DGRAM not implemented");
    }
    if (!this->connection) {
        return -K_ENOTCONN;
    }
    if (how == K_SHUT_RD) {
        this->inClosed=true;
        this->connection->outClosed=true;
        this->connection->wakeAndResetWaitingOnWriteThreads();
    } else if (how == K_SHUT_WR) {
        this->outClosed=true;
        this->connection->inClosed=true;
        this->connection->wakeAndResetWaitingOnReadThreads();
    } else if (how == K_SHUT_RDWR) {
        this->outClosed=true;
        this->inClosed=true;
        this->connection->outClosed=true;
        this->connection->inClosed=true;
        this->connection->wakeAndResetWaitingOnReadThreads();
        this->connection->wakeAndResetWaitingOnWriteThreads();
    }
    this->connection->wakeAndResetWaitingOnConnectionThreads();
    return 0;
}

U32 KUnixSocketObject::setsockopt(KFileDescriptor* fd, U32 level, U32 name, U32 value, U32 len) {
    if (level == K_SOL_SOCKET) {
        switch (name) {
            case K_SO_RCVBUF:
                if (len!=4)
                    kpanic("KUnixSocketObject::setsockopt SO_RCVBUF expecting len of 4");
                this->recvLen = readd(value);
            case K_SO_SNDBUF:
                if (len != 4)
                    kpanic("KUnixSocketObject::setsockopt SO_SNDBUF expecting len of 4");
                this->sendLen = readd(value);
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

U32 KUnixSocketObject::getsockopt(KFileDescriptor* fd, U32 level, U32 name, U32 value, U32 len_address) {
    U32 len = readd(len_address);
    if (level == K_SOL_SOCKET) {
        if (name == K_SO_RCVBUF) {
            if (len!=4)
                kpanic("KUnixSocketObject::getsockopt SO_RCVBUF expecting len of 4");
            writed(value, this->recvLen);
        } else if (name == K_SO_SNDBUF) {
            if (len != 4)
                kpanic("KUnixSocketObject::getsockopt SO_SNDBUF expecting len of 4");
            writed(value, this->sendLen);
        } else if (name == K_SO_ERROR) {
            if (len != 4)
                kpanic("KUnixSocketObject::getsockopt SO_ERROR expecting len of 4");
            writed(value, this->error);
        } else if (name == K_SO_TYPE) { 
            if (len != 4)
                kpanic("KUnixSocketObject::getsockopt K_SO_TYPE expecting len of 4");
            writed(value, this->type);
        } else if (name == K_SO_PEERCRED) {
            if (this->domain!=K_AF_UNIX) {
                return -K_EINVAL; // :TODO: is this right
            }
            if (!this->connection) {
                return -K_EINVAL; // :TODO: is this right
            }
            if (len != 12)
                kpanic("KUnixSocketObject::getsockopt SO_PEERCRED expecting len of 12");
            writed(value, this->connection->pid);
            writed(value + 4, KThread::currentThread()->process->userId);
            writed(value + 8, KThread::currentThread()->process->groupId);
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

U32 KUnixSocketObject::sendmsg(KFileDescriptor* fd, U32 address, U32 flags) {
    MsgHdr hdr;
    KThread* thread = KThread::currentThread();
    U32 result = 0;

    if (!this->connection) {
        if (this->type == K_SOCK_STREAM) {
            return ENOTCONN;
        }
        kpanic("KUnixSocketObject::sendmsg not implemented for type: %d", this->type);
        return 0;
    }
    readMsgHdr(address, &hdr);

    BoxedPtr<KSocketMsg> msg = new KSocketMsg();

    if (hdr.msg_control) {
        CMsgHdr cmsg;			

        readCMsgHdr(hdr.msg_control, &cmsg);
        if (cmsg.cmsg_level != K_SOL_SOCKET) {
            kpanic("KUnixSocketObject::sendmsg control level %d not implemented", cmsg.cmsg_level);
        } else if (cmsg.cmsg_type != K_SCM_RIGHTS) {
            kpanic("KUnixSocketObject::sendmsg control type %d not implemented", cmsg.cmsg_level);
        } else if ((cmsg.cmsg_len & 3) != 0) {
            kpanic("KUnixSocketObject::sendmsg control len %d not implemented", cmsg.cmsg_len);
        }

        for (U32 i=0;i<hdr.msg_controllen/16;i++) {
            KFileDescriptor* f = thread->process->getFileDescriptor(readd(hdr.msg_control + 16 * i + 12));
            if (!f) {
                kpanic("KUnixSocketObject::sendmsg tried to send a bad file descriptor");
            } else {
                KSocketMsgObject d;
                d.object = f->kobject;
                d.accessFlags = f->accessFlags;
                msg->objects.push_back(d);
            }
        }				
    }
    for (U32 i=0;i<hdr.msg_iovlen;i++) {
        U32 p = readd(hdr.msg_iov + 8 * i);
        U32 len = readd(hdr.msg_iov + 8 * i + 4);

        msg->data.push_back((U8)len);
        msg->data.push_back((U8)(len >> 8));
        msg->data.push_back((U8)(len >> 16));
        msg->data.push_back((U8)(len >> 24));
        while (len) {
            msg->data.push_back(readb(p++));
            len--;
            result++;
        }
    }
    this->connection->msgs.push(msg);
    this->connection->wakeAndResetWaitingOnReadThreads();

    return result;
}

U32 KUnixSocketObject::recvmsg(KFileDescriptor* fd, U32 address, U32 flags) {
    MsgHdr hdr;
    U32 result = 0;

    if (this->domain==K_AF_NETLINK)
        return -K_EIO;
    if (!this->msgs.size()) {
        if (this->recvBuffer.getOccupied()) {
            readMsgHdr(address, &hdr);        
            for (U32 i = 0; i < hdr.msg_iovlen; i++) {
                U32 p = readd(hdr.msg_iov + 8 * i);
                U32 len = readd(hdr.msg_iov + 8 * i + 4);
                
                result+=this->read(p, len);
            }
            if (this->type==K_SOCK_STREAM)
                writed(address + 4, 0); // msg_namelen, set to 0 for connected sockets
            writed(address + 20, 0); // msg_controllen
            return result;
        }
        if (!this->blocking) {
            return -K_EWOULDBLOCK;
        }
        // :TODO: what about a time out
        waitOnSocketRead(KThread::currentThread());	
        return -K_WAIT;
    }

    readMsgHdr(address, &hdr);
    BoxedPtr<KSocketMsg> msg = this->msgs.front();
    this->msgs.pop();

    if (hdr.msg_control) {
        KThread* thread = KThread::currentThread();
        U32 i=0;

        for (;i<hdr.msg_controllen/16 && i<msg->objects.size();i++) {
            KFileDescriptor* fd = thread->process->allocFileDescriptor(msg->objects[i].object, msg->objects[i].accessFlags, 0, -1, 0);
            writeCMsgHdr(hdr.msg_control + i * 16, 16, K_SOL_SOCKET, K_SCM_RIGHTS);
            writed(hdr.msg_control + i * 16 + 12, fd->handle);
        }
        writed(address + 20, i * 20);
    }
    U32 pos = 0;
    for (U32 i=0;i<hdr.msg_iovlen;i++) {
        U32 p = readd(hdr.msg_iov + 8 * i);
        U32 len = readd(hdr.msg_iov + 8 * i + 4);
        U32 dataLen = msg->data[pos] | msg->data[pos + 1] | msg->data[pos + 2] | msg->data[pos + 3];
        pos+=4;
        if (len<dataLen) {
            kpanic("unhandled socket msg logic");
        }
        memcopyFromNative(p, (S8*)msg->data.data() + pos, dataLen);
        result+=dataLen;
    }  
    if (this->connection)
        this->connection->wakeAndResetWaitingOnWriteThreads();
    return result;
}

U32 KUnixSocketObject::sendto(KFileDescriptor* fd, U32 message, U32 length, U32 flags, U32 dest_addr, U32 dest_len) {
    return 0;
}

U32 KUnixSocketObject::recvfrom(KFileDescriptor* fd, U32 buffer, U32 length, U32 flags, U32 address, U32 address_len) {
    return 0;
}