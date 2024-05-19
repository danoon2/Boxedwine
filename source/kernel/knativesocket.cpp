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
#include <net/if.h>

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

BString socketAddressName(KMemory* memory, U32 address, U32 len);
#ifdef _DEBUG
#define LOG_SOCK if (1) klog
#else
#define LOG_SOCK if (0) klog
#endif

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
        if (s->readingCond->parentsCount()) {
            FD_SET(s->nativeSocket, &waitingReadset);
            FD_SET(s->nativeSocket, &waitingErrorset);
            errorSet = true;
        }
        if (s->writingCond->parentsCount()) {
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
                    if ((FD_ISSET(s->nativeSocket, &waitingReadset) || FD_ISSET(s->nativeSocket, &waitingErrorset)) && s->readingCond->waitCount()) {
                        if (conditionCount<1024) conditions[conditionCount++]=&s->readingCond;
                    }
                    if ((FD_ISSET(s->nativeSocket, &waitingWriteset) || FD_ISSET(s->nativeSocket, &waitingErrorset)) && s->writingCond->waitCount()) {
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
    char buf = 2;
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
    char buf = 3;
    ::send(nativeSocketPipe[1], &buf, 1, 0);
#endif
}

S32 translateNativeSocketError(const std::shared_ptr<KNativeSocketObject>& s, int error) {
    S32 result = 0;
#ifdef WIN32
    if (error == WSAENOTCONN) {
        result = -K_ENOTCONN;
        LOG_SOCK("  native socket: %x error %s(%x)", s->nativeSocket, "ENOTCONN", result);
    } else if (error == WSAEWOULDBLOCK) {
        result = -K_EWOULDBLOCK;
        LOG_SOCK("  native socket: %x error %s(%x)", s->nativeSocket, "EWOULDBLOCK", result);
    } else if (error == WSAETIMEDOUT) {
        result = -K_ETIMEDOUT;
        LOG_SOCK("  native socket: %x error %s(%x)", s->nativeSocket, "ETIMEDOUT", result);
    } else if (error == WSAECONNRESET) {
        result = -K_ECONNRESET;
        LOG_SOCK("  native socket: %x error %s(%x)", s->nativeSocket, "ECONNRESET", result);
    } else if (error == WSAEDESTADDRREQ) {
        result = -K_EDESTADDRREQ;
        LOG_SOCK("  native socket: %x error %s(%x)", s->nativeSocket, "EDESTADDRREQ", result);
    } else if (error == WSAEHOSTUNREACH) {
        result = -K_EHOSTUNREACH;
        LOG_SOCK("  native socket: %x error %s(%x)", s->nativeSocket, "EHOSTUNREACH", result);
    } else if (error == WSAECONNREFUSED) {
        result = -K_ECONNREFUSED;
        LOG_SOCK("  native socket: %x error %s(%x)", s->nativeSocket, "ECONNREFUSED", result);
    } else if (error == WSAEISCONN) {
        result = -K_EISCONN;
        LOG_SOCK("  native socket: %x error %s(%x)", s->nativeSocket, "ECONNREFUSED", result);
    } else if (error == WSAEMSGSIZE) {
        result = -K_EMSGSIZE;
        LOG_SOCK("  native socket: %x error %s(%x)", s->nativeSocket, "EMSGSIZE", result);
    } else {
        result = -K_EIO;
        LOG_SOCK("  native socket: %x error %s(%x)", s->nativeSocket, "EIO", result);
    }

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
    else if (error == EAFNOSUPPORT)
        result = -K_EAFNOSUPPORT;
    else if (error == EINPROGRESS)
        result = -K_EINPROGRESS;
    else
        result = -K_EIO;
#endif
    return result;
}

S32 handleNativeSocketError(const std::shared_ptr<KNativeSocketObject>& s, bool write) {
#ifdef WIN32
    S32 result = translateNativeSocketError(s, WSAGetLastError());
#else
    S32 result = translateNativeSocketError(s, errno);
#endif
    s->error = -result;
#ifndef BOXEDWINE_MULTI_THREADED
    if (result == -K_EWOULDBLOCK) {
        addWaitingNativeSocket(s);
        if (s->blocking) {
            if (write) {
                BOXEDWINE_CONDITION_LOCK(s->writingCond);
                BOXEDWINE_CONDITION_WAIT(s->writingCond);
                BOXEDWINE_CONDITION_UNLOCK(s->writingCond);
            } else {
                BOXEDWINE_CONDITION_LOCK(s->readingCond);
                BOXEDWINE_CONDITION_WAIT(s->readingCond);
                BOXEDWINE_CONDITION_UNLOCK(s->readingCond);
            }
        }
    }    
#endif
    return result;
}

KNativeSocketObject::KNativeSocketObject(U32 domain, U32 type, U32 protocol) : KSocketObject(KTYPE_NATIVE_SOCKET, domain, type, protocol), 
    connecting(false),
    readingCond(std::make_shared<BoxedWineCondition>(B("KNativeSocketObject::readingCond"))),
    writingCond(std::make_shared<BoxedWineCondition>(B("KNativeSocketObject::writingCond"))) {
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
    LOG_SOCK("native socket: %x close", nativeSocket);
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

/*
struct ifreq {
    char ifr_name[IFNAMSIZ]; // Interface name 
    union {
        struct sockaddr ifr_addr;
        struct sockaddr ifr_dstaddr;
        struct sockaddr ifr_broadaddr;
        struct sockaddr ifr_netmask;
        struct sockaddr ifr_hwaddr;
        short           ifr_flags;
        int             ifr_ifindex;
        int             ifr_metric;
        int             ifr_mtu;
        struct ifmap    ifr_map;
        char            ifr_slave[IFNAMSIZ];
        char            ifr_newname[IFNAMSIZ];
        char* ifr_data;
    };
};

struct ifconf {
    int                 ifc_len; // size of buffer 
    union {
        char* ifc_buf; // buffer address 
        struct ifreq* ifc_req; // array of structures 
    };
};
*/
U32 KNativeSocketObject::ioctl(KThread* thread, U32 request) {
    CPU* cpu = thread->cpu;
    if (request == 0x541b) {        
#ifdef WIN32
        u_long value=0;
        U32 result = ioctlsocket(this->nativeSocket, FIONREAD, &value);
#else        
        int value=0;
        U32 result = ::ioctl(this->nativeSocket, FIONREAD, &value);        
#endif
        LOG_SOCK("  native socket: %x FIONREAD %d", nativeSocket, (int)value);
        if (result != 0) {
            std::shared_ptr< KNativeSocketObject> t = std::dynamic_pointer_cast<KNativeSocketObject>(shared_from_this());
            return handleNativeSocketError(t, true);
        }        
        this->error = 0;
        thread->memory->writed(IOCTL_ARG1, value);
        return 0;
    } else if (request == 0x8912) {
        U32 address = IOCTL_ARG1;
        if (!address) {
            return -K_EFAULT;
        }
#ifdef WIN32
        INTERFACE_INFO interfaces[20];
        unsigned long bytes;

        if (this->nativeSocket == SOCKET_ERROR) {
            return -1;
        }

        if (WSAIoctl(this->nativeSocket, SIO_GET_INTERFACE_LIST, 0, 0, &interfaces, sizeof(interfaces), &bytes, 0, 0) == SOCKET_ERROR) {
            std::shared_ptr< KNativeSocketObject> t = std::dynamic_pointer_cast<KNativeSocketObject>(shared_from_this());
            return handleNativeSocketError(t, false);
        }

        U32 count = bytes / sizeof(INTERFACE_INFO);
        thread->memory->writed(address, count * 40);
        U32 buf = thread->memory->readd(address + 4);
        for (U32 i = 0; i < count; i++) {
            struct sockaddr_in* addr = (struct sockaddr_in*)&(interfaces[i].iiAddress);
            if (addr->sin_family == AF_INET && (interfaces[i].iiFlags & IFF_LOOPBACK)) {
                thread->memory->memcpy(buf + 40 * i, "lo", 3);
            } else {
                BString s;
                s += "eth";
                s += i;
                thread->memory->memcpy(buf + 40 * i, s.c_str(), s.length() + 1);
            }
            thread->memory->memcpy(buf + 40 * i + 16, addr, 16); // 16 sizeof addr            
        }
        this->error = 0;
        return 0;
#else        
        U32 numifs = 64;

#ifdef SIOCGIFNUM
        ::ioctl(this->nativeSocket, SIOCGIFNUM, (char*)&numifs);
#endif
        struct ifreq* ifs = new struct ifreq[numifs];
        struct ifconf ifconf = { 0 };

        ifconf.ifc_buf = (char*)(ifs);
        ifconf.ifc_len = sizeof(struct ifreq) * numifs;

        U32 result = ::ioctl(this->nativeSocket, SIOCGIFCONF, &ifconf);
        if (result == 0xffffffff) {
            std::shared_ptr< KNativeSocketObject> t = std::dynamic_pointer_cast<KNativeSocketObject>(shared_from_this());
            return handleNativeSocketError(t, false);
        }
        U32 count = ifconf.ifc_len / sizeof(struct ifreq);
        thread->memory->writed(address, ifconf.ifc_len);
        thread->memory->memcpy(thread->memory->readd(address + 4), ifs, ifconf.ifc_len);
        delete[] ifs;
        this->error = 0;
        return 0;
#endif
    } else if (request == 0x5421) {
        U32 address = IOCTL_ARG1;
#ifdef WIN32
        u_long value = thread->memory->readd(address);
        U32 result = ioctlsocket(this->nativeSocket, FIONBIO, &value);
#else        
        int value = thread->memory->readd(address);
        U32 result = ::ioctl(this->nativeSocket, FIONBIO, &value);
#endif
        LOG_SOCK("  native socket: %x FIONBIO %d", nativeSocket, (int)value);
        if (result != 0) {
            std::shared_ptr< KNativeSocketObject> t = std::dynamic_pointer_cast<KNativeSocketObject>(shared_from_this());
            return handleNativeSocketError(t, true);
        }
        blocking = value == 0;
        thread->memory->writed(IOCTL_ARG1, value);
        this->error = 0;
        return 0;
    } else {
        kwarn("KNativeSocketObject::ioctl request=%x not implemented", request);
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
    LOG_SOCK("  native socket: %x set blocking %s", nativeSocket, blocking?"true":"false");
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
    /*
    fd_set          sready = {};
    struct timeval  nowait = { 0 };

    FD_ZERO(&sready);
    FD_SET(this->nativeSocket, &sready);

    ::select(this->nativeSocket + 1, nullptr, nullptr, &sready, &nowait);
    bool result = FD_ISSET(this->nativeSocket, &sready) != 0;
    if (result) {
        std::shared_ptr< KNativeSocketObject> t = std::dynamic_pointer_cast<KNativeSocketObject>(shared_from_this());
        this->error = handleNativeSocketError(t, false);
        return false;
    }
    */
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
        LOG_SOCK("%x native socket: %x isReadReady result=%x", KThread::currentThread()->id, nativeSocket, result);
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
        BOXEDWINE_CONDITION_ADD_PARENT(this->readingCond, parentCondition);
    } else {
        BOXEDWINE_CONDITION_REMOVE_PARENT(this->readingCond, parentCondition);
    }
    if (events & K_POLLOUT) {
        BOXEDWINE_CONDITION_ADD_PARENT(this->writingCond, parentCondition);
    } else {
        BOXEDWINE_CONDITION_REMOVE_PARENT(this->writingCond, parentCondition);
    }
    if (events == 0) {
        removeWaitingSocket(this->nativeSocket);
    } else {
        std::shared_ptr< KNativeSocketObject> t = std::dynamic_pointer_cast<KNativeSocketObject>(shared_from_this());
        addWaitingNativeSocket(t);
    }
}

U32 KNativeSocketObject::writeNative(U8* buffer, U32 len) {    
    S32 result = (S32)::send(this->nativeSocket, (const char*)buffer, len, 0);
    LOG_SOCK("native socket: %x writeNative len=%d result=%x", nativeSocket, len, result);
    if (result>=0) {            
        this->error = 0;
        return result;
    }
    std::shared_ptr< KNativeSocketObject> t = std::dynamic_pointer_cast<KNativeSocketObject>(shared_from_this());
    return handleNativeSocketError(t, true);
}

U32 KNativeSocketObject::readNative(U8* buffer, U32 len) {
    BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(readingCond);
    S32 result = (S32)::recv(this->nativeSocket, (char*)buffer, len, 0);

    LOG_SOCK("%x native socket: %x readNative len=%d result=%x", KThread::currentThread()->id, nativeSocket, len, result);
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

BString KNativeSocketObject::selfFd() {
    return B("socket:[")+this->nativeSocket+B("]"); // :TODO: should be inode
}

static void readSockAddrIn(struct sockaddr_in* addr, KMemory* memory, U32 address) {
    addr->sin_family = AF_INET;
    memory->memcpy(&addr->sin_port, address+2, 2);
    memory->memcpy(&addr->sin_addr, address+4, 4);
}
    
static void writeSockAddrIn(struct sockaddr_in* addr, KMemory* memory, U32 address) {
    memory->writew(address, AF_INET);
    memory->memcpy(address+2, &addr->sin_port, 2);
    memory->memcpy(address+4, &addr->sin_addr, 4);
}
    
U32 KNativeSocketObject::bind(KThread* thread, KFileDescriptor* fd, U32 address, U32 len) {
    KMemory* memory = thread->memory;

    U32 family = memory->readw(address);
    if (family == K_AF_INET) {
        if (fd->kobject->type!=KTYPE_NATIVE_SOCKET) {
            return -K_ENOTSOCK;
        }
        struct sockaddr_in addr = {0};
        readSockAddrIn(&addr, memory, address);
        int result = ::bind(this->nativeSocket, (struct sockaddr*)&addr, sizeof(struct sockaddr_in));
        LOG_SOCK("native socket: %x bind result=%x", nativeSocket, result);
        if (result == 0) {
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

    std::shared_ptr< KNativeSocketObject> t = std::dynamic_pointer_cast<KNativeSocketObject>(shared_from_this());
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
                result = translateNativeSocketError(t, error);
                if (result != (U32)(-K_EWOULDBLOCK)) {
                    return result;
                }
            }
        }        
        addWaitingNativeSocket(t);
        BOXEDWINE_CONDITION_LOCK(this->writingCond);
        BOXEDWINE_CONDITION_WAIT(this->writingCond);
        BOXEDWINE_CONDITION_UNLOCK(this->writingCond);
    }
#endif
    memory->memcpy(buffer, address, len);

    U16 family = memory->readw(address);
    struct sockaddr* addr = nullptr;
    sockaddr_in addr_in = {0};
    U32 addrLen = len;
    if (family == K_AF_INET) {
        readSockAddrIn(&addr_in, memory, address);
        addr = (struct sockaddr*)&addr_in;
        addrLen = sizeof(struct sockaddr_in);
        if (memory->readb(address + 4) == 127 && memory->readb(address + 5) == 0 && memory->readb(address + 6) == 0 && memory->readb(address + 7) == 1) {
            U16 port = memory->readb(address + 3) | (((U32)memory->readb(address + 2)) << 8);
            if (port == 631) {
                // wine seems to try and connect to a print server, if we are running on Linux it hangs while trying to read.  That is probably a bug, for now just prevent the connection.
                return -K_ECONNREFUSED;
            }
        }
    } else {
        addr = (struct sockaddr*)buffer;
    }

    int r = ::connect(this->nativeSocket, addr, addrLen);
    LOG_SOCK("native socket: %x connect %s result=%x", nativeSocket, socketAddressName(thread->memory, address, len).c_str(), r);
    if (r == 0) {
        int error = 0;
        socklen_t optLen = 4;
        ::getsockopt(this->nativeSocket, SOL_SOCKET, SO_ERROR, (char*)&error, &optLen);

        if (error == 0) {
            this->connected = true;
            return 0;
        }
        this->connecting = false;
        return -K_ECONNREFUSED;
    }
    result = handleNativeSocketError(t, true);
    if (result == -K_EWOULDBLOCK) {
        result = -K_EINPROGRESS;
        t->error = K_EINPROGRESS;
    }
    if (result == -K_EINPROGRESS) {
        this->connected = true;
    }
#ifndef BOXEDWINE_MULTI_THREADED
    if (result == (U32)(-K_WAIT)) {            
        this->connecting = true;
    }
#endif
    return result;
}

U32 KNativeSocketObject::listen(KThread* thread, KFileDescriptor* fd, U32 backlog) {
    S32 result = ::listen(this->nativeSocket, backlog);
    LOG_SOCK("native socket: %x list result=%x", nativeSocket, result);
    if (result>=0) {
        this->listening = true;
        this->error = 0;
        return result;
    }
    this->listening = false;
    std::shared_ptr< KNativeSocketObject> t = std::dynamic_pointer_cast<KNativeSocketObject>(shared_from_this());
    return handleNativeSocketError(t, false);
}

U32 KNativeSocketObject::accept(KThread* thread, KFileDescriptor* fd, U32 address, U32 len, U32 flags) {
    struct sockaddr_in addr = {};
    socklen_t addrlen = sizeof(struct sockaddr);
    KMemory* memory = thread->memory;
    readSockAddrIn(&addr, memory, address);
    S32 result = (S32)::accept(this->nativeSocket, (struct sockaddr*)&addr, &addrlen);
    LOG_SOCK("native socket: %x listen result=%x", nativeSocket, result);

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

        if (address) {
            writeSockAddrIn(&addr, memory, address);
        }
        if (len) {
            memory->writed(len, 16);
        }
        this->error = 0;
        return resultFD->handle;
    }
    std::shared_ptr< KNativeSocketObject> t = std::dynamic_pointer_cast<KNativeSocketObject>(shared_from_this());
    return handleNativeSocketError(t, false);
}
    
U32 KNativeSocketObject::getsockname(KThread* thread, KFileDescriptor* fd, U32 address, U32 plen) {
    KMemory* memory = thread->memory;

    socklen_t len = (socklen_t)memory->readd( plen);
    struct sockaddr_in addr = {0};
    socklen_t nativeLen = sizeof(struct sockaddr_in);
    U32 result = ::getsockname(this->nativeSocket, (struct sockaddr*)&addr, &nativeLen);
    if (result) {
        std::shared_ptr< KNativeSocketObject> t = std::dynamic_pointer_cast<KNativeSocketObject>(shared_from_this());
        result = handleNativeSocketError(t, false);
    } else {
        writeSockAddrIn(&addr, memory, address);
        memory->writed(plen, 16);
        this->error = 0;
    }
    return result;
}

U32 KNativeSocketObject::getpeername(KThread* thread, KFileDescriptor* fd, U32 address, U32 plen) {
    KMemory* memory = thread->memory;
    socklen_t len = (socklen_t)memory->readd( plen);
    struct sockaddr_in addr = {0};
    socklen_t nativeLen = sizeof(struct sockaddr_in);
    
    S32 result = ::getpeername(this->nativeSocket, (struct sockaddr*)&addr, &nativeLen);
    if (result==0) {
        writeSockAddrIn(&addr, memory, address);
        memory->writed(plen, 16);
        this->error = 0;
    } else {
        std::shared_ptr< KNativeSocketObject> t = std::dynamic_pointer_cast<KNativeSocketObject>(shared_from_this());
        result = handleNativeSocketError(t, false);
    }
    return result;
}

U32 KNativeSocketObject::shutdown(KThread* thread, KFileDescriptor* fd, U32 how) {
    LOG_SOCK("native socket: %x shutdown", nativeSocket);
    if (::shutdown(this->nativeSocket, how) == 0)
        return 0;
    return -1;
}

U32 KNativeSocketObject::setsockopt(KThread* thread, KFileDescriptor* fd, U32 level, U32 name, U32 value, U32 len) {
    KMemory* memory = thread->memory;

    LOG_SOCK("%x native socket: %x setsockopt level=%x name=%x value=%x", thread->id, nativeSocket, memory->readd(value));
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
    this->error = 0;
    return 0;
}

U32 KNativeSocketObject::getsockopt(KThread* thread, KFileDescriptor* fd, U32 level, U32 name, U32 value, U32 len_address) {
    KMemory* memory = thread->memory;
    socklen_t len = (socklen_t)memory->readd(len_address);
    S32 result = 0;
    U32 retrievedValue = 0;

    if (level == K_SOL_SOCKET) {
        if (name == K_SO_RCVBUF) {
            if (len!=4)
                kpanic("KNativeSocketObject::getsockopt SO_RCVBUF expecting len of 4");
            result = ::getsockopt(this->nativeSocket, SOL_SOCKET, SO_RCVBUF, (char*)&retrievedValue, &len);
            if (!result) {
                memory->writed(value, retrievedValue);
            }
        } else if (name == K_SO_SNDBUF) {
            if (len != 4)
                kpanic("KNativeSocketObject::getsockopt SO_SNDBUF expecting len of 4");
            result = ::getsockopt(this->nativeSocket, SOL_SOCKET, SO_SNDBUF, (char*)&retrievedValue, &len);
            if (!result) {
                memory->writed(value, retrievedValue);
            }
        } else if (name == K_SO_ERROR) {
            if (len != 4)
                kpanic("KNativeSocketObject::getsockopt SO_ERROR expecting len of 4");
            memory->writed(value, this->error);
            retrievedValue = this->error;            
        } else if (name == K_SO_TYPE) { 
            if (len != 4)
                kpanic("KNativeSocketObject::getsockopt K_SO_TYPE expecting len of 4");
            memory->writed(value, this->type);
            retrievedValue = this->type;
        } else if (name == K_SO_OOBINLINE) {
            if (len != 4)
                kpanic("KNativeSocketObject::getsockopt SO_OOBINLINE expecting len of 4");
            result = ::getsockopt(this->nativeSocket, SOL_SOCKET, SO_OOBINLINE, (char*)&retrievedValue, &len);
            if (!result) {
                memory->writed(value, retrievedValue);
            }
        } else if (name == K_SO_BROADCAST) {
            if (len != 4)
                kpanic("KNativeSocketObject::getsockopt SO_BROADCAST expecting len of 4");
            U32 result = 0;
            result = ::getsockopt(this->nativeSocket, SOL_SOCKET, SO_BROADCAST, (char*)&retrievedValue, &len);
            if (!result) {
                memory->writed(value, retrievedValue);
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
                retrievedValue = sec;
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
                retrievedValue = sec;
                memory->writed(value, sec);
                memory->writed(value + 4, usec);
            }
        } else if (name == K_SO_BROADCAST) {
            if (len != 4)
                kpanic("KNativeSocketObject::getsockopt SO_BROADCAST expecting len of 4");
            result = ::getsockopt(this->nativeSocket, SOL_SOCKET, SO_BROADCAST, (char*)&retrievedValue, &len);
            if (!result) {
                memory->writed(value, retrievedValue);
            }
        } else if (name == K_SO_ACCEPTCONN) {
            if (len != 4)
                kpanic("KNativeSocketObject::getsockopt SO_ACCEPTCONN expecting len of 4");
            result = ::getsockopt(this->nativeSocket, SOL_SOCKET, SO_ACCEPTCONN, (char*)&retrievedValue, &len);
            if (!result) {
                memory->writed(value, retrievedValue);
            }
        } else {
            kwarn("KNativeSocketObject::getsockopt name %d not implemented", name);
            return -K_EINVAL;
        }
    } else {
        kwarn("KNativeSocketObject::getsockopt level %d not implemented", level);
        return -K_EINVAL;
    }
    LOG_SOCK("%x native socket: %x getsockopt level=%x name=%x value=%x result=%x", thread->id, nativeSocket, level, name, retrievedValue, result);
    if (result != 0) {
        std::shared_ptr< KNativeSocketObject> t = std::dynamic_pointer_cast<KNativeSocketObject>(shared_from_this());
        return handleNativeSocketError(t, true);
    }
    this->error = 0;
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
    struct sockaddr_in dest = {0};
    U32 destLen = 0;
    if (hdr.msg_namelen) {
        destLen = (U32)sizeof(struct sockaddr_in);
        readSockAddrIn(&dest, memory, hdr.msg_name);
    }

    U32 nativeFlags = 0;
    if (flags & K_MSG_OOB)
        nativeFlags |= MSG_OOB;
    flags &= ~K_MSG_NOSIGNAL;
    if (flags & (~1)) {
        kwarn("KNativeSocketObject::sendmsg unsupported flags: %d", flags);
    }

    U32 result;
    if (hdr.msg_namelen) {
        result = (U32)::sendto(this->nativeSocket, (const char*)buffer, len, nativeFlags, destLen == 0 ? nullptr : (struct sockaddr*)&dest, destLen);
    } else {
        result = (U32)::send(this->nativeSocket, (const char*)buffer, len, nativeFlags);
    }
    LOG_SOCK("%x native socket: %x sendmsg msg_name=%x msg_namelen=%x result=%x", thread->id, nativeSocket, hdr.msg_name, hdr.msg_namelen, result);
    delete[] buffer;
    if ((S32)result >= 0) {
        this->error = 0;
        return result;
    }
    std::shared_ptr< KNativeSocketObject> t = std::dynamic_pointer_cast<KNativeSocketObject>(shared_from_this());
    return handleNativeSocketError(t, true);
}

U32 KNativeSocketObject::recvmsg(KThread* thread, KFileDescriptor* fd, U32 address, U32 flags) {
    BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(readingCond);
    KMemory* memory = thread->memory;
    char tmp[K_PAGE_SIZE] = { 0 };
    MsgHdr hdr = { 0 };
    U32 result = 0;
    U32 nativeFlags = 0;
    if (flags) {
        if (flags & K_MSG_PEEK) {
            nativeFlags |= MSG_PEEK;
            flags &= ~K_MSG_PEEK;
        }
        if (flags) {
            kwarn("KNativeSocketObject::recvmsg unhandled flag %x", flags);
        }
    }
    readMsgHdr(thread, address, &hdr);    

    for (U32 i = 0; i < hdr.msg_iovlen; i++) {
        U32 p = memory->readd(hdr.msg_iov + 8 * i);
        U32 len = memory->readd(hdr.msg_iov + 8 * i + 4);

        struct sockaddr_in in = {0};
        socklen_t inLen = sizeof(struct sockaddr_in);

        if (len>sizeof(tmp))
            len = sizeof(tmp);
        S32 r = (S32)::recvfrom(this->nativeSocket, tmp, len, nativeFlags, hdr.msg_name?(struct sockaddr*)&in:nullptr, hdr.msg_name ? &inLen : nullptr);
        LOG_SOCK("%x native socket: %x recvmsg flags=%x msg_name=%x msg_namelen=%x result=%x", thread->id, nativeSocket, flags, hdr.msg_name, hdr.msg_namelen, r);
        if (r>=0) {
            memory->memcpy(p, tmp, r);
            // :TODO: maybe copied fields to the expected location rather than assume the structures are the same
            if (hdr.msg_name && hdr.msg_namelen >= 16) {
                writeSockAddrIn(&in, memory, hdr.msg_name);
            }
            memory->writed(address + 4, inLen);
            result+=r;
            this->error = 0;
        }
        else if (result) {
            break;
        } else {
            std::shared_ptr< KNativeSocketObject> t = std::dynamic_pointer_cast<KNativeSocketObject>(shared_from_this());
            result = handleNativeSocketError(t, false);
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
    struct sockaddr_in dest = { 0 };
    int len=sizeof(struct sockaddr_in);
  

    if (flags & K_MSG_OOB) {
        nativeFlags |= MSG_OOB;
        flags &= ~K_MSG_OOB;
    }
    if (flags & K_MSG_NOSIGNAL) {
        flags &= ~K_MSG_NOSIGNAL;
    }
    if (flags) {
        kwarn("KNativeSocketObject::sendto unsupported flags: %d", flags);
    }
    if (dest_addr) {
        readSockAddrIn(&dest, memory, dest_addr);
    }
    S8* tmp = new S8[length];
    memory->memcpy(tmp, message, length);
    U32 result = (U32)::sendto(this->nativeSocket, (char*)tmp, length, nativeFlags, dest_addr ? (struct sockaddr*)&dest : nullptr, dest_addr ? len : 0);
    LOG_SOCK("%x native socket: %x sendto dest_addr=%x dest_len=%x result=%x", thread->id, nativeSocket, dest_addr, dest_len, result);
    delete[] tmp;
    if ((S32)result>=0) {
        this->error = 0;
        return result;
    }
    std::shared_ptr< KNativeSocketObject> t = std::dynamic_pointer_cast<KNativeSocketObject>(shared_from_this());
    return handleNativeSocketError(t, true);
}

U32 KNativeSocketObject::recvfrom(KThread* thread, KFileDescriptor* fd, U32 buffer, U32 length, U32 flags, U32 address, U32 address_len) {
    BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(readingCond);
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
    struct sockaddr_in addr = {0};

    if (address_len) {
        inLen = sizeof(struct sockaddr_in);
        readSockAddrIn(&addr, memory, address);
    }
    char* tmp = nullptr;
    
    if (length) {
        tmp = new char[length];
    }
    outLen = inLen;
    U32 result = (U32)::recvfrom(this->nativeSocket, tmp, length, nativeFlags, address_len?(struct sockaddr*)&addr:nullptr, address_len?&outLen: nullptr);
    LOG_SOCK("%x native socket: %x recvfrom buffer=%x length=%d address=%x address_len=%x flags=%x result=%x", thread->id, nativeSocket, buffer, length, address, address_len, flags, result);
    if ((S32)result>=0) {
        memory->memcpy(buffer, tmp, result);
        if (address) {
            writeSockAddrIn(&addr, memory, address);
        }
        if (address_len) {
            memory->writed(address_len, 16);
        }
        this->error = 0;
    } else {
        std::shared_ptr< KNativeSocketObject> t = std::dynamic_pointer_cast<KNativeSocketObject>(shared_from_this());
        result = handleNativeSocketError(t, false);
        if (result == -K_EMSGSIZE) {
            if (length && buffer) {
                memory->memcpy(buffer, tmp, length);
            }
            if (address) {
                writeSockAddrIn(&addr, memory, address);
            }
            if (address_len) {
                memory->writed(address_len, 16);
            }
        } 
    }
    if (tmp) {
        delete[] tmp;
    }
    return result;
}

FsOpenNode* openHosts(const std::shared_ptr<FsNode>& node, U32 flags, U32 data) {
#ifdef WIN32
    if (!winsock_intialized) {
        WSADATA wsaData;
        static_cast<void>(WSAStartup(0x0202, &wsaData));
        winsock_intialized = 1;
    }
#endif
    char name[256] = {};
    char buf[256] = {};
    name[0] = 0;
    if (gethostname(name, 256) != 0) {
        strcpy(name, "Boxedwine");
    }
    snprintf(buf, sizeof(buf), "127.0.0.1\tlocalhost\n127.0.1.1\t%s\n::1\tip6-localhost ip6-loopback", name);
    return new BufferAccess(node, flags, BString::copy(buf));
}

FsOpenNode* openHostname(const std::shared_ptr<FsNode>& node, U32 flags, U32 data) {
#ifdef WIN32
    if (!winsock_intialized) {
        WSADATA wsaData;
        static_cast<void>(WSAStartup(0x0202, &wsaData));
        winsock_intialized = 1;
    }
#endif
    char buf[256];
    buf[0] = 0;
    if (gethostname(buf, 256) != 0) {
        strcpy(buf, "Boxedwine");
    }
    return new BufferAccess(node, flags, BString::copy(buf));
}
