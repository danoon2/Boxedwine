#include "boxedwine.h"

#include "knativesocket.h"
#include "ksocket.h"
#include "kstat.h"

#ifdef WIN32
#undef BOOL
#include <winsock.h>
static int winsock_intialized;
#define BOOL unsigned int
#pragma comment(lib, "Ws2_32.lib")
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <fcntl.h>
void closesocket(int socket) { close(socket); }
#endif

KList<KNativeSocketObject*> waitingNativeSockets;
fd_set waitingReadset;
fd_set waitingWriteset;
fd_set waitingErrorset;
int maxSocketId;

void updateWaitingList() {
    FD_ZERO(&waitingReadset);
    FD_ZERO(&waitingWriteset);
    FD_ZERO(&waitingErrorset);

    maxSocketId = 0;
    waitingNativeSockets.for_each([] (KListNode<KNativeSocketObject*>* node) {
        bool errorSet = false;
#ifndef BOXEDWINE_MSVC
        if (node->data->nativeSocket>=FD_SETSIZE) {
            kpanic("updateWaitingList %s socket is too large to select on", node->data->nativeSocket);
        }
#endif
        if (node->data->waitingOnReadThread.size()) {
            FD_SET(node->data->nativeSocket, &waitingReadset);
            FD_SET(node->data->nativeSocket, &waitingErrorset);
            errorSet = true;
        }
        if (node->data->waitingOnWriteThread.size()) {
            FD_SET(node->data->nativeSocket, &waitingWriteset);
            if (!errorSet)
                FD_SET(node->data->nativeSocket, &waitingErrorset);
        }
        if (node->data->nativeSocket>maxSocketId)
            maxSocketId = node->data->nativeSocket;
    });
}

void addWaitingNativeSocket(KNativeSocketObject* s) {
    if (!s->waitingNode.isInList()) {
        waitingNativeSockets.addToBack(&s->waitingNode);
    }
}

void removeWaitingSocket(KNativeSocketObject* s) {
    s->waitingNode.remove();
    s->wakeAndResetWaitingOnReadThreads();
    s->wakeAndResetWaitingOnWriteThreads();
}

bool checkWaitingNativeSockets(int timeout) {
    struct timeval t;
    t.tv_sec = 0;
    t.tv_usec = timeout*1000;

    if (waitingNativeSockets.size()) {
        int result;        

        updateWaitingList();       
        
        if (maxSocketId==0)
            return false;
        result = select(maxSocketId + 1, &waitingReadset, &waitingWriteset, &waitingErrorset, (timeout>=0?&t:0));
        if (result) {
            waitingNativeSockets.for_each([] (KListNode<KNativeSocketObject*>* node) {
                U32 found = 0;
                if (FD_ISSET(node->data->nativeSocket, &waitingReadset) && node->data->wakeAndResetWaitingOnReadThreads()) {
                    found = 1;
                }
                if (FD_ISSET(node->data->nativeSocket, &waitingWriteset) && node->data->wakeAndResetWaitingOnWriteThreads()) {
                    found = 1;
                }
                if (FD_ISSET(node->data->nativeSocket, &waitingErrorset)) {
                    node->data->wakeAndResetWaitingOnWriteThreads();
                    node->data->wakeAndResetWaitingOnReadThreads();
                    found = 1;
                }
                if (found)
                    removeWaitingSocket(node->data);
            });
        }
        return true;
    }
    return false;
}


S32 translateNativeSocketError(int error) {
    S32 result;
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
        result = 0;
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
    else
        result = -K_EIO;
#endif
    return result;
}

S32 handleNativeSocketError(KThread* thread, KNativeSocketObject* s, bool write) {
#ifdef WIN32
    S32 result = translateNativeSocketError(WSAGetLastError());
#else
    S32 result = translateNativeSocketError(errno);
#endif
    if (result == -K_EWOULDBLOCK) {
        if (write) {
            s->waitOnSocketWrite(thread);
        } else {
            s->waitOnSocketRead(thread);
        }
        addWaitingNativeSocket(s);        
        result = -K_WAIT;
    }
    s->error = -result;
    return result;
}

KNativeSocketObject::KNativeSocketObject(U32 domain, U32 type, U32 protocol) : KSocketObject(KTYPE_NATIVE_SOCKET, domain, type, protocol), connecting(false), waitingNode(this) {
#ifdef WIN32
    if (!winsock_intialized) {
        WSADATA wsaData;
        WSAStartup(0x0202, &wsaData);
        winsock_intialized=1;
    }
#endif
    U32 nativeType;
    U32 nativeProtocol;

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
}

KNativeSocketObject::~KNativeSocketObject() {
    closesocket(this->nativeSocket);
    this->nativeSocket = 0;
    removeWaitingSocket(this);
}

U32 KNativeSocketObject::ioctl(U32 request) {
    if (request == 0x541b) {        
#ifdef WIN32
        u_long value=0;
        U32 result = ioctlsocket(this->nativeSocket, FIONREAD, &value);
#else        
        int value=0;
        U32 result = ::ioctl(this->nativeSocket, FIONREAD, &value);        
#endif
        if (result!=0)
            return handleNativeSocketError(KThread::currentThread(), this, true);
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

void KNativeSocketObject::setBlocking(bool blocking) {
    this->blocking = blocking;
    setNativeBlocking(this->nativeSocket, blocking);
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
    kwarn("KNativeSocketObject::getLock not implemented yet");
    return 0;
}

U32 KNativeSocketObject::setLock(KFileLock* lock, bool wait) {
    kwarn("KNativeSocketObject::setLock not implemented yet");
    return -1;
}

bool KNativeSocketObject::supportsLocks() {
    return false;
}

bool KNativeSocketObject::isOpen() {
    return this->listening || this->connected;
}

bool KNativeSocketObject::isReadReady() {
    fd_set          sready;
    struct timeval  nowait;

    FD_ZERO(&sready);
    FD_SET(this->nativeSocket, &sready);
    memset((char *)&nowait,0,sizeof(nowait));

    ::select(this->nativeSocket+1,&sready,NULL,NULL,&nowait);
    return FD_ISSET(this->nativeSocket,&sready)!=0;
}

bool KNativeSocketObject::isWriteReady() {
    fd_set          sready;
    struct timeval  nowait;

    FD_ZERO(&sready);
    FD_SET(this->nativeSocket, &sready);
    memset((char *)&nowait,0,sizeof(nowait));

    ::select(this->nativeSocket+1,NULL,&sready,NULL,&nowait);
    return FD_ISSET(this->nativeSocket,&sready)!=0;
}

void KNativeSocketObject::waitForEvents(U32 events) {
    if (events & K_POLLIN) {
        this->waitOnSocketRead(KThread::currentThread());
    }
    if (events & K_POLLOUT) {
        this->waitOnSocketWrite(KThread::currentThread());
    }
    addWaitingNativeSocket(this);
}

U32 KNativeSocketObject::writeNative(U8* buffer, U32 len) {
    S32 result = ::send(this->nativeSocket, (const char*)buffer, len, this->flags);
    if (result>=0) {            
        this->error = 0;
        return result;
    }
    return handleNativeSocketError(KThread::currentThread(), this, true);
}

U32 KNativeSocketObject::readNative(U8* buffer, U32 len) {
    S32 result = ::recv(this->nativeSocket, (char*)buffer, len, this->flags);
    if (result>=0) {  
        this->error = 0;
        return result;
    }
    return handleNativeSocketError(KThread::currentThread(), this, false);
}

U32 KNativeSocketObject::stat(U32 address, bool is64) {
    KSystem::writeStat("", address, is64, 1, 0, K_S_IFSOCK|K__S_IWRITE|K__S_IREAD, 0, 0, 4096, 0, 0, 1);
    return 0;
}

U32 KNativeSocketObject::map(U32 address, U32 len, S32 prot, S32 flags, U64 off) {
    return 0;
}

bool KNativeSocketObject::canMap() {
    return false;
}

U32 KNativeSocketObject::bind(KFileDescriptor* fd, U32 address, U32 len) {
    U32 family = readw(address);
    if (family == K_AF_INET) {
        if (fd->kobject->type!=KTYPE_NATIVE_SOCKET) {
            return -K_ENOTSOCK;
        }
        char tmp[4096];
        if (len>sizeof(tmp)) {
            kpanic("kbind: buffer not large enough: len=%d", len);
        }
        memcopyToNative(address, tmp, len);
        if (::bind(this->nativeSocket, (struct sockaddr*)tmp, len)==0) {
            this->error = 0;
            return 0;
        }
        return handleNativeSocketError(KThread::currentThread(), this, false);
    }
    return -K_EAFNOSUPPORT;
}

U32 KNativeSocketObject::connect(KFileDescriptor* fd, U32 address, U32 len) {
    char buffer[1024];
    U32 result = 0;

    if (this->connecting) {
        if (this->isWriteReady()) {
            this->error = 0;
            this->connecting = 0;
            this->connected = true;
            removeWaitingSocket(this);
            return 0;
        } else {
            int error=0;
            socklen_t errLen = 4;
            if (::getsockopt(this->nativeSocket, SOL_SOCKET, SO_ERROR, (char*)&error, &errLen) < 0) {
                return -K_EIO;
            }
            if (error) {
                result = translateNativeSocketError(error);
                if (result!=-K_EWOULDBLOCK) {
                    return result;
                }
            }
        }
        addWaitingNativeSocket(this); 
        waitOnSocketWrite(KThread::currentThread());
        return -K_WAIT;
    }

    memcopyToNative(address, buffer, len);
    if (::connect(this->nativeSocket, (struct sockaddr*)buffer, len)==0) {
        int error;
        socklen_t optLen = 4;
        ::getsockopt(this->nativeSocket, SOL_SOCKET, SO_ERROR, (char*)&error, &optLen);
        if (error==0) {
            this->connected = true;
            return 0;
        }
        this->connecting = false;
        return -K_ECONNREFUSED;
    }   
    result = handleNativeSocketError(KThread::currentThread(), this, true);
    if (result == -K_WAIT) {            
        this->connecting = true;
    }
    return result;
}

U32 KNativeSocketObject::listen(KFileDescriptor* fd, U32 backlog) {
    S32 result = ::listen(this->nativeSocket, backlog);
    if (result>=0) {
        this->listening = true;
        return result;
    }
    this->listening = false;
    return handleNativeSocketError(KThread::currentThread(), this, false);
}

U32 KNativeSocketObject::accept(KFileDescriptor* fd, U32 address, U32 len) {
    struct sockaddr addr;
    socklen_t addrlen = sizeof(struct sockaddr);
    S32 result = (S32)::accept(this->nativeSocket, &addr, &addrlen);
    if (result>=0) {
        BoxedPtr<KNativeSocketObject> s = new KNativeSocketObject(this->domain, this->type, this->protocol);
        KFileDescriptor* resultFD = KThread::currentThread()->process->allocFileDescriptor(s, K_O_RDWR, 0, -1, 0);

        s->nativeSocket = result;
        setNativeBlocking(result, false);

        if (address)
            memcopyFromNative(address, (char*)&addr, addrlen);
        if (len) {
            writed(len, addrlen);
        }
        return resultFD->handle;
    }
    return handleNativeSocketError(KThread::currentThread(), this, false);
}

U32 KNativeSocketObject::getsockname(KFileDescriptor* fd, U32 address, U32 plen) {    
    socklen_t len = (socklen_t)readd( plen);
    char* buf = new char[len];
    U32 result = ::getsockname(this->nativeSocket, (struct sockaddr*)buf, &len);
    if (result)
        result = handleNativeSocketError(KThread::currentThread(), this, false);
    else {
        memcopyFromNative(address, buf, len);
        writed(plen, len);
        this->error = 0;
    }
    delete[] buf;
    return result;
}

U32 KNativeSocketObject::getpeername(KFileDescriptor* fd, U32 address, U32 plen) {
    socklen_t len = (socklen_t)readd( plen);
    char* buf = new char[len];

    S32 result = ::getpeername(this->nativeSocket, (struct sockaddr*)buf, &len);
    if (result==0) {
        memcopyFromNative(address, buf, len);
        writed(plen, len);
        this->error = 0;
    } else {
        result = handleNativeSocketError(KThread::currentThread(), this, false);
    }
    delete[] buf;
    return result;
}

U32 KNativeSocketObject::shutdown(KFileDescriptor* fd, U32 how) {
    if (::shutdown(this->nativeSocket, how) == 0)
        return 0;
    return -1;
}

U32 KNativeSocketObject::setsockopt(KFileDescriptor* fd, U32 level, U32 name, U32 value, U32 len) {
    if (level == K_SOL_SOCKET) {
        switch (name) {
            case K_SO_RCVBUF:
                if (len!=4)
                    kpanic("KNativeSocketObject::setsockopt SO_RCVBUF expecting len of 4");
                this->recvLen = readd(value);
                ::setsockopt(this->nativeSocket, SOL_SOCKET, SO_RCVBUF, (const char*)&this->recvLen, 4);
                break;
            case K_SO_SNDBUF:
                if (len != 4)
                    kpanic("KNativeSocketObject::setsockopt SO_SNDBUF expecting len of 4");
                this->sendLen = readd(value);
                ::setsockopt(this->nativeSocket, SOL_SOCKET, SO_SNDBUF, (const char*)&this->recvLen, 4);
                break;
            default:
                kwarn("KNativeSocketObject::setsockopt name %d not implemented", name);
        }
    } else {
        kwarn("KNativeSocketObject::setsockopt level %d not implemented", level);
    }
    return 0;
}

U32 KNativeSocketObject::getsockopt(KFileDescriptor* fd, U32 level, U32 name, U32 value, U32 len_address) {
    socklen_t len = (socklen_t)readd(len_address);
    if (level == K_SOL_SOCKET) {
        if (name == K_SO_RCVBUF) {
            if (len!=4)
                kpanic("KNativeSocketObject::getsockopt SO_RCVBUF expecting len of 4");
            ::getsockopt(this->nativeSocket, SOL_SOCKET, SO_RCVBUF, (char*)&this->recvLen, &len);
            writed(value, this->recvLen);
        } else if (name == K_SO_SNDBUF) {
            if (len != 4)
                kpanic("KNativeSocketObject::getsockopt SO_SNDBUF expecting len of 4");
            ::getsockopt(this->nativeSocket, SOL_SOCKET, SO_SNDBUF, (char*)&this->sendLen, &len);
            writed(value, this->sendLen);
        } else if (name == K_SO_ERROR) {
            if (len != 4)
                kpanic("KNativeSocketObject::getsockopt SO_ERROR expecting len of 4");
            writed(value, this->error);
        } else if (name == K_SO_TYPE) { 
            if (len != 4)
                kpanic("KNativeSocketObject::getsockopt K_SO_TYPE expecting len of 4");
            writed(value, this->type);
        } else {
            kwarn("KNativeSocketObject::getsockopt name %d not implemented", name);
            return -K_EINVAL;
        }
    } else {
        kwarn("KNativeSocketObject::getsockopt level %d not implemented", level);
        return -K_EINVAL;
    }
    return 0;
}

U32 KNativeSocketObject::sendmsg(KFileDescriptor* fd, U32 address, U32 flags) {
    MsgHdr hdr;
    readMsgHdr(address, &hdr);

    if (hdr.msg_control) {
        CMsgHdr cmsg;			

        readCMsgHdr(hdr.msg_control, &cmsg);
        if (cmsg.cmsg_level != K_SOL_SOCKET) {
            kpanic("KNativeSocketObject::sendmsg control level %d not implemented", cmsg.cmsg_level);
        } else if (cmsg.cmsg_type != K_SCM_RIGHTS) {
            kpanic("KNativeSocketObject::sendmsg control type %d not implemented", cmsg.cmsg_level);
        } else if ((cmsg.cmsg_len & 3) != 0) {
            kpanic("KNativeSocketObject::sendmsg control len %d not implemented", cmsg.cmsg_len);
        }
        if (hdr.msg_controllen>0) {
            kpanic("KNativeSocketObject::sendmsg does not support sending file handles");
        }				
    }
    U32 result = 0;
    for (U32 i=0;i<hdr.msg_iovlen;i++) {
        U32 p = readd(hdr.msg_iov + 8 * i);
        U32 len = readd(hdr.msg_iov + 8 * i + 4);
        result+=fd->kobject->write(p, len);
    }
    return result;
}

U32 KNativeSocketObject::recvmsg(KFileDescriptor* fd, U32 address, U32 flags) {
    char tmp[K_PAGE_SIZE];
    MsgHdr hdr;
    U32 result = 0;

    readMsgHdr(address, &hdr);        
    for (U32 i = 0; i < hdr.msg_iovlen; i++) {
        U32 p = readd(hdr.msg_iov + 8 * i);
        U32 len = readd(hdr.msg_iov + 8 * i + 4);

        if (this->type == K_SOCK_DGRAM && this->domain==K_AF_INET && hdr.msg_namelen>=sizeof(struct sockaddr_in)) {
            struct sockaddr_in in;
            S32 r;
            socklen_t inLen = sizeof(struct sockaddr_in);

            if (len>sizeof(tmp))
                len = sizeof(tmp);
            r = ::recvfrom(this->nativeSocket, tmp, len, 0, (struct sockaddr*)&in, &inLen);
            if (r>=0) {
                memcopyFromNative(p, tmp, r);
                // :TODO: maybe copied fields to the expected location rather than assume the structures are the same
                memcopyFromNative(readd(address), (const char*)&in, sizeof(in));
                writed(address + 4, inLen);
                result+=r;
            }
            else if (result)
                break;
            else
                result = handleNativeSocketError(KThread::currentThread(), this, false);
        } else {
            result += this->read(p, len);
        }
    }
    if (this->type==K_SOCK_STREAM)
        writed(address + 4, 0); // msg_namelen, set to 0 for connected sockets
    writed(address + 20, 0); // msg_controllen
    return result;
}

U32 KNativeSocketObject::sendto(KFileDescriptor* fd, U32 message, U32 length, U32 flags, U32 dest_addr, U32 dest_len) {
    U32 nativeFlags = 0;
    struct sockaddr dest;
    int len=sizeof(struct sockaddr);
    U32 result;    

    if (flags & K_MSG_OOB)
        nativeFlags|=MSG_OOB;
    if (flags & (~1)) {
        kwarn("KNativeSocketObject::sendto unsupported flags: %d", flags);
    }
    memcopyToNative(dest_addr, (char*)&dest, len);
    S8* tmp = new S8[length];
    memcopyToNative(message, tmp, length);
    result = ::sendto(this->nativeSocket, tmp, length, nativeFlags, &dest, len);
    delete[] tmp;
    if (result>=0) {
        this->error = 0;
        return result;
    }
    return handleNativeSocketError(KThread::currentThread(), this, true);
}

U32 KNativeSocketObject::recvfrom(KFileDescriptor* fd, U32 buffer, U32 length, U32 flags, U32 address, U32 address_len) {
    U32 nativeFlags = 0;

    if (flags & K_MSG_OOB)
        nativeFlags|=MSG_OOB;
    if (flags & K_MSG_PEEK)
        nativeFlags|=MSG_PEEK;
    if (flags & (~3)) {
        kwarn("krecvfrom unsupported flags: %d", flags);
    }
    int inLen=0;
    socklen_t outLen=0;
    char* fromBuffer = NULL;

    if (address_len) {
        inLen = readd( address_len);
        fromBuffer = new char[inLen];
        memcopyToNative(address, (char*)fromBuffer, inLen);
    }
    char* tmp = new char[length];
    outLen = inLen;
    // :TODO: what about tmp size
    U32 result = :: recvfrom(this->nativeSocket, tmp, length, nativeFlags, (struct sockaddr*)fromBuffer, &outLen);
    if (result>=0) {
        memcopyFromNative(buffer, tmp, result);
        memcopyFromNative(address, (char*)fromBuffer, inLen);
        writed(address_len, outLen);
        this->error = 0;
    } else {
        result = handleNativeSocketError(KThread::currentThread(), this, false);
        if (result == 0) { // WSAEMSGSIZE for example
            result = length;
            memcopyFromNative(address, (char*)fromBuffer, inLen);
            writed(address_len, outLen);
            this->error = 0;
        } 
    }
    delete[] tmp;
    if (fromBuffer)
        delete[] fromBuffer;
    return result;
}
