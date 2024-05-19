#include "boxedwine.h"

#include "kunixsocket.h"
#include "ksocket.h"
#include "kstat.h"
#include "ksignal.h"

KUnixSocketObject::KUnixSocketObject(U32 domain, U32 type, U32 protocol) : KSocketObject(KTYPE_UNIX_SOCKET, domain, type, protocol), 
    lockCond(std::make_shared<BoxedWineCondition>(B("KUnixSocketObject::lockCond")))
{
}

KUnixSocketObject::~KUnixSocketObject() {    
    if (this->node) {
        std::shared_ptr<FsNode> parent = this->node->getParent().lock();
        if (parent) {
            parent->removeChildByName(this->node->name);
        }
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
    {
        BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(this->lockCond);
        for (auto& weakSocket : this->pendingConnections) {
            std::shared_ptr<KUnixSocketObject> s = weakSocket.lock();
            if (s) {
                s->connecting.reset();
                BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(s->lockCond);
                BOXEDWINE_CONDITION_SIGNAL_ALL(s->lockCond);
            }
        }
        BOXEDWINE_CONDITION_SIGNAL_ALL(this->lockCond);
    }
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
    return nullptr;
}

U32  KUnixSocketObject::setLock(KFileLock* lock, bool wait) {
    kdebug(" UnixSocketObject::setLock not implemented yet");
    return -1;
}

bool KUnixSocketObject::isOpen() {
    return this->listening || !connection.expired();
}

bool KUnixSocketObject::isReadReady() {
    //BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(this->lockCond);
    return this->inClosed || this->recvBuffer.size() || this->pendingConnections.size() || this->msgs.size();
}

bool KUnixSocketObject::isWriteReady() {
    return !connection.expired();
}

void KUnixSocketObject::waitForEvents(BOXEDWINE_CONDITION& parentCondition, U32 events) {
    bool addedLock = false;

    if (events & K_POLLIN) {
        BOXEDWINE_CONDITION_ADD_PARENT(this->lockCond, parentCondition);
        addedLock = true;
    }
    if (events & K_POLLOUT) {
        std::shared_ptr<KUnixSocketObject> con = this->connection.lock();
        if (con) {
            BOXEDWINE_CONDITION_ADD_PARENT(con->lockCond, parentCondition);
        } else {
            if (!addedLock) {
                BOXEDWINE_CONDITION_ADD_PARENT(this->lockCond, parentCondition);
                addedLock = true;
            }
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

U32 KUnixSocketObject::internal_write(KThread* thread, const std::shared_ptr<KUnixSocketObject>& con, BOXEDWINE_CONDITION& cond, U32 buffer, U32 len) {
    KMemory* memory = thread->memory;

    if (!memory->canRead(buffer, len)) {
        return -K_EFAULT;
    }
    if (this->type == K_SOCK_DGRAM) {
        if (!strcmp(this->destAddress.data, "/dev/log")) {
            BString s = memory->readString(buffer);
            printf("%s\n", s.c_str());
        }
        return len;
    }
    if (this->outClosed || !con) {
        return writePipeClosed(thread, false);
    }
    //printf("internal_write: %0.8X size=%d capacity=%d writeLen=%d", (int)&this->connection->recvBuffer, (int)this->connection->recvBuffer.size(), (int)this->connection->recvBuffer.capacity(), len);

    memory->performOnMemory(buffer, len, true, [con](U8* ram, U32 len) {
        con->recvBuffer.insert(con->recvBuffer.end(), ram, ram + len);
        return true;
        });
  
    return len;
}

U32 KUnixSocketObject::writev(KThread* thread, U32 iov, S32 iovcnt) {
    U32 len=0;
    std::shared_ptr<KUnixSocketObject> con = this->connection.lock();
    KMemory* memory = thread->memory;

    BOXEDWINE_CONDITION& cond = (con?con->lockCond:this->lockCond);
    BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(cond);

    for (S32 i=0;i<iovcnt;i++) {
        U32 buf = memory->readd(iov + i * 8);
        U32 toWrite = memory->readd(iov + i * 8 + 4);
        S32 result;
        
        if (toWrite) {
            result = this->internal_write(thread, con, cond, buf, toWrite);
            if (result < 0) {
                if (i > 0) {
                    return len;
                }
                return result;
            }
            len += result;
        }
    }    
    if (con) {
        BOXEDWINE_CONDITION_SIGNAL_ALL(cond);
    }
    return len;
}

U32 KUnixSocketObject::write(KThread* thread, U32 buffer, U32 len) {
    this->pid = thread->process->id; // kind of a hack to do this here
    std::shared_ptr<KUnixSocketObject> con = this->connection.lock();
    BOXEDWINE_CONDITION& cond = (con?con->lockCond:this->lockCond);
    BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(cond);
    U32 result = this->internal_write(thread, con, cond, buffer, len);    
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
    if (this->outClosed || !con) {
        return writePipeClosed(KThread::currentThread(), false);
    }

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
    con = nullptr; // don't hold a strong reference to this, if we are blocking then it would prevent the con object from being destroyed when its process is closed
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

U32 KUnixSocketObject::read(KThread* thread, U32 buffer, U32 len) {
    U32 count = 0;
    this->pid = thread->process->id; // kind of a hack to do this here
    std::shared_ptr<KUnixSocketObject> con = this->connection.lock();
    KMemory* memory = thread->memory;

    if (!memory->canWrite(buffer, len)) {
        return -K_EFAULT;
    }
    if (!this->inClosed && !con)
        return -K_EPIPE;
    con = nullptr; // don't hold a strong reference to this, if we are blocking then it would prevent the con object from being destroyed when its process is closed
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

    if (con) {
        BOXEDWINE_CONDITION_SIGNAL_ALL(this->lockCond);
    }

    return count;
}

U32 KUnixSocketObject::stat(KProcess* process, U32 address, bool is64) {
    KSystem::writeStat(process, B(""), address, is64, true, (this->node?this->node->id:0), K_S_IFSOCK|K__S_IWRITE|K__S_IREAD, (this->node?this->node->rdev:0), 0, 4096, 0, this->lastModifiedTime, 1);
    return 0;
}

U32 KUnixSocketObject::map(KThread* thread, U32 address, U32 len, S32 prot, S32 flags, U64 off) {
    return 0;
}

bool KUnixSocketObject::canMap() {
    return false;
}

BString KUnixSocketObject::selfFd() {
    return B("anon_inode:[pipe]");
}

S64 KUnixSocketObject::seek(S64 pos) {
    return -K_ESPIPE;
}

S64 KUnixSocketObject::getPos() {
    return 0;
}

U32 KUnixSocketObject::ioctl(KThread* thread, U32 request) {
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
    UnixSocketNode(U32 id, U32 rdev, BString path, std::shared_ptr<FsNode> parent) : FsNode(Type::Socket, id, rdev, path, B(""), B(""), false, parent) {}
    U32 rename(BString path) override {return -K_EIO;}
    bool remove() override {if (!this->parent.lock()) return false; this->removeNodeFromParent(); return true;}
    U64 lastModified() override {return 0;}
    U64 length() override {return 0;}
    FsOpenNode* open(U32 flags) override {kwarn("unixsocket_open was called, this shouldn't happen.  syscall_open should detect we have a kobject already"); return nullptr;}
    U32 getType(bool checkForLink) override {return 12;} // DT_SOCK
    U32 getMode() override {return K__S_IREAD | K__S_IWRITE | K_S_IFSOCK;}
    U32 removeDir() override {kpanic("UnixSocket::removeDir not implemented"); return 0;}
    U32 setTimes(U64 lastAccessTime, U32 lastAccessTimeNano, U64 lastModifiedTime, U32 lastModifiedTimeNano) override {klog("UnixSocket::setTimes not implemented"); return 0;}
};

U32 KUnixSocketObject::bind(KThread* thread, KFileDescriptor* fd, U32 address, U32 len) {
    KMemory* memory = thread->memory;

    U32 family = memory->readw(address);
    if (family==K_AF_UNIX) {
        BString name = socketAddressName(memory, address, len);

        if (name.length() == 0) {
            return 0; // :TODO: why does XOrg need this
        }
        std::shared_ptr<FsNode> node = Fs::getNodeFromLocalPath(thread->process->currentDirectory, name, true);
        if (node) {
            return -K_EADDRINUSE;
        }        
        BString fullpath = Fs::getFullPath(thread->process->currentDirectory, name);
        std::shared_ptr<FsNode> parentNode = Fs::getNodeFromLocalPath(B(""), Fs::getParentPath(fullpath), true);
        std::shared_ptr<UnixSocketNode> socketNode = std::make_shared<UnixSocketNode>(0, 2, fullpath, parentNode);
        parentNode->addChild(socketNode);
        std::shared_ptr<KUnixSocketObject> s = std::dynamic_pointer_cast<KUnixSocketObject>(fd->kobject);
        socketNode->kobject = fd->kobject;
        s->node = socketNode;
        return 0;
    }
    return -K_EAFNOSUPPORT;
}

U32 KUnixSocketObject::connect(KThread* thread, KFileDescriptor* fd, U32 address, U32 len) {
    KMemory* memory = thread->memory;

    this->pid = thread->process->id;
    if (len-2>sizeof(this->destAddress.data)) {
        kpanic("Socket address is too big");
    }
    this->destAddress.family = memory->readw(address);
    memory->memcpy(this->destAddress.data, address + 2, len - 2);
    if (this->type==K_SOCK_DGRAM) {
        this->connected = 1;		
        return 0;
    } else if (this->type==K_SOCK_STREAM) {
        if (this->destAddress.data[0]==0) {
            // :TODO: why
            memory->memcpy(this->destAddress.data, address + 3, len - 3);
        }
        if (this->destAddress.data[0] == 0) {
            return -K_ENOENT;
        }
        if (this->domain==K_AF_UNIX) {
            std::shared_ptr<FsNode> node = Fs::getNodeFromLocalPath(thread->process->currentDirectory, BString::copy(this->destAddress.data), true);
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
            con = nullptr; // don't hold a strong reference to this, if we are blocking then it would prevent the con object from being destroyed when its process is closed
            if (!this->connecting.expired()) {
                if (this->blocking) {
                    BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(this->lockCond);
                    while (!this->connecting.expired()) {
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
                {
                    BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(destination->lockCond);
                    std::shared_ptr< KUnixSocketObject> t = std::dynamic_pointer_cast<KUnixSocketObject>(shared_from_this());
                    destination->pendingConnections.push_back(t);
                    BOXEDWINE_CONDITION_SIGNAL_ALL(destination->lockCond);
                }

                if (!this->blocking) {
                    return -K_EINPROGRESS;
                }
                // :TODO: what about a time out
                
                BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(this->lockCond);
                if (!this->connecting.expired()) {
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
        kwarn("connect not implemented for type %d", this->type);
        return -K_ECONNREFUSED;
    }
    // should never get here
    return 0;
}

U32 KUnixSocketObject::listen(KThread* thread, KFileDescriptor* fd, U32 backlog) {
    if (!this->node) {
        return -K_EDESTADDRREQ;
    }
    if (!this->connection.expired() || !this->connecting.expired()) {
        return -K_EINVAL;
    }
    this->listening = true;
    return 0;
}

U32 KUnixSocketObject::accept(KThread* thread, KFileDescriptor* fd, U32 address, U32 len, U32 flags) {
    std::shared_ptr<KUnixSocketObject> pendingConnection;
    {
        BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(this->lockCond);
        while (!this->pendingConnections.size()) {
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

        pendingConnection = this->pendingConnections.front().lock();
        this->pendingConnections.pop_front();
    }
    

    BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(pendingConnection->lockCond);
    std::shared_ptr<KUnixSocketObject> resultSocket = std::make_shared<KUnixSocketObject>(domain, type, protocol);
    KFileDescriptor* result = thread->process->allocFileDescriptor(resultSocket, K_O_RDWR, 0, -1, 0);

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

U32 KUnixSocketObject::getsockname(KThread* thread, KFileDescriptor* fd, U32 address, U32 plen) {
    KMemory* memory = thread->memory;

    U32 len = memory->readd( plen);
    if (this->domain == K_AF_UNIX) {
        memory->writew(address, this->destAddress.family);
        len-=2;
        if (len>sizeof(this->destAddress.data))
            len = sizeof(this->destAddress.data);
        memory->memcpy(address + 2, this->destAddress.data, len);
        memory->writed(plen, 2 + (U32)strlen(this->destAddress.data) + 1);
        return 0;
    }
    kwarn("KUnixSocketObject::getsockname not implemented for domain %d", this->domain);
    return 0;
}

U32 KUnixSocketObject::getpeername(KThread* thread, KFileDescriptor* fd, U32 address, U32 plen) {
    KMemory* memory = thread->memory;

    if (this->connection.expired())
        return -K_ENOTCONN;
    U32 len = memory->readd( plen);
    memory->writew(address, this->destAddress.family);
    len-=2;
    if (len>sizeof(this->destAddress.data))
        len = sizeof(this->destAddress.data);
    memory->memcpy(address + 2, this->destAddress.data, len);
    memory->writed(plen, 2 + (U32)strlen(this->destAddress.data) + 1);
    return 0;
}

U32 KUnixSocketObject::shutdown(KThread* thread, KFileDescriptor* fd, U32 how) {
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
        BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(con->lockCond);
        BOXEDWINE_CONDITION_SIGNAL_ALL(con->lockCond);
    } else if (how == K_SHUT_WR) {
        this->outClosed=true;
        con->inClosed=true;
        BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(con->lockCond);
        BOXEDWINE_CONDITION_SIGNAL_ALL(con->lockCond);
    } else if (how == K_SHUT_RDWR) {
        this->outClosed=true;
        this->inClosed=true;
        con->outClosed=true;
        con->inClosed=true;
        BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(con->lockCond);
        BOXEDWINE_CONDITION_SIGNAL_ALL(con->lockCond);
    }
    BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(this->lockCond);
    BOXEDWINE_CONDITION_SIGNAL_ALL(this->lockCond);
    return 0;
}

U32 KUnixSocketObject::setsockopt(KThread* thread, KFileDescriptor* fd, U32 level, U32 name, U32 value, U32 len) {
    KMemory* memory = thread->memory;

    if (level == K_SOL_SOCKET) {
        switch (name) {
            case K_SO_RCVBUFFORCE:
            case K_SO_RCVBUF:
                if (len!=4)
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

U32 KUnixSocketObject::getsockopt(KThread* thread, KFileDescriptor* fd, U32 level, U32 name, U32 value, U32 len_address) {
    KMemory* memory = thread->memory;

    U32 len = memory->readd(len_address);
    if (level == K_SOL_SOCKET) {
        if (name == K_SO_RCVBUF) {
            if (len!=4)
                kpanic("KUnixSocketObject::getsockopt SO_RCVBUF expecting len of 4");
            memory->writed(value, this->recvLen);
        } else if (name == K_SO_SNDBUF) {
            if (len != 4)
                kpanic("KUnixSocketObject::getsockopt SO_SNDBUF expecting len of 4");
            memory->writed(value, this->sendLen);
        } else if (name == K_SO_ERROR) {
            if (len != 4)
                kpanic("KUnixSocketObject::getsockopt SO_ERROR expecting len of 4");
            memory->writed(value, this->error);
        } else if (name == K_SO_TYPE) { 
            if (len != 4)
                kpanic("KUnixSocketObject::getsockopt K_SO_TYPE expecting len of 4");
            memory->writed(value, this->type);
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
            memory->writed(value, con->pid);
            memory->writed(value + 4, thread->process->userId);
            memory->writed(value + 8, thread->process->groupId);
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

U32 KUnixSocketObject::writePipeClosed(KThread* thread, bool noSignal) {
    if (noSignal || thread->process->sigActions[K_SIGPIPE].handlerAndSigAction == K_SIG_IGN || !thread->readyForSignal(K_SIGPIPE)) {
        return -K_EPIPE;
    }
    thread->runSignal(K_SIGPIPE, 0, 0);
    return -K_CONTINUE;
}

U32 KUnixSocketObject::sendmsg(KThread* thread, KFileDescriptor* fd, U32 address, U32 flags) {
    MsgHdr hdr = {};
    KMemory* memory = thread->memory;
    U32 result = 0;
    std::shared_ptr<KUnixSocketObject> con = this->connection.lock();

    if (!con) {
        if (this->type == K_SOCK_STREAM) {
            return K_ENOTCONN;
        }
        kpanic("KUnixSocketObject::sendmsg not implemented for type: %d", this->type);
        return 0;
    }
    bool noSignal = (flags & K_MSG_NOSIGNAL) != 0;
    flags &= ~K_MSG_NOSIGNAL;
    if (flags) {
        kwarn("KUnixSocketObject::sendmsg unhandled flag=%x", flags);
    }
    BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(con->lockCond);
    if (this->outClosed) {
        return writePipeClosed(thread, noSignal);
    }
    readMsgHdr(thread, address, &hdr);

    std::shared_ptr<KSocketMsg> msg = std::make_shared<KSocketMsg>();

    if (hdr.msg_control) {
        CMsgHdr cmsg;			

        readCMsgHdr(thread, hdr.msg_control, &cmsg);
        if (cmsg.cmsg_level != K_SOL_SOCKET) {
            kpanic("KUnixSocketObject::sendmsg control level %d not implemented", cmsg.cmsg_level);
        } else if (cmsg.cmsg_type != K_SCM_RIGHTS) {
            kpanic("KUnixSocketObject::sendmsg control type %d not implemented", cmsg.cmsg_level);
        } else if ((cmsg.cmsg_len & 3) != 0) {
            kpanic("KUnixSocketObject::sendmsg control len %d not implemented", cmsg.cmsg_len);
        }

        for (U32 i=0;i<hdr.msg_controllen/16;i++) {
            KFileDescriptor* f = thread->process->getFileDescriptor(memory->readd(hdr.msg_control + 16 * i + 12));
            if (!f) {
                return -K_EBADF;
            } else {
                KSocketMsgObject d;
                d.object = f->kobject;
                d.accessFlags = f->accessFlags;
                msg->objects.push_back(d);
            }
        }				
    }
    for (U32 i=0;i<hdr.msg_iovlen;i++) {
        U32 p = memory->readd(hdr.msg_iov + 8 * i);
        U32 len = memory->readd(hdr.msg_iov + 8 * i + 4);

        msg->data.push_back((U8)len);
        msg->data.push_back((U8)(len >> 8));
        msg->data.push_back((U8)(len >> 16));
        msg->data.push_back((U8)(len >> 24));
        while (len) {
            msg->data.push_back(memory->readb(p++));
            len--;
            result++;
        }
    }
    con->msgs.push(msg);
    BOXEDWINE_CONDITION_SIGNAL_ALL(con->lockCond);

    return result;
}

U32 KUnixSocketObject::recvmsg(KThread* thread, KFileDescriptor* fd, U32 address, U32 flags) {
    MsgHdr hdr = {};
    U32 result = 0;
    KMemory* memory = thread->memory;

    BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(this->lockCond);
    while (!this->msgs.size()) {
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
            if (this->type==K_SOCK_STREAM)
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

    readMsgHdr(thread, address, &hdr);
    std::shared_ptr<KSocketMsg> msg = this->msgs.front();
    this->msgs.pop();

    if (hdr.msg_control) {
        U32 i=0;

        for (;i<hdr.msg_controllen/16 && i<msg->objects.size();i++) {
            KFileDescriptor* recvFd = thread->process->allocFileDescriptor(msg->objects[i].object, msg->objects[i].accessFlags, 0, -1, 0);
            writeCMsgHdr(thread, hdr.msg_control + i * 16, 16, K_SOL_SOCKET, K_SCM_RIGHTS);
            memory->writed(hdr.msg_control + i * 16 + 12, recvFd->handle);
        }
        memory->writed(address + 20, i * 20);
    }
    U32 pos = 0;
    for (U32 i=0;i<hdr.msg_iovlen;i++) {
        U32 p = memory->readd(hdr.msg_iov + 8 * i);
        U32 len = memory->readd(hdr.msg_iov + 8 * i + 4);
        U32 dataLen = msg->data[pos] | (((U32)msg->data[pos + 1]) << 8) | (((U32)msg->data[pos + 2]) << 16) | (((U32)msg->data[pos + 3]) << 24);
        pos+=4;
        if (len<dataLen) {
            kpanic("unhandled socket msg logic");
        }
        memory->memcpy(p, msg->data.data() + pos, dataLen);
        pos+=dataLen;
        result+=dataLen;
    }  
    if (!this->connection.expired()) {
        BOXEDWINE_CONDITION_SIGNAL_ALL(this->lockCond);
    }
    return result;
}

U32 KUnixSocketObject::sendto(KThread* thread, KFileDescriptor* fd, U32 message, U32 length, U32 flags, U32 dest_addr, U32 dest_len) {
    return 0;
}

U32 KUnixSocketObject::recvfrom(KThread* thread, KFileDescriptor* fd, U32 buffer, U32 length, U32 flags, U32 address, U32 address_len) {
    if (address == 0) {
        if (flags) {
            kpanic("KUnixSocketObject::recvfrom unhandled flags=%x", flags);
        }
        return read(thread, buffer, length);
    }
    return 0;
}
