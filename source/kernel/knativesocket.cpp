#include "boxedwine.h"

#include "knativesocket.h"
#include "ksocket.h"
#include "kstat.h"
#include "bufferaccess.h"

#ifdef WIN32
#undef BOOL
#include <winsock2.h>
#include <ws2tcpip.h>
static int winsock_intialized;
#define BOOL unsigned int
#pragma comment(lib, "Ws2_32.lib")
#undef min
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>
void closesocket(int socket) { close(socket); }

#if !defined(TCP_KEEPIDLE) && defined(TCP_KEEPALIVE)
#define TCP_KEEPIDLE TCP_KEEPALIVE
#endif

#endif

std::vector<std::shared_ptr<KNativeSocketObject>> waitingNativeSockets;
fd_set waitingReadset;
fd_set waitingWriteset;
fd_set waitingErrorset;
int maxSocketId;

#ifdef BOXEDWINE_MULTI_THREADED
#include "knativethread.h"
static KNativeThread* checkWaitingNativeSocketsThread;
static BOXEDWINE_MUTEX checkWaitingNativeSocketsThreadMutex;
static BOXEDWINE_MUTEX waitingNodeMutex;
static bool checkWaitingNativeSocketsThreadDone;
static S32 nativeSocketPipe[2];
#endif

void updateWaitingList() {
    FD_ZERO(&waitingReadset);
    FD_ZERO(&waitingWriteset);
    FD_ZERO(&waitingErrorset);
    
#ifdef BOXEDWINE_MULTI_THREADED
    maxSocketId = nativeSocketPipe[0];
    FD_SET(nativeSocketPipe[0], &waitingReadset);
#else
    maxSocketId = 0;
#endif
    for (auto& s : waitingNativeSockets) {
        bool errorSet = false;
#ifndef BOXEDWINE_MSVC
        if (s->nativeSocket>=FD_SETSIZE) {
            kpanic("updateWaitingList %s socket is too large to select on", s->nativeSocket);
        }
#endif
        if (s->readingCond.parent) {
            FD_SET(s->nativeSocket, &waitingReadset);
            FD_SET(s->nativeSocket, &waitingErrorset);
            errorSet = true;
        }
        if (s->writingCond.parent) {
            FD_SET(s->nativeSocket, &waitingWriteset);
            if (!errorSet)
                FD_SET(s->nativeSocket, &waitingErrorset);
        }
        if (s->nativeSocket>maxSocketId)
            maxSocketId = s->nativeSocket;
    }
}

bool checkWaitingNativeSockets(int timeout) {
    struct timeval t;
    t.tv_sec = 0;
    t.tv_usec = timeout*1000;

#ifdef BOXEDWINE_MULTI_THREADED
    {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(waitingNodeMutex);
        updateWaitingList();
    }
    {
#else
    if (waitingNativeSockets.size()) {
        updateWaitingList();
        if (maxSocketId==0)
            return false;
#endif           
        int result = select(maxSocketId + 1, &waitingReadset, &waitingWriteset, &waitingErrorset, (timeout>=0?&t: nullptr));
        if (result) {
#ifdef BOXEDWINE_MULTI_THREADED
            if (FD_ISSET(nativeSocketPipe[0], &waitingReadset)) {
                char buf = 0;
                ::recv(nativeSocketPipe[0], &buf, 1, 0);
                return true;
            }
#endif
            BOXEDWINE_CONDITION* conditions[1024] = { };
            U32 conditionCount = 0;
            {
                BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(waitingNodeMutex);                

                for (auto& s : waitingNativeSockets) {
                    if (FD_ISSET(s->nativeSocket, &waitingReadset) && s->readingCond.parent) {
                        if (conditionCount<1024) conditions[conditionCount++]=&s->readingCond;
                    }
                    if (FD_ISSET(s->nativeSocket, &waitingWriteset) && s->writingCond.parent) {                    
                        if (conditionCount<1024) conditions[conditionCount++]=&s->writingCond;
                    }
                    if (FD_ISSET(s->nativeSocket, &waitingErrorset)) {
                        if (conditionCount<1024) conditions[conditionCount++]=&s->readingCond;
                        if (conditionCount<1024) conditions[conditionCount++]=&s->writingCond;
                    }
                }
            }
            for (U32 i=0;i<conditionCount;i++) {
                BOXEDWINE_CONDITION& c = *conditions[i];
                BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(c);
                BOXEDWINE_CONDITION_SIGNAL_ALL(c);
            }
        }
        return true;
    }
    return false;
}

void setNativeBlocking(int nativeSocket, bool blocking) {
#ifdef WIN32
    u_long mode = blocking?0:1;
    ioctlsocket(nativeSocket, FIONBIO, &mode);           
#else
    if (blocking)
        fcntl(nativeSocket, F_SETFL, fcntl(nativeSocket, F_GETFL, 0) & ~O_NONBLOCK);
    else
        fcntl(nativeSocket, F_SETFL, fcntl(nativeSocket, F_GETFL, 0) | O_NONBLOCK);
#endif
}

#ifdef BOXEDWINE_MULTI_THREADED
static int checkWaitingNativeSockets_thread(void *ptr) {
    while(!checkWaitingNativeSocketsThreadDone) {
        checkWaitingNativeSockets(1000);
    }
    return 0;
}

void startNativeSocketsThread() {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(checkWaitingNativeSocketsThreadMutex);
    if (!checkWaitingNativeSocketsThread) {
        Platform::nativeSocketPair(nativeSocketPipe);
        setNativeBlocking(nativeSocketPipe[0], false);
        setNativeBlocking(nativeSocketPipe[1], false);
        checkWaitingNativeSocketsThread = KNativeThread::createAndStartThread(checkWaitingNativeSockets_thread, B("NativeSockeThread"), (void *)nullptr);
    }    
}

void stopNativeSocketsThread() {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(checkWaitingNativeSocketsThreadMutex);
    checkWaitingNativeSocketsThreadDone = true;
    char buf = 0;
    ::send(nativeSocketPipe[1], &buf, 1, 0);
    checkWaitingNativeSocketsThread->wait();
    checkWaitingNativeSocketsThreadDone = false;
    checkWaitingNativeSocketsThread = nullptr;
}
#endif

void addWaitingNativeSocket(const std::shared_ptr<KNativeSocketObject>& s) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(waitingNodeMutex);
    for (auto& waitingSocket : waitingNativeSockets) {
        if (waitingSocket->nativeSocket == s->nativeSocket) {
            return;
        }
    }

    waitingNativeSockets.push_back(s);
#ifdef BOXEDWINE_MULTI_THREADED
    startNativeSocketsThread();
    char buf = 0;
    ::send(nativeSocketPipe[1], &buf, 1, 0);
#endif
}

void removeWaitingSocket(S32 nativeSocket) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(waitingNodeMutex);
    for (int i = 0; i < (int)waitingNativeSockets.size(); i++) {
        if (waitingNativeSockets[i]->nativeSocket == nativeSocket) {
            waitingNativeSockets.erase(waitingNativeSockets.begin() + i);
            break;
        }
    }
#ifdef BOXEDWINE_MULTI_THREADED
    char buf = 0;
    ::send(nativeSocketPipe[1], &buf, 1, 0);
#endif
}

S32 translateNativeSocketError(int error) {
    S32 result = 0;
#ifdef WIN32
    if (error == WSAENOTCONN)
        result = -K_ENOTCONN;
    else if (error == WSAEWOULDBLOCK)
        result = -K_EWOULDBLOCK;
    else if (error == WSAETIMEDOUT)
        result = -K_ETIMEDOUT;
    else if (error == WSAECONNRESET)
        result = -K_ECONNRESET;
    else if (error == WSAEDESTADDRREQ)
        result = -K_EDESTADDRREQ;
    else if (error == WSAEHOSTUNREACH)
        result = -K_EHOSTUNREACH;
     else if (error == WSAECONNREFUSED)
        result = -K_ECONNREFUSED;
    else if (error == WSAEISCONN)
        result = -K_EISCONN;
    else if (error == WSAEMSGSIZE)
        result = -K_EMSGSIZE;
    else
        result =-K_EIO;

#else 
    if (error == ENOTCONN)
        result = -K_ENOTCONN;
    else if (error == EWOULDBLOCK)
        result = -K_EWOULDBLOCK;
    else if (error == ETIMEDOUT)
        result = -K_ETIMEDOUT;
    else if (error == ECONNRESET)
        result = -K_ECONNRESET;
    else if (error == EDESTADDRREQ)
        result = -K_EDESTADDRREQ;
    else if (error == EHOSTUNREACH)
        result = -K_EHOSTUNREACH;
    else if (error == EISCONN)
        result = -K_EISCONN;
    else if (error == ECONNREFUSED)
        result = -K_ECONNREFUSED;
    else if (error == EMSGSIZE)
        result = -K_EMSGSIZE;
    else
        result = -K_EIO;
#endif
    return result;
}

S32 handleNativeSocketError(const std::shared_ptr<KNativeSocketObject>& s, bool write) {
#ifdef WIN32
    S32 result = translateNativeSocketError(WSAGetLastError());
#else
    S32 result = translateNativeSocketError(errno);
#endif
    s->error = -result;
#ifndef BOXEDWINE_MULTI_THREADED
    if (result == -K_EWOULDBLOCK) {
        addWaitingNativeSocket(s);        
        if (write) {
            BOXEDWINE_CONDITION_LOCK(s->writingCond);
            BOXEDWINE_CONDITION_WAIT(s->writingCond);
            BOXEDWINE_CONDITION_UNLOCK(s->writingCond);            
        } else {
            BOXEDWINE_CONDITION_LOCK(s->readingCond);
            BOXEDWINE_CONDITION_WAIT(s->readingCond);
            BOXEDWINE_CONDITION_UNLOCK(s->readingCond);
        }        
        result = -K_WAIT;
    }    
#endif
    return result;
}

KNativeSocketObject::KNativeSocketObject(U32 domain, U32 type, U32 protocol) : KSocketObject(KTYPE_NATIVE_SOCKET, domain, type, protocol), 
    connecting(false),
    readingCond(B("KNativeSocketObject::readingCond")),
    writingCond(B("KNativeSocketObject::writingCond")) {
#ifdef WIN32
    if (!winsock_intialized) {
        WSADATA wsaData;
        static_cast<void>(WSAStartup(0x0202, &wsaData));
        winsock_intialized=1;
    }
#endif
    U32 nativeType = 0;
    U32 nativeProtocol = 0;

    this->nativeSocket = 0;

    if (type == K_SOCK_STREAM) {
        nativeType = SOCK_STREAM;
    } else if (type == K_SOCK_DGRAM) {
        nativeType = SOCK_DGRAM;
    } else if (type == K_SOCK_RAW) {
        nativeType = SOCK_RAW;
    } else if (type == K_SOCK_RDM) {
        nativeType = SOCK_RDM;
    } else if (type == K_SOCK_SEQPACKET) {
        nativeType = SOCK_SEQPACKET;
    } else {
        this->error = K_EPROTOTYPE;
        return;
    }
    if (protocol == 0) {
        nativeProtocol = IPPROTO_IP ;
    } else if (protocol == 1) {
        nativeProtocol = IPPROTO_ICMP;
    } else if (protocol == 2) {
        nativeProtocol = IPPROTO_IGMP;
    } else if (protocol == 6) {
        nativeProtocol = IPPROTO_TCP;
    } else if (protocol == 12) {
        nativeProtocol = IPPROTO_PUP;
    } else if (protocol == 17) {
        nativeProtocol = IPPROTO_UDP;
    } else if (protocol == 22) {
        nativeProtocol = IPPROTO_IDP;
    } else if (protocol == 255) {
        nativeProtocol = IPPROTO_RAW;
    } else {
        this->error = -K_EPROTOTYPE;
        nativeProtocol = IPPROTO_IP;
    }
    this->nativeSocket = (S32)socket(AF_INET, nativeType, nativeProtocol);
#ifndef BOXEDWINE_MULTI_THREADED
    setNativeBlocking(this->nativeSocket, false);
#endif
}

KNativeSocketObject::~KNativeSocketObject() {
    closesocket(this->nativeSocket);    
    removeWaitingSocket(this->nativeSocket);
    this->nativeSocket = 0;
    {
        BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(this->readingCond);
        BOXEDWINE_CONDITION_SIGNAL_ALL(this->readingCond);
    }
    {
        BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(this->writingCond);
        BOXEDWINE_CONDITION_SIGNAL_ALL(this->writingCond);
    }
}

U32 KNativeSocketObject::ioctl(KThread* thread, U32 request) {
    if (request == 0x541b) {        
#ifdef WIN32
        u_long value=0;
        U32 result = ioctlsocket(this->nativeSocket, FIONREAD, &value);
#else        
        int value=0;
        U32 result = ::ioctl(this->nativeSocket, FIONREAD, &value);        
#endif
        if (result != 0) {
            std::shared_ptr< KNativeSocketObject> t = std::dynamic_pointer_cast<KNativeSocketObject>(shared_from_this());
            return handleNativeSocketError(t, true);
        }
        return value;
    }
    return -K_ENOTTY;
}

S64 KNativeSocketObject::seek(S64 pos) {
    return -K_ESPIPE;
}

S64 KNativeSocketObject::length() {
    return -1;
}

S64 KNativeSocketObject::getPos() {
    return 0;
}

void KNativeSocketObject::setBlocking(bool blocking) {
    this->blocking = blocking;
#ifdef BOXEDWINE_MULTI_THREADED
    setNativeBlocking(this->nativeSocket, blocking);
#endif
}

bool KNativeSocketObject::isBlocking() {
    return this->blocking;
}

void KNativeSocketObject::setAsync(bool isAsync) {
    if (isAsync)
        kpanic("KNativeSocketObject::setAsync not implemented yet");
}

bool KNativeSocketObject::isAsync() {
    return false;
}

KFileLock* KNativeSocketObject::getLock(KFileLock* lock) {
    kdebug("KNativeSocketObject::getLock not implemented yet");
    return nullptr;
}

U32 KNativeSocketObject::setLock(KFileLock* lock, bool wait) {
    kdebug("KNativeSocketObject::setLock not implemented yet");
    return -1;
}

bool KNativeSocketObject::supportsLocks() {
    return false;
}

bool KNativeSocketObject::isOpen() {
    return this->listening || this->connected;
}

bool KNativeSocketObject::isPriorityReadReady() {
    fd_set          sready = {};
    struct timeval  nowait = { 0 };

    FD_ZERO(&sready);
    FD_SET(this->nativeSocket, &sready);

    ::select(this->nativeSocket + 1, nullptr, nullptr, &sready, &nowait);
    bool result = FD_ISSET(this->nativeSocket, &sready) != 0;
    if (result) {
        this->error = 0;
    }
    return result;
}

bool KNativeSocketObject::isReadReady() {
    fd_set          sready = {};
    struct timeval  nowait = { 0 };

    FD_ZERO(&sready);
    FD_SET(this->nativeSocket, &sready);

    ::select(this->nativeSocket + 1, &sready, nullptr, nullptr, &nowait);
    bool result = FD_ISSET(this->nativeSocket, &sready) != 0;
    if (result) {
        this->error = 0;
    }
    return result;
}

bool KNativeSocketObject::isWriteReady() {
    fd_set          sready = {};
    struct timeval  nowait = { 0 };

    FD_ZERO(&sready);
    FD_SET(this->nativeSocket, &sready);

    ::select(this->nativeSocket + 1, nullptr, &sready, nullptr, &nowait);
    bool result = FD_ISSET(this->nativeSocket, &sready) != 0;
    if (result) {
        this->error = 0;
    }
    return result;
}

void KNativeSocketObject::waitForEvents(BOXEDWINE_CONDITION& parentCondition, U32 events) {
    if (events & K_POLLIN) {
        BOXEDWINE_CONDITION_SET_PARENT(this->readingCond, &parentCondition);
    } else {
        BOXEDWINE_CONDITION_SET_PARENT(this->readingCond, nullptr);
    }
    if (events & K_POLLOUT) {
        BOXEDWINE_CONDITION_SET_PARENT(this->writingCond, &parentCondition);
    } else {
        BOXEDWINE_CONDITION_SET_PARENT(this->writingCond, nullptr);
    }
    if (events == 0) {
        removeWaitingSocket(this->nativeSocket);
    } else {
        std::shared_ptr< KNativeSocketObject> t = std::dynamic_pointer_cast<KNativeSocketObject>(shared_from_this());
        addWaitingNativeSocket(t);
    }
}

U32 KNativeSocketObject::writeNative(U8* buffer, U32 len) {
    S32 result = (S32)::send(this->nativeSocket, (const char*)buffer, len, this->flags);
    if (result>=0) {            
        this->error = 0;
        return result;
    }
    std::shared_ptr< KNativeSocketObject> t = std::dynamic_pointer_cast<KNativeSocketObject>(shared_from_this());
    return handleNativeSocketError(t, true);
}

U32 KNativeSocketObject::readNative(U8* buffer, U32 len) {
    S32 result = (S32)::recv(this->nativeSocket, (char*)buffer, len, this->flags);
    if (result>=0) {  
        this->error = 0;
        return result;
    }
    std::shared_ptr< KNativeSocketObject> t = std::dynamic_pointer_cast<KNativeSocketObject>(shared_from_this());
    return handleNativeSocketError(t, false);
}

U32 KNativeSocketObject::stat(KProcess* process, U32 address, bool is64) {
    KSystem::writeStat(process, B(""), address, is64, 1, 0, K_S_IFSOCK|K__S_IWRITE|K__S_IREAD, 0, 0, 4096, 0, 0, 1);
    return 0;
}

U32 KNativeSocketObject::map(KThread* thread, U32 address, U32 len, S32 prot, S32 flags, U64 off) {
    return 0;
}

bool KNativeSocketObject::canMap() {
    return false;
}

U32 KNativeSocketObject::bind(KThread* thread, KFileDescriptor* fd, U32 address, U32 len) {
    KMemory* memory = thread->memory;

    U32 family = memory->readw(address);
    if (family == K_AF_INET) {
        if (fd->kobject->type!=KTYPE_NATIVE_SOCKET) {
            return -K_ENOTSOCK;
        }
        char tmp[4096] = {};
        if (len>sizeof(tmp)) {
            kpanic("kbind: buffer not large enough: len=%d", len);
        }
        memory->memcpy(tmp, address, len);
        if (::bind(this->nativeSocket, (struct sockaddr*)tmp, len)==0) {
            this->error = 0;
            return 0;
        }
        std::shared_ptr< KNativeSocketObject> t = std::dynamic_pointer_cast<KNativeSocketObject>(shared_from_this());
        return handleNativeSocketError(t, false);
    }
    return -K_EAFNOSUPPORT;
}

U32 KNativeSocketObject::connect(KThread* thread, KFileDescriptor* fd, U32 address, U32 len) {
    char buffer[1024] = {};
    U32 result = 0;
    KMemory* memory = thread->memory;

#ifndef BOXEDWINE_MULTI_THREADED
    if (this->connecting) {
        if (this->isWriteReady()) {
            this->error = 0;
            this->connecting = 0;
            this->connected = true;
            removeWaitingSocket(this->nativeSocket);
            return 0;
        }
        else {
            int error = 0;
            socklen_t errLen = 4;
            if (::getsockopt(this->nativeSocket, SOL_SOCKET, SO_ERROR, (char*)&error, &errLen) < 0) {
                return -K_EIO;
            }
            if (error) {
                result = translateNativeSocketError(error);
                if (result != (U32)(-K_EWOULDBLOCK)) {
                    return result;
                }
            }
        }
        std::shared_ptr< KNativeSocketObject> t = std::dynamic_pointer_cast<KNativeSocketObject>(shared_from_this());
        addWaitingNativeSocket(t);
        BOXEDWINE_CONDITION_LOCK(this->writingCond);
        BOXEDWINE_CONDITION_WAIT(this->writingCond);
        BOXEDWINE_CONDITION_UNLOCK(this->writingCond);
    }
#endif
    memory->memcpy(buffer, address, len);
#ifdef BOXEDWINE_MULTI_THREADED
    bool setBlocking = false;
    /*
    if (!this->blocking) {
        // :TODO: why is this necessary?
        setBlocking = true;
        setNativeBlocking(this->nativeSocket, true);
    }
    */
#endif
    U16 family = memory->readw(address);
    if (family == K_AF_INET) {
        if (memory->readb(address + 4) == 127 && memory->readb(address + 5) == 0 && memory->readb(address + 6) == 0 && memory->readb(address + 7) == 1) {
            U16 port = memory->readb(address + 3) | (((U32)memory->readb(address + 2)) << 8);
            if (port == 631) {
                // wine seems to try and connect to a print server, if we are running on Linux it hangs while trying to read.  That is probably a bug, for now just prevent the connection.
                return -K_ECONNREFUSED;
            }
        }
    }

    if (::connect(this->nativeSocket, (struct sockaddr*)buffer, len) == 0) {
        int error = 0;
        socklen_t optLen = 4;
        ::getsockopt(this->nativeSocket, SOL_SOCKET, SO_ERROR, (char*)&error, &optLen);
#ifdef BOXEDWINE_MULTI_THREADED
        if (setBlocking) {
            setNativeBlocking(this->nativeSocket, false);
        }
#endif
        if (error == 0) {
            this->connected = true;
            return 0;
        }
        this->connecting = false;
        return -K_ECONNREFUSED;
    }
    std::shared_ptr< KNativeSocketObject> t = std::dynamic_pointer_cast<KNativeSocketObject>(shared_from_this());
    result = handleNativeSocketError(t, true);
#ifdef BOXEDWINE_MSVC
    if (result == -K_EWOULDBLOCK) {
        result = -K_EINPROGRESS;
        t->error = K_EINPROGRESS;
    }
#endif
    if (result == -K_EINPROGRESS) {
        this->connected = true;
    }
#ifdef BOXEDWINE_MULTI_THREADED
        if (setBlocking) {
            setNativeBlocking(this->nativeSocket, false);
        }
#else
    if (result == (U32)(-K_WAIT)) {            
        this->connecting = true;
    }
#endif
    return result;
}

U32 KNativeSocketObject::listen(KThread* thread, KFileDescriptor* fd, U32 backlog) {
    S32 result = ::listen(this->nativeSocket, backlog);
    if (result>=0) {
        this->listening = true;
        return result;
    }
    this->listening = false;
    std::shared_ptr< KNativeSocketObject> t = std::dynamic_pointer_cast<KNativeSocketObject>(shared_from_this());
    return handleNativeSocketError(t, false);
}

U32 KNativeSocketObject::accept(KThread* thread, KFileDescriptor* fd, U32 address, U32 len, U32 flags) {
    struct sockaddr addr = {};
    socklen_t addrlen = sizeof(struct sockaddr);
    S32 result = (S32)::accept(this->nativeSocket, &addr, &addrlen);
    KMemory* memory = thread->memory;

    if (result>=0) {
        std::shared_ptr<KNativeSocketObject> s = std::make_shared<KNativeSocketObject>(this->domain, this->type, this->protocol);
        KFileDescriptor* resultFD = KThread::currentThread()->process->allocFileDescriptor(s, K_O_RDWR, 0, -1, 0);

        if (flags & FD_CLOEXEC) {
            resultFD->descriptorFlags|=FD_CLOEXEC;
        }
        if (flags & K_O_NONBLOCK) {
            resultFD->kobject->setBlocking(false);
        }

        s->nativeSocket = result;
#ifndef BOXEDWINE_MULTI_THREADED
        setNativeBlocking(result, false);
#endif

        if (address)
            memory->memcpy(address, &addr, addrlen);
        if (len) {
            memory->writed(len, addrlen);
        }
        return resultFD->handle;
    }
    std::shared_ptr< KNativeSocketObject> t = std::dynamic_pointer_cast<KNativeSocketObject>(shared_from_this());
    return handleNativeSocketError(t, false);
}

U32 KNativeSocketObject::getsockname(KThread* thread, KFileDescriptor* fd, U32 address, U32 plen) {    
    KMemory* memory = thread->memory;

    socklen_t len = (socklen_t)memory->readd( plen);
    char* buf = new char[len];
    U32 result = ::getsockname(this->nativeSocket, (struct sockaddr*)buf, &len);
    if (result) {
        std::shared_ptr< KNativeSocketObject> t = std::dynamic_pointer_cast<KNativeSocketObject>(shared_from_this());
        result = handleNativeSocketError(t, false);
    } else {
        memory->memcpy(address, buf, len);
        memory->writed(plen, len);
        this->error = 0;
    }
    delete[] buf;
    return result;
}

U32 KNativeSocketObject::getpeername(KThread* thread, KFileDescriptor* fd, U32 address, U32 plen) {
    KMemory* memory = thread->memory;
    socklen_t len = (socklen_t)memory->readd( plen);
    char* buf = new char[len];

    S32 result = ::getpeername(this->nativeSocket, (struct sockaddr*)buf, &len);
    if (result==0) {
        memory->memcpy(address, buf, len);
        memory->writed(plen, len);
        this->error = 0;
    } else {
        std::shared_ptr< KNativeSocketObject> t = std::dynamic_pointer_cast<KNativeSocketObject>(shared_from_this());
        result = handleNativeSocketError(t, false);
    }
    delete[] buf;
    return result;
}

U32 KNativeSocketObject::shutdown(KThread* thread, KFileDescriptor* fd, U32 how) {
    if (::shutdown(this->nativeSocket, how) == 0)
        return 0;
    return -1;
}

U32 KNativeSocketObject::setsockopt(KThread* thread, KFileDescriptor* fd, U32 level, U32 name, U32 value, U32 len) {
    KMemory* memory = thread->memory;

    if (level == K_IPPROTO_IP) {
        U32 v = 0;
        switch (name) {
        case K_IP_MTU_DISCOVER:
            if (len != 4)
                kpanic("KNativeSocketObject::setsockopt K_IP_MULTICAST_TTL expecting len of 4");
            v = memory->readd(value);
#if defined(IP_MTU_DISCOVER) && defined(IP_PMTUDISC_DO) && defined(IP_PMTUDISC_DONT)
            ::setsockopt(this->nativeSocket, IPPROTO_IP, IP_MTU_DISCOVER, (const char*)&v, 4);
#endif
            break;
        case K_IP_TOS: 
            if (len != 4)
                kpanic("KNativeSocketObject::setsockopt K_IP_TOS expecting len of 4");
            v = memory->readd(value);
            ::setsockopt(this->nativeSocket, IPPROTO_IP, IP_TOS, (const char*)&v, 4);
            break;
        case K_IP_RECVTOS:
            if (len != 4)
                kpanic("KNativeSocketObject::setsockopt K_IP_RECVTOS expecting len of 4");
            v = memory->readd(value);
            ::setsockopt(this->nativeSocket, IPPROTO_IP, IP_RECVTOS, (const char*)&v, 4);
            break;
        case K_IP_RECVERR:
            if (len != 4)
                kpanic("KNativeSocketObject::setsockopt K_IP_RECVERR expecting len of 4");
            v = memory->readd(value);
#ifdef IP_RECVERR
            ::setsockopt(this->nativeSocket, IPPROTO_IP, IP_RECVERR, (const char*)&v, 4);
#endif
            break;
        default:
            kwarn("KNativeSocketObject::setsockopt IPPROTO_IP name %d not implemented", name);
            return -K_EINVAL;
        }
    }
    else if (level == K_IPPROTO_TCP) {
        U32 v = 0;
        switch (name) {
        case K_TCP_NODELAY:
            if (len != 4)
                kpanic("KNativeSocketObject::setsockopt TCP_NODELAY expecting len of 4");
            v = memory->readd(value);
            ::setsockopt(this->nativeSocket, IPPROTO_TCP, TCP_NODELAY, (const char*)&v, 4);
            break;
        case K_TCP_KEEPIDLE:
            if (len != 4)
                kpanic("KNativeSocketObject::setsockopt TCP_KEEPIDLE expecting len of 4");
            v = memory->readd(value);
            ::setsockopt(this->nativeSocket, IPPROTO_TCP, TCP_KEEPIDLE, (const char*)&v, 4);
            break;
        case K_TCP_KEEPINTVL:
            if (len != 4)
                kpanic("KNativeSocketObject::setsockopt TCP_KEEPINTVL expecting len of 4");
            v = memory->readd(value);
            ::setsockopt(this->nativeSocket, IPPROTO_TCP, TCP_KEEPINTVL, (const char*)&v, 4);
            break;
        default:
            kwarn("KNativeSocketObject::setsockopt IPPROTO_TCP name %d not implemented", name);
            return -K_EINVAL;
        }
    } else if (level == K_SOL_SOCKET) {
        U32 v = 0;
        switch (name) {
            case K_SO_RCVBUF:
                if (len!=4)
                    kpanic("KNativeSocketObject::setsockopt SO_RCVBUF expecting len of 4");
                this->recvLen = memory->readd(value);
                ::setsockopt(this->nativeSocket, SOL_SOCKET, SO_RCVBUF, (const char*)&this->recvLen, 4);
                break;
            case K_SO_SNDBUF:
                if (len != 4)
                    kpanic("KNativeSocketObject::setsockopt SO_SNDBUF expecting len of 4");
                this->sendLen = memory->readd(value);
                ::setsockopt(this->nativeSocket, SOL_SOCKET, SO_SNDBUF, (const char*)&this->recvLen, 4);
                break;
            case K_SO_SNDTIMEO:            
                if (len != 8) {
                    kpanic("KNativeSocketObject::setsockopt SO_SNDTIMEO expecting len of 8");
                }
                {
#ifdef BOXEDWINE_MSVC
                    U32 sec = memory->readd(value);
                    U32 usec = memory->readd(value+4);
                    DWORD v = sec * 1000 + usec / 1000;
                    len = 4;
#else
                    struct timeval v;
                    v.tv_usec = memory->readd(value+4);
                    v.tv_sec = memory->readd(value);
                    len = sizeof(struct timeval);
#endif
                    ::setsockopt(this->nativeSocket, SOL_SOCKET, SO_SNDTIMEO, (const char*)&v, len);
                }
                break;
            case K_SO_RCVTIMEO:
                if (len != 8) {
                    kpanic("KNativeSocketObject::setsockopt SO_RCVTIMEO expecting len of 8");
                }
                {
#ifdef BOXEDWINE_MSVC
                    U32 sec = memory->readd(value);
                    U32 usec = memory->readd(value + 4);
                    DWORD v = sec * 1000 + usec / 1000;
                    len = 4;
#else
                    struct timeval v;
                    v.tv_usec = memory->readd(value + 4);
                    v.tv_sec = memory->readd(value);
                    len = sizeof(struct timeval);
#endif
                    ::setsockopt(this->nativeSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&v, len);
                }
                break;
            case K_SO_TIMESTAMP:
                break;
            case K_SO_KEEPALIVE:
                if (len != 4)
                    kpanic("KNativeSocketObject::setsockopt SO_KEEPALIVE expecting len of 4");
                v = memory->readd(value);
                ::setsockopt(this->nativeSocket, SOL_SOCKET, SO_KEEPALIVE, (const char*)&v, 4);
                break;
            case K_SO_BROADCAST:
                if (len != 4)
                    kpanic("KNativeSocketObject::setsockopt SO_BROADCAST expecting len of 4");
                v = memory->readd(value);
                ::setsockopt(this->nativeSocket, SOL_SOCKET, SO_BROADCAST, (const char*)&v, 4);
                break;
            case K_SO_REUSEADDR:
                if (len != 4) {
                    kpanic("KNativeSocketObject::setsockopt SO_REUSEADDR expecting len of 4");
                }
                v = memory->readd(value);
                ::setsockopt(this->nativeSocket, SOL_SOCKET, SO_BROADCAST, (const char*)&v, 4);
                break;
            default:
                kwarn("KNativeSocketObject::setsockopt SOL_SOCKET name %d not implemented", name);
                return -K_EINVAL;
        }
    } else {
        kwarn("KNativeSocketObject::setsockopt level %d not implemented", level);
        return -K_EINVAL;
    }
    return 0;
}

U32 KNativeSocketObject::getsockopt(KThread* thread, KFileDescriptor* fd, U32 level, U32 name, U32 value, U32 len_address) {
    KMemory* memory = thread->memory;
    socklen_t len = (socklen_t)memory->readd(len_address);
    S32 result = 0;

    if (level == K_SOL_SOCKET) {
        if (name == K_SO_RCVBUF) {
            if (len!=4)
                kpanic("KNativeSocketObject::getsockopt SO_RCVBUF expecting len of 4");
            result = ::getsockopt(this->nativeSocket, SOL_SOCKET, SO_RCVBUF, (char*)&this->recvLen, &len);
            if (!result) {
                memory->writed(value, this->recvLen);
            }
        } else if (name == K_SO_SNDBUF) {
            if (len != 4)
                kpanic("KNativeSocketObject::getsockopt SO_SNDBUF expecting len of 4");
            result = ::getsockopt(this->nativeSocket, SOL_SOCKET, SO_SNDBUF, (char*)&this->sendLen, &len);
            if (!result) {
                memory->writed(value, this->sendLen);
            }
        } else if (name == K_SO_ERROR) {
            if (len != 4)
                kpanic("KNativeSocketObject::getsockopt SO_ERROR expecting len of 4");
            memory->writed(value, this->error);
            //this->error = 0;
        } else if (name == K_SO_TYPE) { 
            if (len != 4)
                kpanic("KNativeSocketObject::getsockopt K_SO_TYPE expecting len of 4");
            memory->writed(value, this->type);
        } else if (name == K_SO_OOBINLINE) {
            if (len != 4)
                kpanic("KNativeSocketObject::getsockopt SO_OOBINLINE expecting len of 4");
            result = ::getsockopt(this->nativeSocket, SOL_SOCKET, SO_OOBINLINE, (char*)&result, &len);
            if (!result) {
                memory->writed(value, result);
            }
        } else if (name == K_SO_BROADCAST) {
            if (len != 4)
                kpanic("KNativeSocketObject::getsockopt SO_BROADCAST expecting len of 4");
            U32 result = 0;
            result = ::getsockopt(this->nativeSocket, SOL_SOCKET, SO_BROADCAST, (char*)&result, &len);
            if (!result) {
                memory->writed(value, result);
            }
        } else if (name == K_SO_RCVTIMEO) {
            if (len != 8)
                kpanic("KNativeSocketObject::getsockopt SO_RCVTIMEO expecting len of 8");            
#ifdef BOXEDWINE_MSVC            
            len = 4;
            DWORD v = 0;
            result = ::getsockopt(this->nativeSocket, SOL_SOCKET, SO_RCVTIMEO, (char*)&v, &len);

            U32 sec = v / 1000;
            U32 usec = (v % 1000) * 1000;
#else
            struct timeval v;
            len = sizeof(struct timeval);
            result = ::getsockopt(this->nativeSocket, SOL_SOCKET, SO_RCVTIMEO, (char*)&v, &len);

            U32 sec = (U32)v.tv_sec;
            U32 usec = (U32)v.tv_usec;
#endif
            if (!result) {
                memory->writed(value, sec);
                memory->writed(value + 4, usec);
            }
        } else if (name == K_SO_SNDTIMEO) {
            if (len != 8)
                kpanic("KNativeSocketObject::getsockopt SO_SNDTIMEO expecting len of 8");
#ifdef BOXEDWINE_MSVC            
            len = 4;
            DWORD v = 0;
            result = ::getsockopt(this->nativeSocket, SOL_SOCKET, SO_SNDTIMEO, (char*)&v, &len);

            U32 sec = v / 1000;
            U32 usec = (v % 1000) * 1000;
#else
            struct timeval v;
            len = sizeof(struct timeval);
            result = ::getsockopt(this->nativeSocket, SOL_SOCKET, SO_RCVTIMEO, (char*)&v, &len);

            U32 sec = (U32)v.tv_sec;
            U32 usec = (U32)v.tv_usec;
#endif
            if (!result) {
                memory->writed(value, sec);
                memory->writed(value + 4, usec);
            }
        } else if (name == K_SO_BROADCAST) {
            if (len != 4)
                kpanic("KNativeSocketObject::getsockopt SO_BROADCAST expecting len of 4");
            result = ::getsockopt(this->nativeSocket, SOL_SOCKET, SO_BROADCAST, (char*)&result, &len);
            if (!result) {
                memory->writed(value, result);
            }
        } else if (name == K_SO_ACCEPTCONN) {
            if (len != 4)
                kpanic("KNativeSocketObject::getsockopt SO_ACCEPTCONN expecting len of 4");
            result = ::getsockopt(this->nativeSocket, SOL_SOCKET, SO_ACCEPTCONN, (char*)&result, &len);
            if (!result) {
                memory->writed(value, result);
            }
        } else {
            kwarn("KNativeSocketObject::getsockopt name %d not implemented", name);
            return -K_EINVAL;
        }
    } else {
        kwarn("KNativeSocketObject::getsockopt level %d not implemented", level);
        return -K_EINVAL;
    }
    if (result != 0) {
        std::shared_ptr< KNativeSocketObject> t = std::dynamic_pointer_cast<KNativeSocketObject>(shared_from_this());
        return handleNativeSocketError(t, true);
    }
    return (U32)result;
}

U32 KNativeSocketObject::sendmsg(KThread* thread, KFileDescriptor* fd, U32 address, U32 flags) {
    MsgHdr hdr = {};
    KMemory* memory = thread->memory;
    readMsgHdr(thread, address, &hdr);

    if (hdr.msg_control) {
        CMsgHdr cmsg;			

        readCMsgHdr(thread, hdr.msg_control, &cmsg);
        if (cmsg.cmsg_level != K_SOL_SOCKET) {
            kwarn("KNativeSocketObject::sendmsg control level %d not implemented", cmsg.cmsg_level);
        } else if (cmsg.cmsg_type != K_SCM_RIGHTS) {
            kwarn("KNativeSocketObject::sendmsg control type %d not implemented", cmsg.cmsg_level);
        } else if ((cmsg.cmsg_len & 3) != 0) {
            kwarn("KNativeSocketObject::sendmsg control len %d not implemented", cmsg.cmsg_len);
        }
        if (hdr.msg_controllen>0) {
            kwarn("KNativeSocketObject::sendmsg does not support sending file handles");
        }				
    }
    U32 len = 0;
    for (U32 i=0;i<hdr.msg_iovlen;i++) {
        len += memory->readd(hdr.msg_iov + 8 * i + 4);
    }
    U8* buffer = new U8[len];
    len = 0;
    for (U32 i = 0; i < hdr.msg_iovlen; i++) {
        U32 p = memory->readd(hdr.msg_iov + 8 * i);
        U32 toCopy = memory->readd(hdr.msg_iov + 8 * i + 4);
        memory->memcpy(buffer + len, p, toCopy);
        len += toCopy;
    }
    struct sockaddr dest = {};
    U32 destLen = std::min((U32)sizeof(struct sockaddr), hdr.msg_namelen);
    if (destLen) {
        memory->memcpy(&dest, hdr.msg_name, destLen);
    }

    U32 nativeFlags = 0;
    if (flags & K_MSG_OOB)
        nativeFlags |= MSG_OOB;
    flags &= ~K_MSG_NOSIGNAL;
    if (flags & (~1)) {
        kwarn("KNativeSocketObject::sendmsg unsupported flags: %d", flags);
    }

    U32 result = (U32)::sendto(this->nativeSocket, (const char*)buffer, len, nativeFlags, &dest, destLen);
    delete[] buffer;
    if ((S32)result >= 0) {
        this->error = 0;
        return result;
    }
    std::shared_ptr< KNativeSocketObject> t = std::dynamic_pointer_cast<KNativeSocketObject>(shared_from_this());
    return handleNativeSocketError(t, true);
}

U32 KNativeSocketObject::recvmsg(KThread* thread, KFileDescriptor* fd, U32 address, U32 flags) {
    KMemory* memory = thread->memory;
    char tmp[K_PAGE_SIZE] = { 0 };
    MsgHdr hdr = { 0 };
    U32 result = 0;

    readMsgHdr(thread, address, &hdr);        
    for (U32 i = 0; i < hdr.msg_iovlen; i++) {
        U32 p = memory->readd(hdr.msg_iov + 8 * i);
        U32 len = memory->readd(hdr.msg_iov + 8 * i + 4);

        if (this->type == K_SOCK_DGRAM && this->domain==K_AF_INET && hdr.msg_namelen>=sizeof(struct sockaddr_in)) {
            struct sockaddr_in in = {};
            socklen_t inLen = sizeof(struct sockaddr_in);

            if (len>sizeof(tmp))
                len = sizeof(tmp);
            S32 r = (S32)::recvfrom(this->nativeSocket, tmp, len, 0, (struct sockaddr*)&in, &inLen);
            if (r>=0) {
                memory->memcpy(p, tmp, r);
                // :TODO: maybe copied fields to the expected location rather than assume the structures are the same
                memory->memcpy(memory->readd(address), &in, sizeof(in));
                memory->writed(address + 4, inLen);
                result+=r;
            }
            else if (result) {
                break;
            } else {
                std::shared_ptr< KNativeSocketObject> t = std::dynamic_pointer_cast<KNativeSocketObject>(shared_from_this());
                result = handleNativeSocketError(t, false);
            }
        } else {
            result += this->read(thread, p, len);
        }
    }
    if (this->type==K_SOCK_STREAM)
        memory->writed(address + 4, 0); // msg_namelen, set to 0 for connected sockets
    memory->writed(address + 20, 0); // msg_controllen
    return result;
}

U32 KNativeSocketObject::sendto(KThread* thread, KFileDescriptor* fd, U32 message, U32 length, U32 flags, U32 dest_addr, U32 dest_len) {
    KMemory* memory = thread->memory;
    U32 nativeFlags = 0;
    struct sockaddr dest = { 0 };
    int len=sizeof(struct sockaddr);
  

    if (flags & K_MSG_OOB)
        nativeFlags|=MSG_OOB;
    if (flags & (~1)) {
        kwarn("KNativeSocketObject::sendto unsupported flags: %d", flags);
    }
    memory->memcpy(&dest, dest_addr, len);
    S8* tmp = new S8[length];
    memory->memcpy(tmp, message, length);
    U32 result = (U32)::sendto(this->nativeSocket, (char*)tmp, length, nativeFlags, &dest, len);
    delete[] tmp;
    if ((S32)result>=0) {
        this->error = 0;
        return result;
    }
    std::shared_ptr< KNativeSocketObject> t = std::dynamic_pointer_cast<KNativeSocketObject>(shared_from_this());
    return handleNativeSocketError(t, true);
}

U32 KNativeSocketObject::recvfrom(KThread* thread, KFileDescriptor* fd, U32 buffer, U32 length, U32 flags, U32 address, U32 address_len) {
    U32 nativeFlags = 0;
    KMemory* memory = thread->memory;

    if (length && !thread->memory->canWrite(buffer, length)) {
        return -K_EFAULT;
    }
    if (flags & K_MSG_OOB)
        nativeFlags|=MSG_OOB;
    if (flags & K_MSG_PEEK)
        nativeFlags|=MSG_PEEK;
    if (flags & (~3)) {
        kwarn("krecvfrom unsupported flags: %d", flags);
    }
    int inLen=0;
    socklen_t outLen=0;
    char* fromBuffer = nullptr;

    if (address_len) {
        inLen = memory->readd( address_len);
        fromBuffer = new char[inLen];
        memory->memcpy(fromBuffer, address, inLen);
    }
    char* tmp = nullptr;
    
    if (length) {
        tmp = new char[length];
    }
    outLen = inLen;
    U32 result = (U32)::recvfrom(this->nativeSocket, tmp, length, nativeFlags, (struct sockaddr*)fromBuffer, &outLen);
    if ((S32)result>=0) {
        memory->memcpy(buffer, tmp, result);
        memory->memcpy(address, fromBuffer, std::min((int)outLen, inLen));
        memory->writed(address_len, outLen);
        this->error = 0;
    } else {
        std::shared_ptr< KNativeSocketObject> t = std::dynamic_pointer_cast<KNativeSocketObject>(shared_from_this());
        result = handleNativeSocketError(t, false);
        if (result == -K_EMSGSIZE) {
            if (length && buffer) {
                memory->memcpy(buffer, tmp, length);
            }
            memory->memcpy(address, fromBuffer, std::min((int)outLen, inLen));
            memory->writed(address_len, outLen);
        } 
    }
    if (tmp) {
        delete[] tmp;
    }
    if (fromBuffer)
        delete[] fromBuffer;
    return result;
}

FsOpenNode* openHosts(const std::shared_ptr<FsNode>& node, U32 flags, U32 data) {
    char name[256] = {};
    char buf[256] = {};
    name[0] = 0;
    gethostname(name, 256);
    snprintf(buf, sizeof(buf), "127.0.0.1\tlocalhost\n127.0.1.1\t%s\n::1\tip6-localhost ip6-loopback", name);
    return new BufferAccess(node, flags, BString::copy(buf));
}

FsOpenNode* openHostname(const std::shared_ptr<FsNode>& node, U32 flags, U32 data) {
    char buf[256];
    buf[0] = 0;
    gethostname(buf, 256);
    return new BufferAccess(node, flags, BString::copy(buf));
}
