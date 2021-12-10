#include "boxedwine.h"

#include "kunixsocket.h"
#include "ksocket.h"
#include "kstat.h"

KUnixSocketObject::KUnixSocketObject(U32 pid, U32 domain, U32 type, U32 protocol) : KSocketObject(KTYPE_UNIX_SOCKET, domain, type, protocol), 
    lockCond("KUnixSocketObject::lockCond")
{
}

KUnixSocketObject::~KUnixSocketObject() {    
    if (this->node) {
        this->node->getParent()->removeChildByName(this->node->name); 
    }    

    std::shared_ptr<KUnixSocketObject> con = this->connection.lock();
    if (con) {
        BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(con->lockCond);
        con = this->connection.lock();
        if (con) {
            con->connection.reset();
            con->inClosed = true;
            con->outClosed = true;
            BOXEDWINE_CONDITION_SIGNAL_ALL(con->lockCond);
        }
    }        
    
    std::shared_ptr<KUnixSocketObject> c = this->connecting.lock();
    if (c) {
        BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(c->lockCond);
        auto it = c->pendingConnections.begin();
        while (it != c->pendingConnections.end()) {
            std::shared_ptr<KUnixSocketObject> p = (*it).lock();
            if (p == shared_from_this()) {
                it = c->pendingConnections.erase(it);
            } else {
                it++;
            }
        }
        this->connecting.reset();
    }
    BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(this->lockCond);
    for (auto& weakSocket : this->pendingConnections) {
        std::shared_ptr<KUnixSocketObject> s = weakSocket.lock();
        if (s) {
            s->connecting.reset();
            BOXEDWINE_CONDITION_SIGNAL_ALL_NEED_LOCK(s->lockCond);
        }
    }    
    BOXEDWINE_CONDITION_SIGNAL_ALL(this->lockCond);
}

void KUnixSocketObject::setBlocking(bool blocking) {
    BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(this->lockCond);
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
    kdebug("UnixSocketObject::getLock not implemented yet");
    return NULL;
}

U32  KUnixSocketObject::setLock(KFileLock* lock, bool wait) {
    kdebug(" UnixSocketObject::setLock not implemented yet");
    return -1;
}

bool KUnixSocketObject::isOpen() {
    return this->listening || !this->connection.expired();
}

bool KUnixSocketObject::isReadReady() {
    //BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(this->lockCond);
    return this->inClosed || this->recvBuffer.size() || this->pendingConnections.size() || this->msgs.size();
}

bool KUnixSocketObject::isWriteReady() {
    return !this->connection.expired();
}

void KUnixSocketObject::waitForEvents(BOXEDWINE_CONDITION& parentCondition, U32 events) {
    bool addedLock = false;

    if (events & K_POLLIN) {
        BOXEDWINE_CONDITION_ADD_CHILD_CONDITION(parentCondition, this->lockCond, nullptr);
        addedLock = true;
    }
    if (events & K_POLLOUT) {
        std::shared_ptr<KUnixSocketObject> con = this->connection.lock();
        if (con) {
            BOXEDWINE_CONDITION_ADD_CHILD_CONDITION(parentCondition, con->lockCond, nullptr);
        } else {
            if (!addedLock) {
                BOXEDWINE_CONDITION_ADD_CHILD_CONDITION(parentCondition, this->lockCond, nullptr);
                addedLock = true;
            }
        }
    }
    if ((events & ~(K_POLLIN | K_POLLOUT)) || this->listening) {
        if (!addedLock) {
            BOXEDWINE_CONDITION_ADD_CHILD_CONDITION(parentCondition, this->lockCond, nullptr);
        }
    }
}

U32 KUnixSocketObject::internal_write(const std::shared_ptr<KUnixSocketObject>& con, BOXEDWINE_CONDITION& cond, U32 buffer, U32 len) {
    U32 count=0;
    
    if (this->type == K_SOCK_DGRAM) {
        if (!strcmp(this->destAddress.data, "/dev/log")) {
            char tmp[MAX_FILEPATH_LEN];
            printf("%s\n", getNativeString(buffer, tmp, sizeof(tmp)));
        }
        return len;
    }
    if (this->outClosed || !con)
        return -K_EPIPE;  
    
    //printf("internal_write: %0.8X size=%d capacity=%d writeLen=%d", (int)&this->connection->recvBuffer, (int)this->connection->recvBuffer.size(), (int)this->connection->recvBuffer.capacity(), len);

    while (len) {
        S8 tmp[4096];
        U32 todo = len;

        if (todo>4096)
            todo = 4096;
        if (!KThread::currentThread()->memory->isValidReadAddress(buffer, todo)) {
            kwarn("KUnixSocketObject::internal_write about to crash reading buffer to buffer");
        }
        memcopyToNative(buffer, tmp, todo);
        con->recvBuffer.insert(con->recvBuffer.end(), tmp, tmp + todo);
        buffer+=todo;
        len-=todo;
        count+=todo;
    }     
    return count;
}

U32 KUnixSocketObject::writev(U32 iov, S32 iovcnt) {
    U32 len=0;
    S32 i;
    std::shared_ptr<KUnixSocketObject> con = this->connection.lock();

    BOXEDWINE_CONDITION& cond = (con?con->lockCond:this->lockCond);
    BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(cond);

    for (i=0;i<iovcnt;i++) {
        U32 buf = readd(iov + i * 8);
        U32 toWrite = readd(iov + i * 8 + 4);
        S32 result;

        result = this->internal_write(con, cond, buf, toWrite);
        if (result<0) {
            if (i>0) {
                return len;
            }
            return result;
        }
        len+=result;
    }    
    if (con) {
        BOXEDWINE_CONDITION_SIGNAL_ALL(cond);
    }
    return len;
}

U32 KUnixSocketObject::write(U32 buffer, U32 len) {
    std::shared_ptr<KUnixSocketObject> con = this->connection.lock();
    BOXEDWINE_CONDITION& cond = (con?con->lockCond:this->lockCond);
    BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(cond);
    U32 result = this->internal_write(con, cond, buffer, len);    
    if (con) {
        BOXEDWINE_CONDITION_SIGNAL_ALL(con->lockCond);
    }
    return result;
}

U32 KUnixSocketObject::writeNative(U8* buffer, U32 len) {
    if (this->type == K_SOCK_DGRAM) {
        if (!strcmp(this->destAddress.data, "/dev/log")) {
            klog("%s", buffer);
        }
        return len;
    }
    std::shared_ptr<KUnixSocketObject> con = this->connection.lock();
    if (this->outClosed || !con)
        return -K_EPIPE;

    BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(con->lockCond); 
    con->recvBuffer.insert(con->recvBuffer.end(), buffer, buffer + len);
    BOXEDWINE_CONDITION_SIGNAL_ALL(con->lockCond);
    return len;
}

U32 KUnixSocketObject::unixsocket_write_native_nowait(const std::shared_ptr<KObject>& obj, U8* value, int len) {
    if (obj->type!=KTYPE_UNIX_SOCKET)
        return 0;
    std::shared_ptr<KUnixSocketObject> s = std::dynamic_pointer_cast<KUnixSocketObject>(obj);

    if (s->type == K_SOCK_DGRAM) {
        return 0;
    }
    std::shared_ptr<KUnixSocketObject> con = s->connection.lock();
    if (s->outClosed || !con)
        return -K_EPIPE;

    BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(con->lockCond);
    //printf("SOCKET write len=%d bufferSize=%d pos=%d\n", len, s->connection->recvBufferLen, s->connection->recvBufferWritePos);
    con->recvBuffer.insert(con->recvBuffer.end(), value, value + len);
    BOXEDWINE_CONDITION_SIGNAL_ALL(con->lockCond);

    return len;
}

U32 KUnixSocketObject::readNative(U8* buffer, U32 len) {
    std::shared_ptr<KUnixSocketObject> con = this->connection.lock();
    if (!this->inClosed && !con)
        return -K_EPIPE;
    con = nullptr; // don't hold a strong reference to this, it we are blocking then it would prevent the con object from being destroyed when its process is closed
    BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(this->lockCond);
    while (this->recvBuffer.size()==0) {
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
    if (con) {
        BOXEDWINE_CONDITION_SIGNAL_ALL(this->lockCond);
    }
    //printf("    readNative: %0.8X size=%d capacity=%d writeLen=%d", (int)&this->recvBuffer, (int)this->recvBuffer.size(), (int)this->recvBuffer.capacity(), len);
    return len;
}

U32 KUnixSocketObject::read(U32 buffer, U32 len) {
    U32 count = 0;
    std::shared_ptr<KUnixSocketObject> con = this->connection.lock();
    if (!this->inClosed && !con)
        return -K_EPIPE;
    con = nullptr; // don't hold a strong reference to this, it we are blocking then it would prevent the con object from being destroyed when its process is closed
    BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(this->lockCond);
    while (this->recvBuffer.size()==0) {
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
    // :TODO: remove extra copy
    while (len && this->recvBuffer.size()!=0) {
        S8 tmp[4096];
        U32 todo = len;

        if (todo > 4096)
            todo = 4096;
        if (todo > this->recvBuffer.size())
            todo = (U32)this->recvBuffer.size();

        std::copy(this->recvBuffer.begin(), this->recvBuffer.begin() + todo, tmp);
        this->recvBuffer.erase(this->recvBuffer.begin(), this->recvBuffer.begin() + todo);

        if (!KThread::currentThread()->memory->isValidWriteAddress(buffer, todo)) {
            kwarn("KUnixSocketObject::read about to crash writing to buffer");
        }
        memcopyFromNative(buffer, tmp, todo);

        buffer += todo;
        count += todo;
        len -= todo;
    }      
    if (con) {
        BOXEDWINE_CONDITION_SIGNAL_ALL(this->lockCond);
    }

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
    UnixSocketNode(U32 id, U32 rdev, const std::string& path, BoxedPtr<FsNode> parent) : FsNode(Socket, id, rdev, path, "", "", false, parent) {}
    U32 rename(const std::string& path) {return -K_EIO;}
    bool remove() {if (!this->parent) return false; this->removeNodeFromParent(); return true;}
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
        std::string fullpath = Fs::getFullPath(KThread::currentThread()->process->currentDirectory, name);
        BoxedPtr<FsNode> parentNode = Fs::getNodeFromLocalPath("", Fs::getParentPath(fullpath), true);
        BoxedPtr<UnixSocketNode> socketNode = new UnixSocketNode(0, 2, fullpath, parentNode);
        parentNode->addChild(socketNode);
        std::shared_ptr<KUnixSocketObject> s = std::dynamic_pointer_cast<KUnixSocketObject>(fd->kobject);
        socketNode->kobject = fd->kobject;
        s->node = socketNode;
        return 0;
    } else if (family == K_AF_NETLINK) {
        std::shared_ptr<KUnixSocketObject> s = std::dynamic_pointer_cast<KUnixSocketObject>(fd->kobject);
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
            std::shared_ptr<KObject> kobject;
            if (node) {
                kobject = node->kobject.lock();
            }
            if (!node || !kobject || kobject->type!=KTYPE_UNIX_SOCKET) {
                this->destAddress.family = 0;
                return -K_ECONNREFUSED;
            }     
            std::shared_ptr<KUnixSocketObject> con = this->connection.lock();
            if (con) {
                this->connected = 1;
                return 0;
            }     
            con = nullptr; // don't hold a strong reference to this, it we are blocking then it would prevent the con object from being destroyed when its process is closed
            if (!this->connecting.expired()) {
                if (this->blocking) {
                    BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(this->lockCond);
                    while (!this->connecting.expired()) {
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
                    if (this->connection.expired()) {
                        return -K_ECONNREFUSED;
                    }
                    this->connected = 1;
                    return 0;
                } else {
                    return -K_EINPROGRESS;
                }
            } else {
                std::shared_ptr<KUnixSocketObject> destination = std::dynamic_pointer_cast<KUnixSocketObject>(node->kobject.lock());

                this->connecting = destination;
                BOXEDWINE_CONDITION_LOCK(destination->lockCond);
                std::shared_ptr< KUnixSocketObject> t = std::dynamic_pointer_cast<KUnixSocketObject>(shared_from_this());
                destination->pendingConnections.push_back(t);
                BOXEDWINE_CONDITION_SIGNAL_ALL(destination->lockCond);
                BOXEDWINE_CONDITION_UNLOCK(destination->lockCond);

                if (!this->blocking) {
                    return -K_EINPROGRESS;
                }
                // :TODO: what about a time out
                
                BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(this->lockCond);
                if (!this->connecting.expired()) {
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
                if (this->connection.expired()) {
                    return -K_ECONNREFUSED;
                }
                this->connected = 1;
                return 0;
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
    if (!this->connection.expired() || !this->connecting.expired()) {
        return -K_EINVAL;
    }
    this->listening = true;
    return 0;
}

U32 KUnixSocketObject::accept(KFileDescriptor* fd, U32 address, U32 len, U32 flags) {
    BOXEDWINE_CONDITION_LOCK(this->lockCond);
    while (!this->pendingConnections.size()) {
        if (!this->blocking) {
            BOXEDWINE_CONDITION_UNLOCK(this->lockCond);
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
    
    std::shared_ptr<KUnixSocketObject> pendingConnection = this->pendingConnections.front().lock();
    this->pendingConnections.pop_front();

    BOXEDWINE_CONDITION_UNLOCK(this->lockCond);

    BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(pendingConnection->lockCond);
    std::shared_ptr<KUnixSocketObject> resultSocket = std::make_shared<KUnixSocketObject>(this->pid, domain, type, protocol);
    KFileDescriptor* result = KThread::currentThread()->process->allocFileDescriptor(resultSocket, K_O_RDWR, 0, -1, 0);

    if (flags & FD_CLOEXEC) {
        result->descriptorFlags|=FD_CLOEXEC;
    }
    if (flags & K_O_NONBLOCK) {
        result->kobject->setBlocking(false);
    }

    pendingConnection->connection = resultSocket;
    //pendingConnection->connected = true; this will be handled when the connecting thread is unblocked    

    resultSocket->connected = true;
    resultSocket->connection = pendingConnection; // weak reference
    
    BOXEDWINE_CONDITION_SIGNAL_ALL(pendingConnection->lockCond);

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
    if (this->connection.expired())
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
        kwarn("shutdown on SOCK_DGRAM not implemented");
        return -1;
    }
    std::shared_ptr<KUnixSocketObject> con = this->connection.lock();
    if (!con) {
        return -K_ENOTCONN;
    }
    if (how == K_SHUT_RD) {
        this->inClosed=true;
        con->outClosed=true;
        BOXEDWINE_CONDITION_SIGNAL_ALL_NEED_LOCK(con->lockCond);
    } else if (how == K_SHUT_WR) {
        this->outClosed=true;
        con->inClosed=true;
        BOXEDWINE_CONDITION_SIGNAL_ALL_NEED_LOCK(con->lockCond);
    } else if (how == K_SHUT_RDWR) {
        this->outClosed=true;
        this->inClosed=true;
        con->outClosed=true;
        con->inClosed=true;
        BOXEDWINE_CONDITION_SIGNAL_ALL_NEED_LOCK(con->lockCond);
    }
    BOXEDWINE_CONDITION_SIGNAL_ALL_NEED_LOCK(this->lockCond);
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
            std::shared_ptr<KUnixSocketObject> con = this->connection.lock();
            if (!con) {
                return -K_EINVAL; // :TODO: is this right
            }
            if (len != 12)
                kpanic("KUnixSocketObject::getsockopt SO_PEERCRED expecting len of 12");
            writed(value, con->pid);
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
    std::shared_ptr<KUnixSocketObject> con = this->connection.lock();

    if (!con) {
        if (this->type == K_SOCK_STREAM) {
            return K_ENOTCONN;
        }
        kpanic("KUnixSocketObject::sendmsg not implemented for type: %d", this->type);
        return 0;
    }
    BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(con->lockCond);
    if (this->outClosed)
        return -K_EPIPE;
    readMsgHdr(address, &hdr);

    std::shared_ptr<KSocketMsg> msg = std::make_shared<KSocketMsg>();

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
    con->msgs.push(msg);
    BOXEDWINE_CONDITION_SIGNAL_ALL(con->lockCond);

    return result;
}

U32 KUnixSocketObject::recvmsg(KFileDescriptor* fd, U32 address, U32 flags) {
    MsgHdr hdr;
    U32 result = 0;

    if (this->domain==K_AF_NETLINK)
        return -K_EIO;
    BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(this->lockCond);
    while (!this->msgs.size()) {
        if (this->recvBuffer.size()) {
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
        if (this->inClosed) {
            return 0;
        }
        if (!this->blocking) {
            return -K_EWOULDBLOCK;
        }
        // :TODO: what about a time out
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

    readMsgHdr(address, &hdr);
    std::shared_ptr<KSocketMsg> msg = this->msgs.front();
    this->msgs.pop();

    if (hdr.msg_control) {
        KThread* thread = KThread::currentThread();
        U32 i=0;

        for (;i<hdr.msg_controllen/16 && i<msg->objects.size();i++) {
            KFileDescriptor* recvFd = thread->process->allocFileDescriptor(msg->objects[i].object, msg->objects[i].accessFlags, 0, -1, 0);
            writeCMsgHdr(hdr.msg_control + i * 16, 16, K_SOL_SOCKET, K_SCM_RIGHTS);
            writed(hdr.msg_control + i * 16 + 12, recvFd->handle);
        }
        writed(address + 20, i * 20);
    }
    U32 pos = 0;
    for (U32 i=0;i<hdr.msg_iovlen;i++) {
        U32 p = readd(hdr.msg_iov + 8 * i);
        U32 len = readd(hdr.msg_iov + 8 * i + 4);
        U32 dataLen = msg->data[pos] | (((U32)msg->data[pos + 1]) << 8) | (((U32)msg->data[pos + 2]) << 16) | (((U32)msg->data[pos + 3]) << 24);
        pos+=4;
        if (len<dataLen) {
            kpanic("unhandled socket msg logic");
        }
        memcopyFromNative(p, msg->data.data() + pos, dataLen);
        pos+=dataLen;
        result+=dataLen;
    }  
    if (!this->connection.expired()) {
        BOXEDWINE_CONDITION_SIGNAL_ALL(this->lockCond);
    }
    return result;
}

U32 KUnixSocketObject::sendto(KFileDescriptor* fd, U32 message, U32 length, U32 flags, U32 dest_addr, U32 dest_len) {
    return 0;
}

U32 KUnixSocketObject::recvfrom(KFileDescriptor* fd, U32 buffer, U32 length, U32 flags, U32 address, U32 address_len) {
    return 0;
}
