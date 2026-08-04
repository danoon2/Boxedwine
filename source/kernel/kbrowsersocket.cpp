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

#ifdef __EMSCRIPTEN__

#include "boxedwine.h"

#include "kbrowsersocket.h"
#include "knativesocket.h"
#include "kpoll.h"
#include "ksignal.h"
#include "ksocket.h"
#include "kstat.h"

#include <emscripten/emscripten.h>
#include <stdint.h>

extern "C" {
extern int bw_net_is_enabled();
extern int bw_net_is_debug_enabled();
extern int bw_net_socket(int domain, int type, int protocol);
extern void bw_net_close(int socket);
extern int bw_net_get_events(int socket);
extern int bw_net_get_error(int socket);
extern int bw_net_readable_bytes(int socket);
extern int bw_net_connect(int socket, unsigned int ipv4, int port);
extern int bw_net_bind(int socket, unsigned int ipv4, int port);
extern int bw_net_listen(int socket, int backlog);
extern int bw_net_accept(int socket);
extern unsigned int bw_net_get_peer_ipv4(int socket);
extern int bw_net_get_peer_port(int socket);
extern unsigned int bw_net_get_local_ipv4(int socket);
extern int bw_net_get_local_port(int socket);
extern int bw_net_send(int socket, unsigned int buffer, unsigned int len, int flags, unsigned int ipv4, int port);
extern int bw_net_recv(int socket, unsigned int buffer, unsigned int len, int flags, unsigned int address, unsigned int addressLen);
extern unsigned int bw_net_get_last_recv_ipv4(int socket);
extern int bw_net_get_last_recv_port(int socket);
extern int bw_net_shutdown(int socket, int how);
extern int bw_net_setsockopt(int socket, int level, int name, unsigned int value, unsigned int len);
}

#define K_SIOCGIFHWADDR 0x8927
#define K_SIOCGIFCONF 0x8912
#define K_SIOCGIFADDR 0x8915
#define K_SIOCGIFFLAGS 0x8913
#define K_SIOCGIFBRDADDR 0x8919
#define K_SIOCGIFNETMASK 0x891b
#define K_SIOCGIFMTU 0x8921
#define K_SIOCGIFINDEX 0x8933
#define K_SIOCGIWNAME 0x8b01

#define K_IFNAMSIZ 16
#define K_IFREQ_SIZE 40

static BOXEDWINE_MUTEX browserSocketsMutex;
static std::vector<KBrowserSocketObject*> browserSockets;

bool isBrowserNetworkDebugEnabled() {
    return bw_net_is_debug_enabled() != 0;
}

static void registerBrowserSocket(KBrowserSocketObject* socket) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(browserSocketsMutex);
    browserSockets.push_back(socket);
}

static void unregisterBrowserSocket(KBrowserSocketObject* socket) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(browserSocketsMutex);
    for (U32 i = 0; i < browserSockets.size(); i++) {
        if (browserSockets[i] == socket) {
            browserSockets.erase(browserSockets.begin() + i);
            break;
        }
    }
}

extern "C" EMSCRIPTEN_KEEPALIVE void boxedwine_browser_socket_notify(int browserSocket) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(browserSocketsMutex);
    for (KBrowserSocketObject* socket : browserSockets) {
        if (socket->getBrowserSocket() == (U32)browserSocket) {
            socket->signalBrowserEvents();
            break;
        }
    }
}

static U32 readSockAddrIpv4(KMemory* memory, U32 address) {
    return address ? memory->readd(address + 4) : 0;
}

static U16 readSockAddrPort(KMemory* memory, U32 address) {
    if (!address) {
        return 0;
    }
    return memory->readb(address + 3) | (((U32)memory->readb(address + 2)) << 8);
}

static void writeSockAddrIn(KMemory* memory, U32 address, U32 ipv4, U16 port) {
    memory->writew(address, K_AF_INET);
    memory->writeb(address + 2, (U8)(port >> 8));
    memory->writeb(address + 3, (U8)(port & 0xff));
    memory->writed(address + 4, ipv4);
    memory->memset(address + 8, 0, 8);
}

static void writeLastRecvSockAddr(KMemory* memory, U32 browserSocket, U32 address, U32 addressLen) {
    if (!address || !addressLen) {
        return;
    }
    U32 requestedLen = memory->readd(addressLen);
    if (requestedLen >= 16) {
        U32 ipv4 = bw_net_get_last_recv_ipv4((int)browserSocket);
        U16 port = (U16)bw_net_get_last_recv_port((int)browserSocket);
        writeSockAddrIn(memory, address, ipv4, port);
        BROWSER_NET_DEBUG_LOG("[boxedwine-net-cpp] udp sockaddr socket=%d %u.%u.%u.%u:%u",
            browserSocket,
            ipv4 & 0xff,
            (ipv4 >> 8) & 0xff,
            (ipv4 >> 16) & 0xff,
            (ipv4 >> 24) & 0xff,
            port);
    }
    memory->writed(addressLen, 16);
}

static const KEmulatedNetworkInterface* getInterfaceFromIfReq(KMemory* memory, U32 address) {
    return getEmulatedNetworkInterfaceByName(memory->readString(address));
}

static void writeIfReqSockAddr(KMemory* memory, U32 address, U32 ipv4) {
    writeEmulatedNetworkSockAddr(memory, address + K_IFNAMSIZ, ipv4);
}

static void writeIfReqNameAndAddr(KMemory* memory, U32 address, const KEmulatedNetworkInterface& iface) {
    memory->memset(address, 0, K_IFREQ_SIZE);
    memory->memcpy(address, iface.name, (U32)strlen(iface.name) + 1);
    writeIfReqSockAddr(memory, address, iface.ipv4);
}

KBrowserSocketObject::KBrowserSocketObject(U32 domain, U32 type, U32 protocol) : KSocketObject(KTYPE_NATIVE_SOCKET, domain, type, protocol),
    readingCond(std::make_shared<BoxedWineCondition>(B("KBrowserSocketObject::readingCond"))),
    writingCond(std::make_shared<BoxedWineCondition>(B("KBrowserSocketObject::writingCond"))) {
    if (type != K_SOCK_STREAM && type != K_SOCK_DGRAM && type != K_SOCK_RAW && type != K_SOCK_RDM && type != K_SOCK_SEQPACKET) {
        this->error = -K_EPROTOTYPE;
        return;
    }
    if (protocol != 0 && protocol != 1 && protocol != 2 && protocol != 6 && protocol != 12 && protocol != 17 && protocol != 22 && protocol != 255) {
        this->error = -K_EPROTOTYPE;
        return;
    }
    if (bw_net_is_enabled()) {
        S32 result = bw_net_socket((int)domain, (int)type, (int)protocol);
        if (result < 0) {
            this->error = result;
        } else {
            this->browserSocket = (U32)result;
            if (this->browserSocket) {
                registerBrowserSocket(this);
            }
        }
    }
}

KBrowserSocketObject::KBrowserSocketObject(U32 domain, U32 type, U32 protocol, U32 browserSocket) : KSocketObject(KTYPE_NATIVE_SOCKET, domain, type, protocol),
    browserSocket(browserSocket),
    readingCond(std::make_shared<BoxedWineCondition>(B("KBrowserSocketObject::readingCond"))),
    writingCond(std::make_shared<BoxedWineCondition>(B("KBrowserSocketObject::writingCond"))) {
    if (this->browserSocket) {
        registerBrowserSocket(this);
        this->connected = true;
    }
}

KBrowserSocketObject::~KBrowserSocketObject() {
    unregisterBrowserSocket(this);
    if (this->browserSocket) {
        bw_net_close((int)this->browserSocket);
        this->browserSocket = 0;
    }
    {
        BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(this->readingCond);
        BOXEDWINE_CONDITION_SIGNAL_ALL(this->readingCond);
    }
    {
        BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(this->writingCond);
        BOXEDWINE_CONDITION_SIGNAL_ALL(this->writingCond);
    }
}

bool KBrowserSocketObject::hasBrowserSocket() const {
    return this->browserSocket != 0;
}

U32 KBrowserSocketObject::getBrowserSocket() const {
    return this->browserSocket;
}

void KBrowserSocketObject::signalBrowserEvents() {
    this->updateEvents();
    if (this->asyncProcessId && (this->eventMask & (K_POLLIN | K_POLLOUT | K_POLLERR | K_POLLHUP))) {
        KProcessPtr process = KSystem::getProcess(this->asyncProcessId);
        if (process) {
            U32 code = (this->eventMask & K_POLLIN) ? K_POLL_IN : K_POLL_OUT;
            S32 band = 0;
            if (this->eventMask & K_POLLERR) {
                code = K_POLL_ERR;
            } else if (this->eventMask & K_POLLHUP) {
                code = K_POLL_HUP;
            }
            BROWSER_NET_DEBUG_LOG("[boxedwine-net-cpp] async signal socket=%d fd=%d events=0x%x code=%d", this->browserSocket, this->asyncProcessFd, this->eventMask, code);
            process->signalIO(code, band, this->asyncProcessFd);
        }
    }
    if (this->eventMask & (K_POLLIN | K_POLLERR | K_POLLHUP)) {
        BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(this->readingCond);
        BOXEDWINE_CONDITION_SIGNAL_ALL(this->readingCond);
    }
    if (this->eventMask & (K_POLLOUT | K_POLLERR | K_POLLHUP)) {
        BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(this->writingCond);
        BOXEDWINE_CONDITION_SIGNAL_ALL(this->writingCond);
    }
}

U32 KBrowserSocketObject::networkUnavailable() {
    this->error = K_ENETUNREACH;
    return -K_ENETUNREACH;
}

U32 KBrowserSocketObject::updateEvents() {
    if (!this->browserSocket) {
        this->eventMask = 0;
        return 0;
    }
    this->eventMask = (U32)bw_net_get_events((int)this->browserSocket);
    S32 browserError = bw_net_get_error((int)this->browserSocket);
    if (browserError) {
        this->error = (U32)browserError;
    } else if (this->connecting && (this->eventMask & K_POLLOUT)) {
        this->connecting = false;
        this->connected = true;
        this->error = 0;
        BROWSER_NET_DEBUG_LOG("[boxedwine-net-cpp] readiness connect complete socket=%d events=0x%x", this->browserSocket, this->eventMask);
    }
    return this->eventMask;
}

void KBrowserSocketObject::refreshAddressMetadata() {
    if (!this->browserSocket) {
        return;
    }
    U32 peerIpv4 = bw_net_get_peer_ipv4((int)this->browserSocket);
    U16 peerPort = (U16)bw_net_get_peer_port((int)this->browserSocket);
    if (peerIpv4 || peerPort) {
        this->peerIpv4 = peerIpv4;
        this->peerPort = peerPort;
    }
    U32 localIpv4 = bw_net_get_local_ipv4((int)this->browserSocket);
    U16 localPort = (U16)bw_net_get_local_port((int)this->browserSocket);
    if (localIpv4 || localPort) {
        this->localIpv4 = localIpv4;
        this->localPort = localPort;
    }
}

U32 KBrowserSocketObject::finishBrowserResult(S32 result) {
    if (result < 0) {
        this->error = (U32)-result;
        return result;
    }
    this->error = 0;
    return (U32)result;
}

U32 KBrowserSocketObject::ioctl(KThread* thread, U32 request) {
    CPU* cpu = thread->cpu;

    if (request == 0x541b) {
        U32 value = this->browserSocket ? (U32)bw_net_readable_bytes((int)this->browserSocket) : 0;
        thread->memory->writed(IOCTL_ARG1, value);
        this->error = 0;
        BROWSER_NET_DEBUG_LOG("[boxedwine-net-cpp] ioctl FIONREAD process=%s browser=%d value=%d", thread->process->name.c_str(), this->browserSocket, value);
        return 0;
    } else if (request == 0x5421) {
        U32 value = thread->memory->readd(IOCTL_ARG1);
        this->blocking = value == 0;
        thread->memory->writed(IOCTL_ARG1, value);
        this->error = 0;
        BROWSER_NET_DEBUG_LOG("[boxedwine-net-cpp] ioctl FIONBIO process=%s browser=%d nonblocking=%d", thread->process->name.c_str(), this->browserSocket, value != 0);
        return 0;
    } else if (request == K_SIOCGIFCONF) {
        U32 address = IOCTL_ARG1;
        if (!address) {
            return -K_EFAULT;
        }
        U32 requestedLen = thread->memory->readd(address);
        U32 buf = thread->memory->readd(address + 4);
        if (!buf && requestedLen) {
            return -K_EFAULT;
        }

        const std::vector<KEmulatedNetworkInterface>& interfaces = getEmulatedNetworkInterfaces();
        U32 maxCount = requestedLen / K_IFREQ_SIZE;
        U32 count = std::min((U32)interfaces.size(), maxCount);
        for (U32 i = 0; i < count; i++) {
            writeIfReqNameAndAddr(thread->memory, buf + K_IFREQ_SIZE * i, interfaces[i]);
        }
        thread->memory->writed(address, count * K_IFREQ_SIZE);
        this->error = 0;
        return 0;
    } else if (request == K_SIOCGIFHWADDR) {
        U32 address = IOCTL_ARG1;
        const KEmulatedNetworkInterface* iface = getInterfaceFromIfReq(thread->memory, address);
        if (!iface) {
            return -K_ENODEV;
        }
        thread->memory->writew(address + K_IFNAMSIZ, iface->hardwareType);
        thread->memory->memcpy(address + K_IFNAMSIZ + 2, iface->mac, 6);
        thread->memory->memset(address + K_IFNAMSIZ + 8, 0, 8);
        this->error = 0;
        return 0;
    } else if (request == K_SIOCGIFADDR || request == K_SIOCGIFBRDADDR || request == K_SIOCGIFNETMASK) {
        U32 address = IOCTL_ARG1;
        const KEmulatedNetworkInterface* iface = getInterfaceFromIfReq(thread->memory, address);
        if (!iface) {
            return -K_ENODEV;
        }
        if (request == K_SIOCGIFADDR) {
            writeIfReqSockAddr(thread->memory, address, iface->ipv4);
        } else if (request == K_SIOCGIFBRDADDR) {
            writeIfReqSockAddr(thread->memory, address, iface->broadcast ? iface->broadcast : iface->ipv4);
        } else {
            writeIfReqSockAddr(thread->memory, address, iface->netmask);
        }
        this->error = 0;
        return 0;
    } else if (request == K_SIOCGIFFLAGS) {
        U32 address = IOCTL_ARG1;
        const KEmulatedNetworkInterface* iface = getInterfaceFromIfReq(thread->memory, address);
        if (!iface) {
            return -K_ENODEV;
        }
        thread->memory->writew(address + K_IFNAMSIZ, (U16)iface->flags);
        this->error = 0;
        return 0;
    } else if (request == K_SIOCGIFMTU) {
        U32 address = IOCTL_ARG1;
        const KEmulatedNetworkInterface* iface = getInterfaceFromIfReq(thread->memory, address);
        if (!iface) {
            return -K_ENODEV;
        }
        thread->memory->writed(address + K_IFNAMSIZ, iface->mtu);
        this->error = 0;
        return 0;
    } else if (request == K_SIOCGIFINDEX) {
        U32 address = IOCTL_ARG1;
        const KEmulatedNetworkInterface* iface = getInterfaceFromIfReq(thread->memory, address);
        if (!iface) {
            return -K_ENODEV;
        }
        thread->memory->writed(address + K_IFNAMSIZ, iface->index);
        this->error = 0;
        return 0;
    } else if (request == K_SIOCGIWNAME) {
        return -K_EOPNOTSUPP;
    }
    kwarn_fmt("KBrowserSocketObject::ioctl request=%x not implemented", request);
    return -K_ENOTTY;
}

S64 KBrowserSocketObject::seek(S64 pos) {
    return -K_ESPIPE;
}

S64 KBrowserSocketObject::length() {
    return -1;
}

S64 KBrowserSocketObject::getPos() {
    return 0;
}

void KBrowserSocketObject::setBlocking(bool blocking) {
    this->blocking = blocking;
    KThread* thread = KThread::currentThread();
    BROWSER_NET_DEBUG_LOG("[boxedwine-net-cpp] setBlocking process=%s browser=%d blocking=%d", thread ? thread->process->name.c_str() : "unknown", this->browserSocket, blocking ? 1 : 0);
}

bool KBrowserSocketObject::isBlocking() {
    return this->blocking;
}

void KBrowserSocketObject::setAsync(bool isAsync) {
    this->async = isAsync;
    KThread* thread = KThread::currentThread();
    if (isAsync && thread) {
        this->asyncProcessId = thread->process->id;
        this->asyncProcessFd = thread->cpu->reg[3].u32;
        BROWSER_NET_DEBUG_LOG("[boxedwine-net-cpp] async enabled socket=%d fd=%d process=%d", this->browserSocket, this->asyncProcessFd, this->asyncProcessId);
    } else if (!isAsync && thread && this->asyncProcessId == thread->process->id) {
        BROWSER_NET_DEBUG_LOG("[boxedwine-net-cpp] async disabled socket=%d fd=%d process=%d", this->browserSocket, this->asyncProcessFd, this->asyncProcessId);
        this->asyncProcessId = 0;
        this->asyncProcessFd = 0;
    }
}

bool KBrowserSocketObject::isAsync() {
    return this->async;
}

KFileLock* KBrowserSocketObject::getLock(KFileLock* lock) {
    kdebug("KBrowserSocketObject::getLock not implemented yet");
    return nullptr;
}

U32 KBrowserSocketObject::setLock(KFileLock* lock, bool wait) {
    kdebug("KBrowserSocketObject::setLock not implemented yet");
    return -1;
}

bool KBrowserSocketObject::supportsLocks() {
    return false;
}

bool KBrowserSocketObject::isOpen() {
    if (this->browserSocket) {
        return (this->updateEvents() & K_POLLHUP) == 0;
    }
    return this->listening || this->connected;
}

bool KBrowserSocketObject::isReadReady() {
    return (this->updateEvents() & K_POLLIN) != 0;
}

bool KBrowserSocketObject::isPriorityReadReady() {
    return false;
}

bool KBrowserSocketObject::isWriteReady() {
    return (this->updateEvents() & K_POLLOUT) != 0;
}

void KBrowserSocketObject::waitForEvents(BOXEDWINE_CONDITION& parentCondition, U32 events) {
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
}

U32 KBrowserSocketObject::writeNative(U8* buffer, U32 len) {
    if (!this->hasBrowserSocket()) {
        return this->networkUnavailable();
    }
    BROWSER_NET_DEBUG_LOG("[boxedwine-net-cpp] write socket=%d len=%d", this->browserSocket, len);
    while (true) {
        S32 result = bw_net_send((int)this->browserSocket, (U32)(uintptr_t)buffer, len, 0, 0, 0);
        if (result != -K_EWOULDBLOCK || !this->blocking) {
            return this->finishBrowserResult(result);
        }
        BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(this->writingCond);
        BOXEDWINE_CONDITION_WAIT(this->writingCond);
    }
}

U32 KBrowserSocketObject::readNative(U8* buffer, U32 len) {
    if (!this->hasBrowserSocket()) {
        return this->networkUnavailable();
    }
    while (true) {
        S32 result = bw_net_recv((int)this->browserSocket, (U32)(uintptr_t)buffer, len, 0, 0, 0);
        if (result != -K_EWOULDBLOCK || !this->blocking) {
            return this->finishBrowserResult(result);
        }
        BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(this->readingCond);
        BOXEDWINE_CONDITION_WAIT(this->readingCond);
    }
}

U32 KBrowserSocketObject::stat(KProcess* process, U32 address, bool is64) {
    KSystem::writeStat(process, B(""), address, is64, 1, 0, K_S_IFSOCK | K__S_IWRITE | K__S_IREAD, 0, 0, 4096, 0, 0, 1);
    return 0;
}

U32 KBrowserSocketObject::map(KThread* thread, U32 address, U32 len, S32 prot, S32 flags, U64 off) {
    return 0;
}

bool KBrowserSocketObject::canMap() {
    return false;
}

BString KBrowserSocketObject::selfFd() {
    return B("browser-socket:[") + this->browserSocket + B("]");
}

U32 KBrowserSocketObject::accept(KThread* thread, const KFileDescriptorPtr& fd, U32 address, U32 len, U32 flags) {
    if (!this->hasBrowserSocket()) {
        return this->networkUnavailable();
    }
    while (true) {
        S32 result = bw_net_accept((int)this->browserSocket);
        if (result > 0) {
            std::shared_ptr<KBrowserSocketObject> acceptedSocket = std::make_shared<KBrowserSocketObject>(this->domain, this->type, this->protocol, (U32)result);
            acceptedSocket->refreshAddressMetadata();
            KFileDescriptorPtr acceptedFd = thread->process->allocFileDescriptor(acceptedSocket, K_O_RDWR, 0, -1, 0);

            if (flags & FD_CLOEXEC) {
                acceptedFd->descriptorFlags |= FD_CLOEXEC;
            }
            if (flags & K_O_NONBLOCK) {
                acceptedFd->kobject->setBlocking(false);
            }
            if (address) {
                writeSockAddrIn(thread->memory, address, acceptedSocket->peerIpv4, acceptedSocket->peerPort);
            }
            if (len) {
                thread->memory->writed(len, 16);
            }
            this->error = 0;
            BROWSER_NET_DEBUG_LOG("[boxedwine-net-cpp] accept listener=%d accepted=%d fd=%d", this->browserSocket, result, acceptedFd->handle);
            return acceptedFd->handle;
        }
        if (result != -K_EWOULDBLOCK || !this->blocking) {
            return this->finishBrowserResult(result);
        }
        BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(this->readingCond);
        BOXEDWINE_CONDITION_WAIT(this->readingCond);
    }
}

U32 KBrowserSocketObject::bind(KThread* thread, const KFileDescriptorPtr& fd, U32 address, U32 len) {
    if (!this->hasBrowserSocket()) {
        return this->networkUnavailable();
    }
    if (!address || thread->memory->readw(address) != K_AF_INET) {
        return -K_EAFNOSUPPORT;
    }
    U32 ipv4 = readSockAddrIpv4(thread->memory, address);
    U16 port = readSockAddrPort(thread->memory, address);
    U32 result = this->finishBrowserResult(bw_net_bind((int)this->browserSocket, ipv4, port));
    if (result == 0) {
        this->localIpv4 = ipv4;
        this->localPort = port;
    }
    return result;
}

U32 KBrowserSocketObject::connect(KThread* thread, const KFileDescriptorPtr& fd, U32 address, U32 len) {
    if (!this->hasBrowserSocket()) {
        return this->networkUnavailable();
    }
    if (!address || thread->memory->readw(address) != K_AF_INET) {
        return -K_EAFNOSUPPORT;
    }
    U32 ipv4 = readSockAddrIpv4(thread->memory, address);
    U16 port = readSockAddrPort(thread->memory, address);
    this->peerIpv4 = ipv4;
    this->peerPort = port;
    while (true) {
        S32 result = bw_net_connect((int)this->browserSocket, ipv4, port);
        if (result == 0) {
            this->connecting = false;
            this->connected = true;
            BROWSER_NET_DEBUG_LOG("[boxedwine-net-cpp] connect complete socket=%d", this->browserSocket);
            return this->finishBrowserResult(result);
        }
        if (result != -K_EINPROGRESS || !this->blocking) {
            if (result == -K_EINPROGRESS) {
                this->connecting = true;
                BROWSER_NET_DEBUG_LOG("[boxedwine-net-cpp] connect pending socket=%d", this->browserSocket);
            }
            return this->finishBrowserResult(result);
        }
        this->connecting = true;
        BROWSER_NET_DEBUG_LOG("[boxedwine-net-cpp] connect blocking wait socket=%d", this->browserSocket);
        BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(this->writingCond);
        BOXEDWINE_CONDITION_WAIT(this->writingCond);
    }
}

U32 KBrowserSocketObject::getpeername(KThread* thread, const KFileDescriptorPtr& fd, U32 address, U32 plen) {
    if (!this->connected) {
        return -K_ENOTCONN;
    }
    this->refreshAddressMetadata();
    writeSockAddrIn(thread->memory, address, this->peerIpv4, this->peerPort);
    thread->memory->writed(plen, 16);
    this->error = 0;
    return 0;
}

U32 KBrowserSocketObject::getsockname(KThread* thread, const KFileDescriptorPtr& fd, U32 address, U32 plen) {
    this->refreshAddressMetadata();
    writeSockAddrIn(thread->memory, address, this->localIpv4, this->localPort);
    thread->memory->writed(plen, 16);
    this->error = 0;
    return 0;
}

U32 KBrowserSocketObject::getsockopt(KThread* thread, const KFileDescriptorPtr& fd, U32 level, U32 name, U32 value, U32 len_address) {
    KMemory* memory = thread->memory;
    U32 len = memory->readd(len_address);
    U32 retrievedValue = 0;

    if (level != K_SOL_SOCKET) {
        return -K_EINVAL;
    }
    if (name == K_SO_RCVBUF) {
        if (len != 4) {
            kpanic("KBrowserSocketObject::getsockopt SO_RCVBUF expecting len of 4");
        }
        retrievedValue = this->recvLen;
        memory->writed(value, retrievedValue);
    } else if (name == K_SO_SNDBUF) {
        if (len != 4) {
            kpanic("KBrowserSocketObject::getsockopt SO_SNDBUF expecting len of 4");
        }
        retrievedValue = this->sendLen;
        memory->writed(value, retrievedValue);
    } else if (name == K_SO_ERROR) {
        if (len != 4) {
            kpanic("KBrowserSocketObject::getsockopt SO_ERROR expecting len of 4");
        }
        if (this->browserSocket) {
            S32 browserError = bw_net_get_error((int)this->browserSocket);
            if (browserError) {
                this->error = (U32)browserError;
            } else if (this->connecting && (this->updateEvents() & K_POLLOUT)) {
                this->connecting = false;
                this->connected = true;
                this->error = 0;
                BROWSER_NET_DEBUG_LOG("[boxedwine-net-cpp] getsockopt SO_ERROR connect complete socket=%d", this->browserSocket);
            } else if (!this->connecting) {
                this->error = 0;
            }
        }
        retrievedValue = this->error;
        memory->writed(value, retrievedValue);
    } else if (name == K_SO_TYPE) {
        if (len != 4) {
            kpanic("KBrowserSocketObject::getsockopt SO_TYPE expecting len of 4");
        }
        retrievedValue = this->type;
        memory->writed(value, retrievedValue);
    } else if (name == K_SO_ACCEPTCONN || name == K_SO_BROADCAST || name == K_SO_OOBINLINE) {
        if (len != 4) {
            kpanic("KBrowserSocketObject::getsockopt boolean option expecting len of 4");
        }
        retrievedValue = name == K_SO_ACCEPTCONN && this->listening ? 1 : 0;
        memory->writed(value, retrievedValue);
    } else if (name == K_SO_PROTOCOL) {
        if (len != 4) {
            kpanic("KBrowserSocketObject::getsockopt SO_PROTOCOL expecting len of 4");
        }
        retrievedValue = this->protocol;
        memory->writed(value, retrievedValue);
    } else if (name == K_SO_RCVTIMEO || name == K_SO_SNDTIMEO) {
        if (len != 8) {
            kpanic("KBrowserSocketObject::getsockopt timeout expecting len of 8");
        }
        memory->writed(value, 0);
        memory->writed(value + 4, 0);
    } else {
        return -K_EINVAL;
    }
    memory->writed(len_address, len);
    this->error = 0;
    return 0;
}

U32 KBrowserSocketObject::listen(KThread* thread, const KFileDescriptorPtr& fd, U32 backlog) {
    if (!this->hasBrowserSocket()) {
        return this->networkUnavailable();
    }
    U32 result = this->finishBrowserResult(bw_net_listen((int)this->browserSocket, (int)backlog));
    if (result == 0) {
        this->listening = true;
        this->refreshAddressMetadata();
    }
    return result;
}

U32 KBrowserSocketObject::recvfrom(KThread* thread, const KFileDescriptorPtr& fd, U32 buffer, U32 length, U32 flags, U32 address, U32 address_len) {
    if (!this->hasBrowserSocket()) {
        return this->networkUnavailable();
    }
    if (length && !thread->memory->canWrite(buffer, length)) {
        return -K_EFAULT;
    }
    std::vector<U8> nativeBuffer(length);
    while (true) {
        S32 result = bw_net_recv((int)this->browserSocket, (U32)(uintptr_t)nativeBuffer.data(), length, (int)flags, address, address_len);
        if (result != -K_EWOULDBLOCK || !this->blocking) {
            if (result > 0) {
                thread->memory->memcpy(buffer, nativeBuffer.data(), (U32)result);
            }
            if ((S32)result >= 0 && this->type == K_SOCK_DGRAM) {
                writeLastRecvSockAddr(thread->memory, this->browserSocket, address, address_len);
            }
            return this->finishBrowserResult(result);
        }
        BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(this->readingCond);
        BOXEDWINE_CONDITION_WAIT(this->readingCond);
    }
}

U32 KBrowserSocketObject::recvmsg(KThread* thread, const KFileDescriptorPtr& fd, U32 address, U32 flags) {
    if (!this->hasBrowserSocket()) {
        return this->networkUnavailable();
    }

    KMemory* memory = thread->memory;
    MsgHdr hdr = {};
    readMsgHdr(thread, address, &hdr);

    U32 totalLen = 0;
    for (U32 i = 0; i < hdr.msg_iovlen; i++) {
        U32 p = memory->readd(hdr.msg_iov + 8 * i);
        U32 len = memory->readd(hdr.msg_iov + 8 * i + 4);
        if (len && !memory->canWrite(p, len)) {
            return -K_EFAULT;
        }
        totalLen += len;
    }

    std::vector<U8> nativeBuffer(totalLen);
    while (true) {
        S32 result = bw_net_recv((int)this->browserSocket, (U32)(uintptr_t)nativeBuffer.data(), totalLen, (int)flags, 0, 0);
        if (result != -K_EWOULDBLOCK || !this->blocking) {
            if (result > 0) {
                U32 copied = 0;
                for (U32 i = 0; i < hdr.msg_iovlen && copied < (U32)result; i++) {
                    U32 p = memory->readd(hdr.msg_iov + 8 * i);
                    U32 len = memory->readd(hdr.msg_iov + 8 * i + 4);
                    U32 toCopy = std::min(len, (U32)result - copied);
                    if (toCopy) {
                        memory->memcpy(p, nativeBuffer.data() + copied, toCopy);
                        copied += toCopy;
                    }
                }
            }
            if ((S32)result >= 0 && this->type == K_SOCK_DGRAM) {
                writeLastRecvSockAddr(memory, this->browserSocket, hdr.msg_name, hdr.msg_namelen ? address + 4 : 0);
            }
            if (this->type == K_SOCK_STREAM) {
                memory->writed(address + 4, 0);
            }
            memory->writed(address + 20, 0);
            return this->finishBrowserResult(result);
        }
        BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(this->readingCond);
        BOXEDWINE_CONDITION_WAIT(this->readingCond);
    }
}

U32 KBrowserSocketObject::sendmsg(KThread* thread, const KFileDescriptorPtr& fd, U32 address, U32 flags) {
    if (!this->hasBrowserSocket()) {
        return this->networkUnavailable();
    }

    KMemory* memory = thread->memory;
    MsgHdr hdr = {};
    readMsgHdr(thread, address, &hdr);
    BROWSER_NET_DEBUG_LOG("[boxedwine-net-cpp] sendmsg socket=%d iovlen=%d flags=0x%x", this->browserSocket, hdr.msg_iovlen, flags);

    if (hdr.msg_control || hdr.msg_controllen) {
        kwarn("KBrowserSocketObject::sendmsg control messages not implemented");
    }

    U32 totalLen = 0;
    for (U32 i = 0; i < hdr.msg_iovlen; i++) {
        U32 p = memory->readd(hdr.msg_iov + 8 * i);
        U32 len = memory->readd(hdr.msg_iov + 8 * i + 4);
        if (len && !memory->canRead(p, len)) {
            return -K_EFAULT;
        }
        totalLen += len;
    }

    std::vector<U8> nativeBuffer(totalLen);
    U32 copied = 0;
    for (U32 i = 0; i < hdr.msg_iovlen; i++) {
        U32 p = memory->readd(hdr.msg_iov + 8 * i);
        U32 len = memory->readd(hdr.msg_iov + 8 * i + 4);
        if (len) {
            memory->memcpy(nativeBuffer.data() + copied, p, len);
            copied += len;
        }
    }

    U32 ipv4 = 0;
    U16 port = 0;
    if (hdr.msg_name && hdr.msg_namelen) {
        if (memory->readw(hdr.msg_name) != K_AF_INET) {
            return -K_EAFNOSUPPORT;
        }
        ipv4 = readSockAddrIpv4(memory, hdr.msg_name);
        port = readSockAddrPort(memory, hdr.msg_name);
    }

    while (true) {
        S32 result = bw_net_send((int)this->browserSocket, (U32)(uintptr_t)nativeBuffer.data(), totalLen, (int)flags, ipv4, port);
        if (result != -K_EWOULDBLOCK || !this->blocking) {
            return this->finishBrowserResult(result);
        }
        BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(this->writingCond);
        BOXEDWINE_CONDITION_WAIT(this->writingCond);
    }
}

U32 KBrowserSocketObject::sendto(KThread* thread, const KFileDescriptorPtr& fd, U32 message, U32 length, U32 flags, U32 dest_addr, U32 dest_len) {
    if (!this->hasBrowserSocket()) {
        return this->networkUnavailable();
    }
    BROWSER_NET_DEBUG_LOG("[boxedwine-net-cpp] sendto socket=%d len=%d flags=0x%x", this->browserSocket, length, flags);
    if (length && !thread->memory->canRead(message, length)) {
        return -K_EFAULT;
    }
    std::vector<U8> nativeBuffer(length);
    if (length) {
        thread->memory->memcpy(nativeBuffer.data(), message, length);
    }
    U32 ipv4 = 0;
    U16 port = 0;
    if (dest_addr) {
        if (thread->memory->readw(dest_addr) != K_AF_INET) {
            return -K_EAFNOSUPPORT;
        }
        ipv4 = readSockAddrIpv4(thread->memory, dest_addr);
        port = readSockAddrPort(thread->memory, dest_addr);
    }
    while (true) {
        S32 result = bw_net_send((int)this->browserSocket, (U32)(uintptr_t)nativeBuffer.data(), length, (int)flags, ipv4, port);
        if (result != -K_EWOULDBLOCK || !this->blocking) {
            return this->finishBrowserResult(result);
        }
        BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(this->writingCond);
        BOXEDWINE_CONDITION_WAIT(this->writingCond);
    }
}

U32 KBrowserSocketObject::setsockopt(KThread* thread, const KFileDescriptorPtr& fd, U32 level, U32 name, U32 value, U32 len) {
    KMemory* memory = thread->memory;

    if (level == K_SOL_SOCKET) {
        if (name == K_SO_RCVBUF) {
            if (len != 4) {
                kpanic("KBrowserSocketObject::setsockopt SO_RCVBUF expecting len of 4");
            }
            this->recvLen = memory->readd(value);
        } else if (name == K_SO_SNDBUF) {
            if (len != 4) {
                kpanic("KBrowserSocketObject::setsockopt SO_SNDBUF expecting len of 4");
            }
            this->sendLen = memory->readd(value);
        } else if (name == K_SO_SNDTIMEO || name == K_SO_RCVTIMEO) {
            if (len != 8) {
                kpanic("KBrowserSocketObject::setsockopt timeout expecting len of 8");
            }
        } else if (name == K_SO_KEEPALIVE || name == K_SO_BROADCAST || name == K_SO_REUSEADDR || name == K_SO_REUSEPORT) {
            if (len != 4) {
                kpanic("KBrowserSocketObject::setsockopt boolean option expecting len of 4");
            }
        } else if (name == K_SO_LINGER) {
            if (len != 8) {
                kpanic("KBrowserSocketObject::setsockopt SO_LINGER expecting len of 8");
            }
        } else if (name != K_SO_TIMESTAMP) {
            kwarn_fmt("KBrowserSocketObject::setsockopt SOL_SOCKET name %d not implemented", name);
            return -K_EINVAL;
        }
    } else if (level == K_IPPROTO_IP) {
        if (name == K_IP_ADD_MEMBERSHIP || name == K_IP_DROP_MEMBERSHIP) {
            if (len != 8 && len != 12) {
                kpanic("KBrowserSocketObject::setsockopt multicast membership expecting len of 8 or 12");
            }
        } else if (name == K_IP_MULTICAST_IF) {
            if (len != 4 && len != 8 && len != 12) {
                kpanic("KBrowserSocketObject::setsockopt IP_MULTICAST_IF expecting len of 4, 8, or 12");
            }
        } else if (name == K_IP_MULTICAST_TTL || name == K_IP_MULTICAST_LOOP) {
            if (len != 1 && len != 4) {
                kpanic("KBrowserSocketObject::setsockopt multicast byte option expecting len of 1 or 4");
            }
        } else if (name == K_IP_TTL || name == K_IP_TOS || name == K_IP_RECVTOS || name == K_IP_RECVERR || name == K_IP_MTU_DISCOVER) {
            if (len != 4) {
                kpanic("KBrowserSocketObject::setsockopt IP option expecting len of 4");
            }
        } else {
            // Browser transports will apply IP options only when a backend supports them.
        }
    } else if (level == K_IPPROTO_TCP) {
        // Browser transports will apply TCP options only when a backend supports them.
    } else {
        kwarn_fmt("KBrowserSocketObject::setsockopt level %d not implemented", level);
        return -K_EINVAL;
    }
    if (this->browserSocket) {
        S32 result = bw_net_setsockopt((int)this->browserSocket, (int)level, (int)name, value, len);
        if (result < 0) {
            return this->finishBrowserResult(result);
        }
    }
    this->error = 0;
    return 0;
}

U32 KBrowserSocketObject::shutdown(KThread* thread, const KFileDescriptorPtr& fd, U32 how) {
    if (!this->hasBrowserSocket()) {
        return this->networkUnavailable();
    }
    U32 result = this->finishBrowserResult(bw_net_shutdown((int)this->browserSocket, (int)how));
    if (result == 0) {
        if (how == K_SHUT_RD || how == K_SHUT_RDWR) {
            this->inClosed = true;
        }
        if (how == K_SHUT_WR || how == K_SHUT_RDWR) {
            this->outClosed = true;
        }
    }
    return result;
}

#endif
