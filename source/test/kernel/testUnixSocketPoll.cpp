/*
 *  Copyright (C) 2012-2026  The BoxedWine Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 */

#include "boxedwine.h"

#ifdef __TEST

#include "../../util/ring_buffer.h"

#define private public
#include "kpoll.h"
#include "ksocket.h"
#include "knativesocket.h"
#include "kunixsocket.h"
#undef private

#include "../cpu/testCPU.h"

void testUnixSocketPollOutClearsPeerCondition() {
    std::shared_ptr<KUnixSocketObject> first = std::make_shared<KUnixSocketObject>(K_AF_UNIX, K_SOCK_STREAM, 0);
    std::shared_ptr<KUnixSocketObject> second = std::make_shared<KUnixSocketObject>(K_AF_UNIX, K_SOCK_STREAM, 0);
    BOXEDWINE_CONDITION pollCond = std::make_shared<BoxedWineCondition>(B("test poll condition"));

    first->connected = true;
    second->connected = true;
    first->connection = second;
    second->connection = first;

    first->waitForEvents(pollCond, K_POLLOUT);
    if (second->lockCond->parentsCount() != 1) {
        testFail("POLLOUT registration was not added to peer socket condition");
        return;
    }

    first->waitForEvents(pollCond, 0);
    if (second->lockCond->parentsCount() != 0) {
        testFail("POLLOUT cleanup left poll condition registered on peer socket");
    }
}

void testUnixSocketSendmsgStreamPayloadCanBeRead() {
    TestContext& context = testContext();
    KThread* thread = context.thread;
    KMemory* memory = context.memory;
    constexpr U32 MSG = TEST_HEAP_ADDRESS + 0x800;
    constexpr U32 IOV = TEST_HEAP_ADDRESS + 0x840;
    constexpr U32 DATA = TEST_HEAP_ADDRESS + 0x880;
    constexpr U32 OUT = TEST_HEAP_ADDRESS + 0x8c0;
    const char payload[] = "socket-stream";
    const U32 payloadLen = sizeof(payload) - 1;

    std::shared_ptr<KUnixSocketObject> first = std::make_shared<KUnixSocketObject>(K_AF_UNIX, K_SOCK_STREAM, 0);
    std::shared_ptr<KUnixSocketObject> second = std::make_shared<KUnixSocketObject>(K_AF_UNIX, K_SOCK_STREAM, 0);
    first->connected = true;
    second->connected = true;
    first->connection = second;
    second->connection = first;
    second->setBlocking(false);

    memory->memcpy(DATA, payload, payloadLen);
    memory->writed(IOV, DATA);
    memory->writed(IOV + 4, payloadLen);
    memory->writed(MSG, 0);
    memory->writed(MSG + 4, 0);
    memory->writed(MSG + 8, IOV);
    memory->writed(MSG + 12, 1);
    memory->writed(MSG + 16, 0);
    memory->writed(MSG + 20, 0);
    memory->writed(MSG + 24, 0);
    memory->memset(OUT, 0, payloadLen);

    KFileDescriptorPtr noDescriptor;
    U32 sendResult = first->sendmsg(thread, noDescriptor, MSG, 0);
    if (sendResult != payloadLen) {
        testFail("sendmsg stream payload expected %u, got %d (0x%X)", payloadLen, (S32)sendResult, sendResult);
    }

    U32 readResult = second->read(thread, OUT, payloadLen);
    if (readResult != payloadLen) {
        testFail("read after stream sendmsg expected %u, got %d (0x%X)", payloadLen, (S32)readResult, readResult);
    } else {
        char actual[sizeof(payload)] = {};
        memory->memcpy(actual, OUT, payloadLen);
        if (memcmp(actual, payload, payloadLen)) {
            testFail("read after stream sendmsg returned wrong payload");
        }
    }
    if (second->isReadReady()) {
        testFail("stream socket remained readable after read drained sendmsg payload");
    }
}

void testUnixSocketWritevInvalidSecondIovAfterZeroLengthFirstReturnsEfault() {
    TestContext& context = testContext();
    KThread* thread = context.thread;
    KMemory* memory = context.memory;
    constexpr U32 IOV = TEST_HEAP_ADDRESS + 0x900;
    constexpr U32 DATA = TEST_HEAP_ADDRESS + 0x940;
    constexpr U32 INVALID = 0xdeadbee0;

    std::shared_ptr<KUnixSocketObject> first = std::make_shared<KUnixSocketObject>(K_AF_UNIX, K_SOCK_STREAM, 0);
    std::shared_ptr<KUnixSocketObject> second = std::make_shared<KUnixSocketObject>(K_AF_UNIX, K_SOCK_STREAM, 0);
    first->connected = true;
    second->connected = true;
    first->connection = second;
    second->connection = first;

    memory->writed(IOV, DATA);
    memory->writed(IOV + 4, 0);
    memory->writed(IOV + 8, INVALID);
    memory->writed(IOV + 12, 2);

    U32 result = first->writev(thread, IOV, 2);
    if (result != (U32)-K_EFAULT) {
        testFail("writev invalid second iov after zero-length first expected EFAULT, got %d (0x%X)", (S32)result, result);
    }
    if (second->isReadReady()) {
        testFail("writev invalid second iov after zero-length first wrote data");
    }
}

void testUnixSocketWritevInvalidSecondIovDoesNotPartiallyWriteFirst() {
    TestContext& context = testContext();
    KThread* thread = context.thread;
    KMemory* memory = context.memory;
    constexpr U32 IOV = TEST_HEAP_ADDRESS + 0x980;
    constexpr U32 DATA = TEST_HEAP_ADDRESS + 0x9c0;
    constexpr U32 INVALID = 0xdeadbee0;
    const char payload[] = "hdr!";
    const U32 payloadLen = sizeof(payload) - 1;

    std::shared_ptr<KUnixSocketObject> first = std::make_shared<KUnixSocketObject>(K_AF_UNIX, K_SOCK_STREAM, 0);
    std::shared_ptr<KUnixSocketObject> second = std::make_shared<KUnixSocketObject>(K_AF_UNIX, K_SOCK_STREAM, 0);
    first->connected = true;
    second->connected = true;
    first->connection = second;
    second->connection = first;

    memory->memcpy(DATA, payload, payloadLen);
    memory->writed(IOV, DATA);
    memory->writed(IOV + 4, payloadLen);
    memory->writed(IOV + 8, INVALID);
    memory->writed(IOV + 12, 2);

    U32 result = first->writev(thread, IOV, 2);
    if (result != (U32)-K_EFAULT) {
        testFail("writev invalid second iov expected EFAULT without partial write, got %d (0x%X)", (S32)result, result);
    }
    if (second->isReadReady()) {
        testFail("writev invalid second iov partially wrote first iov");
    }
}

void testUnixSocketPendingConnectionsOnlyReadableForListeners() {
    std::shared_ptr<KUnixSocketObject> connected = std::make_shared<KUnixSocketObject>(K_AF_UNIX, K_SOCK_STREAM, 0);
    std::shared_ptr<KUnixSocketObject> peer = std::make_shared<KUnixSocketObject>(K_AF_UNIX, K_SOCK_STREAM, 0);
    std::shared_ptr<KUnixSocketObject> pending = std::make_shared<KUnixSocketObject>(K_AF_UNIX, K_SOCK_STREAM, 0);

    connected->connected = true;
    peer->connected = true;
    connected->connection = peer;
    peer->connection = connected;
    connected->pendingConnections.push_back(pending);
    connected->pendingConnectionCount++;

    if (connected->isReadReady()) {
        testFail("non-listening stream socket became readable from pending connection queue");
        return;
    }

    std::shared_ptr<KUnixSocketObject> listener = std::make_shared<KUnixSocketObject>(K_AF_UNIX, K_SOCK_STREAM, 0);
    listener->listening = true;
    listener->pendingConnections.push_back(pending);
    listener->pendingConnectionCount++;

    if (!listener->isReadReady()) {
        testFail("listening socket with pending connection was not readable");
    }
}

void testSoftRingBufferConcurrentWritersPreserveData() {
#ifdef BOXEDWINE_MULTI_THREADED
    constexpr U32 WRITER_COUNT = 8;
    constexpr U32 RECORDS_PER_WRITER = 4096;
    Soft_Ring_Buffer buffer(1);
    std::atomic<bool> start = false;
    std::vector<std::thread> writers;

    for (U32 writer = 0; writer < WRITER_COUNT; writer++) {
        writers.emplace_back([writer, &buffer, &start]() {
            while (!start.load(std::memory_order_acquire)) {
                std::this_thread::yield();
            }
            for (U32 record = 0; record < RECORDS_PER_WRITER; record++) {
                U64 value = ((U64)writer << 32) | record;
                buffer.put(value);
            }
        });
    }

    start.store(true, std::memory_order_release);
    for (std::thread& writer : writers) {
        writer.join();
    }

    constexpr size_t EXPECTED_SIZE = (size_t)WRITER_COUNT * RECORDS_PER_WRITER * sizeof(U64);
    if (buffer.size_used() != EXPECTED_SIZE) {
        testFail("concurrent soft ring buffer writers expected %zu bytes, got %zu", EXPECTED_SIZE, buffer.size_used());
    }
#endif
}

#ifdef BOXEDWINE_MULTI_THREADED
class PollSignalOnRegistrationUnixSocket : public KUnixSocketObject {
public:
    PollSignalOnRegistrationUnixSocket() : KUnixSocketObject(K_AF_UNIX, K_SOCK_STREAM, 0),
        eventCondition(std::make_shared<BoxedWineCondition>(B("PollSignalOnRegistrationUnixSocket::eventCondition"))) {
    }

    void waitForEvents(BOXEDWINE_CONDITION& parentCondition, U32 events) override {
        if (events) {
            BOXEDWINE_CONDITION_ADD_PARENT(eventCondition, parentCondition);
            BOXEDWINE_CONDITION_SIGNAL_ALL(eventCondition);
        } else {
            BOXEDWINE_CONDITION_REMOVE_PARENT(eventCondition, parentCondition);
        }
    }

private:
    BOXEDWINE_CONDITION eventCondition;
};
#endif

void testPollRegistrationCanSignalParentCondition() {
#ifdef BOXEDWINE_MULTI_THREADED
    TestContext& context = testContext();
    auto object = std::make_shared<PollSignalOnRegistrationUnixSocket>();
    KFileDescriptorPtr descriptor = context.process->allocFileDescriptor(object, K_O_RDWR, 0, -1, 0);
    KPollData pollData = {};
    pollData.fd = descriptor->handle;
    pollData.events = K_POLLIN;

    S32 result = internal_poll(context.thread, &pollData, 1, 0);
    if (result != 1 || pollData.revents != K_POLLHUP) {
        testFail("poll registration signal expected POLLHUP, got result=%d revents=0x%X", result, pollData.revents);
    }
    context.process->close(descriptor->handle);
#endif
}

void testNativeSocketSetOobInlineCanBeReadBack() {
#ifndef __EMSCRIPTEN__
    TestContext& context = testContext();
    KThread* thread = context.thread;
    KMemory* memory = context.memory;
    constexpr U32 OPTION = TEST_HEAP_ADDRESS + 0xa00;
    constexpr U32 RESULT = TEST_HEAP_ADDRESS + 0xa04;
    constexpr U32 RESULT_LEN = TEST_HEAP_ADDRESS + 0xa08;
    U32 socket = ksocket(K_AF_INET, K_SOCK_STREAM, K_IPPROTO_TCP);

    if ((S32)socket < 0) {
        testFail("native socket creation for SO_OOBINLINE failed with %d (0x%X)", (S32)socket, socket);
        return;
    }

    memory->writed(OPTION, 1);
    U32 setResult = ksetsockopt(thread, socket, K_SOL_SOCKET, K_SO_OOBINLINE, OPTION, 4);
    if (setResult != 0) {
        testFail("setting SO_OOBINLINE expected 0, got %d (0x%X)", (S32)setResult, setResult);
    } else {
        memory->writed(RESULT, 0);
        memory->writed(RESULT_LEN, 4);
        U32 getResult = kgetsockopt(thread, socket, K_SOL_SOCKET, K_SO_OOBINLINE, RESULT, RESULT_LEN);
        if (getResult != 0) {
            testFail("reading SO_OOBINLINE expected 0, got %d (0x%X)", (S32)getResult, getResult);
        } else if (memory->readd(RESULT) != 1) {
            testFail("SO_OOBINLINE expected 1 after setting it, got %u", memory->readd(RESULT));
        }
    }
    context.process->close(socket);
#endif
}

void testNativeSocketBindUnavailableAddressReturnsEaddrnotavail() {
#ifndef __EMSCRIPTEN__
    TestContext& context = testContext();
    KMemory* memory = context.memory;
    constexpr U32 ADDRESS = TEST_HEAP_ADDRESS + 0xa20;
    U32 socket = ksocket(K_AF_INET, K_SOCK_STREAM, K_IPPROTO_TCP);

    if ((S32)socket < 0) {
        testFail("native socket creation for unavailable-address bind failed with %d (0x%X)", (S32)socket, socket);
        return;
    }

    memory->memset(ADDRESS, 0, 16);
    memory->writew(ADDRESS, K_AF_INET);
    memory->writeb(ADDRESS + 4, 192);
    memory->writeb(ADDRESS + 5, 0);
    memory->writeb(ADDRESS + 6, 2);

    U32 bindResult = kbind(context.thread, socket, ADDRESS, 16);
    if (bindResult != (U32)-K_EADDRNOTAVAIL) {
        testFail("native bind to unavailable address expected EADDRNOTAVAIL, got %d (0x%X)", (S32)bindResult,
            bindResult);
    }
    context.process->close(socket);
#endif
}

// Native non-MT poll can yield from a timed wait, so poll zero-time readiness in a bounded host-side loop.
#ifndef __EMSCRIPTEN__
static bool waitForNativeSocketRead(TestContext& context, U32 socket, const char* description) {
    KPollData pollData = {};
    pollData.fd = socket;
    pollData.events = K_POLLIN;
    S32 pollResult = 0;
    for (U32 i = 0; i < 1000; ++i) {
        pollResult = internal_poll(context.thread, &pollData, 1, 0);
        if (pollResult != 0) {
            break;
        }
        Platform::nanoSleep(1000000);
    }
    if (pollResult != 1 || pollData.revents != K_POLLIN) {
        testFail("native %s poll expected POLLIN, got result=%d revents=0x%X", description, pollResult,
            pollData.revents);
        return false;
    }
    return true;
}

static bool createNativeTcpTestPair(TestContext& context, U32& client, U32& server) {
    KThread* thread = context.thread;
    KMemory* memory = context.memory;
    constexpr U32 LISTEN_ADDRESS = TEST_HEAP_ADDRESS + 0xa40;
    constexpr U32 LISTEN_ADDRESS_LEN = TEST_HEAP_ADDRESS + 0xa60;
    constexpr U32 PEER_ADDRESS = TEST_HEAP_ADDRESS + 0xa80;
    constexpr U32 PEER_ADDRESS_LEN = TEST_HEAP_ADDRESS + 0xaa0;
    U32 listener = (U32)-1;

    client = (U32)-1;
    server = (U32)-1;
    memory->memset(LISTEN_ADDRESS, 0, 16);
    memory->writew(LISTEN_ADDRESS, K_AF_INET);
    memory->writeb(LISTEN_ADDRESS + 4, 127);
    memory->writeb(LISTEN_ADDRESS + 7, 1);

    listener = ksocket(K_AF_INET, K_SOCK_STREAM, K_IPPROTO_TCP);
    if ((S32)listener < 0) {
        testFail("native TCP listener creation failed with %d (0x%X)", (S32)listener, listener);
        return false;
    }
    if (kbind(thread, listener, LISTEN_ADDRESS, 16) != 0) {
        testFail("native TCP listener bind failed");
        context.process->close(listener);
        return false;
    }
    if (klisten(thread, listener, 1) != 0) {
        testFail("native TCP listener listen failed");
        context.process->close(listener);
        return false;
    }

    memory->writed(LISTEN_ADDRESS_LEN, 16);
    if (kgetsockname(thread, listener, LISTEN_ADDRESS, LISTEN_ADDRESS_LEN) != 0) {
        testFail("native TCP listener getsockname failed");
        context.process->close(listener);
        return false;
    }

    client = ksocket(K_AF_INET, K_SOCK_STREAM, K_IPPROTO_TCP);
    if ((S32)client < 0) {
        testFail("native TCP client creation failed with %d (0x%X)", (S32)client, client);
        context.process->close(listener);
        return false;
    }
    KFileDescriptorPtr clientDescriptor = context.process->getFileDescriptor(client);
    clientDescriptor->kobject->setBlocking(false);
    U32 connectResult = kconnect(thread, client, LISTEN_ADDRESS, 16);
    if (connectResult != 0 && (S32)connectResult != -K_EINPROGRESS) {
        testFail("native TCP client connect failed with %d (0x%X)", (S32)connectResult, connectResult);
        context.process->close(client);
        context.process->close(listener);
        client = (U32)-1;
        return false;
    }

    if (!waitForNativeSocketRead(context, listener, "TCP listener")) {
        context.process->close(client);
        context.process->close(listener);
        client = (U32)-1;
        return false;
    }

    memory->memset(PEER_ADDRESS, 0, 16);
    memory->writed(PEER_ADDRESS_LEN, 16);
    server = kaccept(thread, listener, PEER_ADDRESS, PEER_ADDRESS_LEN, 0);
    context.process->close(listener);
    if ((S32)server < 0) {
        testFail("native TCP accept failed with %d (0x%X)", (S32)server, server);
        context.process->close(client);
        client = (U32)-1;
        return false;
    }
    return true;
}
#endif

void testNativeSocketRecvmsgReceivesOobData() {
#if defined(BOXEDWINE_MULTI_THREADED) && !defined(__EMSCRIPTEN__)
    TestContext& context = testContext();
    KThread* thread = context.thread;
    KMemory* memory = context.memory;
    constexpr U32 SEND_OOB = TEST_HEAP_ADDRESS + 0xac0;
    constexpr U32 SEND_NORMAL = TEST_HEAP_ADDRESS + 0xac4;
    constexpr U32 RECV_MESSAGE = TEST_HEAP_ADDRESS + 0xae0;
    constexpr U32 RECV_IOV = TEST_HEAP_ADDRESS + 0xb00;
    constexpr U32 RECV_DATA = TEST_HEAP_ADDRESS + 0xb20;
    U32 client;
    U32 server;

    if (!createNativeTcpTestPair(context, client, server)) {
        return;
    }

    memory->writeb(SEND_OOB, '!');
    memory->writeb(SEND_NORMAL, 'n');
    U32 sendOobResult = ksend(thread, client, SEND_OOB, 1, K_MSG_OOB);
    U32 sendNormalResult = ksend(thread, client, SEND_NORMAL, 1, 0);
    if (sendOobResult != 1 || sendNormalResult != 1) {
        testFail("native TCP send expected OOB=1 and normal=1, got OOB=%d and normal=%d",
            (S32)sendOobResult, (S32)sendNormalResult);
    } else {
        KFileDescriptorPtr serverDescriptor = context.process->getFileDescriptor(server);
        std::shared_ptr<KNativeSocketObject> serverSocket =
            serverDescriptor ? std::dynamic_pointer_cast<KNativeSocketObject>(serverDescriptor->kobject) : nullptr;
        for (U32 i = 0; serverSocket && !serverSocket->isPriorityReadReady() && i < 1000; ++i) {
            Platform::nanoSleep(1000000);
        }
        if (!serverSocket || !serverSocket->isPriorityReadReady()) {
            testFail("native TCP socket did not report priority data ready");
            context.process->close(server);
            context.process->close(client);
            return;
        }

        memory->writed(RECV_IOV, RECV_DATA);
        memory->writed(RECV_IOV + 4, 1);
        memory->writed(RECV_MESSAGE, 0);
        memory->writed(RECV_MESSAGE + 4, 0);
        memory->writed(RECV_MESSAGE + 8, RECV_IOV);
        memory->writed(RECV_MESSAGE + 12, 1);
        memory->writed(RECV_MESSAGE + 16, 0);
        memory->writed(RECV_MESSAGE + 20, 0);
        memory->writed(RECV_MESSAGE + 24, 0);
        memory->writeb(RECV_DATA, 0);

        U32 recvResult = krecvmsg(thread, server, RECV_MESSAGE, K_MSG_OOB);
        if (recvResult != 1) {
            testFail("recvmsg MSG_OOB expected 1, got %d (0x%X)", (S32)recvResult, recvResult);
        } else if (memory->readb(RECV_DATA) != '!') {
            testFail("recvmsg MSG_OOB expected urgent byte '!', got 0x%02X", memory->readb(RECV_DATA));
        }
    }

    context.process->close(server);
    context.process->close(client);
#endif
}

void testNativeDatagramSocketPollDoesNotReportHangup() {
#ifndef __EMSCRIPTEN__
    TestContext& context = testContext();
    KThread* thread = context.thread;
    KMemory* memory = context.memory;
    constexpr U32 ADDRESS = TEST_HEAP_ADDRESS + 0xb40;
    U32 socket = ksocket(K_AF_INET, K_SOCK_DGRAM, 17);

    if ((S32)socket < 0) {
        testFail("native UDP socket creation failed with %d (0x%X)", (S32)socket, socket);
        return;
    }

    KPollData pollData = {};
    pollData.fd = socket;
    pollData.events = K_POLLIN | K_POLLPRI | K_POLLOUT;
    S32 pollResult = internal_poll(thread, &pollData, 1, 0);
    if (pollResult != 1 || pollData.revents != K_POLLOUT) {
        testFail("fresh UDP poll expected POLLOUT, got result=%d revents=0x%X", pollResult, pollData.revents);
    }

    memory->memset(ADDRESS, 0, 16);
    memory->writew(ADDRESS, K_AF_INET);
    memory->writeb(ADDRESS + 4, 127);
    memory->writeb(ADDRESS + 7, 1);
    if (kbind(thread, socket, ADDRESS, 16) != 0) {
        testFail("native UDP bind failed");
    } else {
        pollData.events = K_POLLIN | K_POLLPRI | K_POLLOUT;
        pollResult = internal_poll(thread, &pollData, 1, 0);
        if (pollResult != 1 || pollData.revents != K_POLLOUT) {
            testFail("bound UDP poll expected POLLOUT, got result=%d revents=0x%X", pollResult, pollData.revents);
        }

        pollData.events = K_POLLIN | K_POLLPRI;
        pollResult = internal_poll(thread, &pollData, 1, 0);
        if (pollResult != 0 || pollData.revents != 0) {
            testFail("idle UDP read poll expected no events, got result=%d revents=0x%X", pollResult,
                pollData.revents);
        }
    }
    context.process->close(socket);
#endif
}

#ifndef __EMSCRIPTEN__
static bool createNativeUdpTestPair(TestContext& context, U32 address, U32 addressLen, U32& receiver, U32& sender) {
    KMemory* memory = context.memory;
    receiver = ksocket(K_AF_INET, K_SOCK_DGRAM, 17);
    sender = ksocket(K_AF_INET, K_SOCK_DGRAM, 17);
    if ((S32)receiver < 0 || (S32)sender < 0) {
        testFail("native UDP test socket creation failed receiver=%d sender=%d", (S32)receiver, (S32)sender);
        if ((S32)receiver >= 0) context.process->close(receiver);
        if ((S32)sender >= 0) context.process->close(sender);
        return false;
    }

    memory->memset(address, 0, 16);
    memory->writew(address, K_AF_INET);
    memory->writeb(address + 4, 127);
    memory->writeb(address + 7, 1);
    if (kbind(context.thread, receiver, address, 16) != 0) {
        testFail("native UDP test receiver bind failed");
        context.process->close(sender);
        context.process->close(receiver);
        return false;
    }

    memory->writed(addressLen, 16);
    if (kgetsockname(context.thread, receiver, address, addressLen) != 0) {
        testFail("native UDP test receiver getsockname failed");
        context.process->close(sender);
        context.process->close(receiver);
        return false;
    }

    KFileDescriptorPtr descriptor = context.process->getFileDescriptor(receiver);
    descriptor->kobject->setBlocking(false);
    return true;
}

static bool sendNativeUdpTestData(TestContext& context, U32 sender, U32 receiver, U32 address,
                                  U32 source, const char* data, U32 length) {
    context.memory->memcpy(source, data, length);
    U32 result = ksendto(context.thread, sender, source, length, 0, address, 16);
    if (result != length) {
        testFail("native UDP test send expected %u, got %d", length, (S32)result);
        return false;
    }

    return waitForNativeSocketRead(context, receiver, "UDP receiver");
}

static void prepareNativeRecvmsgScatter(KMemory* memory, U32 message, U32 iov, U32 data, U32 initialFlags) {
    memory->memset(data, static_cast<char>(0xcc), 8);
    memory->writed(iov, data);
    memory->writed(iov + 4, 2);
    memory->writed(iov + 8, data + 3);
    memory->writed(iov + 12, 4);
    memory->memset(message, 0, 28);
    memory->writed(message + 8, iov);
    memory->writed(message + 12, 2);
    memory->writed(message + 24, initialFlags);
}
#endif

void testNativeDatagramRecvmsgPeekScattersOnce() {
#ifndef __EMSCRIPTEN__
    TestContext& context = testContext();
    KMemory* memory = context.memory;
    constexpr U32 ADDRESS = TEST_HEAP_ADDRESS + 0xb80;
    constexpr U32 ADDRESS_LEN = TEST_HEAP_ADDRESS + 0xb90;
    constexpr U32 SOURCE = TEST_HEAP_ADDRESS + 0xba0;
    constexpr U32 MESSAGE = TEST_HEAP_ADDRESS + 0xbc0;
    constexpr U32 IOV = TEST_HEAP_ADDRESS + 0xbe0;
    constexpr U32 DATA = TEST_HEAP_ADDRESS + 0xc00;
    constexpr U32 PEEK_DATA = TEST_HEAP_ADDRESS + 0xc20;
    const char sendData[] = "data";
    U32 receiver;
    U32 sender;

    if (createNativeUdpTestPair(context, ADDRESS, ADDRESS_LEN, receiver, sender)) {
        if (sendNativeUdpTestData(context, sender, receiver, ADDRESS, SOURCE, sendData, sizeof(sendData) - 1)) {
            prepareNativeRecvmsgScatter(memory, MESSAGE, IOV, DATA, 0);
            U32 recvResult = krecvmsg(context.thread, receiver, MESSAGE, K_MSG_PEEK);
            if (recvResult != 4) testFail("native UDP recvmsg MSG_PEEK expected 4, got %d", (S32)recvResult);
            if (memory->readb(DATA) != 'd') testFail("native UDP recvmsg MSG_PEEK first byte was not 'd'");
            if (memory->readb(DATA + 1) != 'a') testFail("native UDP recvmsg MSG_PEEK second byte was not 'a'");
            if (memory->readb(DATA + 2) != 0xcc) testFail("native UDP recvmsg MSG_PEEK gap was modified");
            if (memory->readb(DATA + 3) != 't') testFail("native UDP recvmsg MSG_PEEK third byte was not 't'");
            if (memory->readb(DATA + 4) != 'a') testFail("native UDP recvmsg MSG_PEEK fourth byte was not 'a'");
            if (memory->readd(MESSAGE + 24) != 0) testFail("native UDP recvmsg MSG_PEEK set message flags");

            memory->memset(PEEK_DATA, 0, sizeof(sendData) - 1);
            U32 readResult = krecv(context.thread, receiver, PEEK_DATA, sizeof(sendData) - 1, 0);
            if (readResult != sizeof(sendData) - 1) {
                testFail("native UDP recv after MSG_PEEK expected 4, got %d", (S32)readResult);
            } else {
                char received[sizeof(sendData)] = {};
                memory->memcpy(received, PEEK_DATA, sizeof(sendData) - 1);
                if (memcmp(received, sendData, sizeof(sendData) - 1)) {
                    testFail("native UDP recv after MSG_PEEK did not return data");
                }
            }
        }
        context.process->close(sender);
        context.process->close(receiver);
    }
#endif
}

void testNativeDatagramRecvmsgScattersSingleMessage() {
#ifndef __EMSCRIPTEN__
    TestContext& context = testContext();
    KMemory* memory = context.memory;
    constexpr U32 ADDRESS = TEST_HEAP_ADDRESS + 0xb80;
    constexpr U32 ADDRESS_LEN = TEST_HEAP_ADDRESS + 0xb90;
    constexpr U32 SOURCE = TEST_HEAP_ADDRESS + 0xba0;
    constexpr U32 MESSAGE = TEST_HEAP_ADDRESS + 0xbc0;
    constexpr U32 IOV = TEST_HEAP_ADDRESS + 0xbe0;
    constexpr U32 DATA = TEST_HEAP_ADDRESS + 0xc00;
    const char sendData[] = "data";
    U32 receiver;
    U32 sender;

    if (createNativeUdpTestPair(context, ADDRESS, ADDRESS_LEN, receiver, sender)) {
        if (sendNativeUdpTestData(context, sender, receiver, ADDRESS, SOURCE, sendData, sizeof(sendData))) {
            prepareNativeRecvmsgScatter(memory, MESSAGE, IOV, DATA, 0);
            U32 recvResult = krecvmsg(context.thread, receiver, MESSAGE, 0);
            if (recvResult != 5) testFail("native UDP recvmsg expected 5, got %d", (S32)recvResult);
            if (memory->readb(DATA) != 'd') testFail("native UDP recvmsg first byte was not 'd'");
            if (memory->readb(DATA + 1) != 'a') testFail("native UDP recvmsg second byte was not 'a'");
            if (memory->readb(DATA + 2) != 0xcc) testFail("native UDP recvmsg gap was modified");
            if (memory->readb(DATA + 3) != 't') testFail("native UDP recvmsg third byte was not 't'");
            if (memory->readb(DATA + 4) != 'a') testFail("native UDP recvmsg fourth byte was not 'a'");
            if (memory->readb(DATA + 5) != 0) testFail("native UDP recvmsg terminator was not received");
            if (memory->readd(MESSAGE + 24) != 0) testFail("native UDP recvmsg set message flags");
        }
        context.process->close(sender);
        context.process->close(receiver);
    }
#endif
}

void testNativeDatagramRecvmsgReportsTruncation() {
#ifndef __EMSCRIPTEN__
    TestContext& context = testContext();
    KMemory* memory = context.memory;
    constexpr U32 ADDRESS = TEST_HEAP_ADDRESS + 0xb80;
    constexpr U32 ADDRESS_LEN = TEST_HEAP_ADDRESS + 0xb90;
    constexpr U32 SOURCE = TEST_HEAP_ADDRESS + 0xba0;
    constexpr U32 MESSAGE = TEST_HEAP_ADDRESS + 0xbc0;
    constexpr U32 IOV = TEST_HEAP_ADDRESS + 0xbe0;
    constexpr U32 DATA = TEST_HEAP_ADDRESS + 0xc00;
    const char sendData[] = "moredata";
    U32 receiver;
    U32 sender;

    if (createNativeUdpTestPair(context, ADDRESS, ADDRESS_LEN, receiver, sender)) {
        if (sendNativeUdpTestData(context, sender, receiver, ADDRESS, SOURCE, sendData, sizeof(sendData))) {
            prepareNativeRecvmsgScatter(memory, MESSAGE, IOV, DATA, 0);
            U32 recvResult = krecvmsg(context.thread, receiver, MESSAGE, 0);
            if (recvResult != 6) testFail("native UDP recvmsg truncation expected 6, got %d", (S32)recvResult);
            if (memory->readb(DATA) != 'm') testFail("native UDP recvmsg truncation first byte was not 'm'");
            if (memory->readb(DATA + 1) != 'o') testFail("native UDP recvmsg truncation second byte was not 'o'");
            if (memory->readb(DATA + 2) != 0xcc) testFail("native UDP recvmsg truncation gap was modified");
            if (memory->readb(DATA + 3) != 'r') testFail("native UDP recvmsg truncation third byte was not 'r'");
            if (memory->readb(DATA + 4) != 'e') testFail("native UDP recvmsg truncation fourth byte was not 'e'");
            if (memory->readb(DATA + 5) != 'd') testFail("native UDP recvmsg truncation fifth byte was not 'd'");
            if (memory->readb(DATA + 6) != 'a') testFail("native UDP recvmsg truncation sixth byte was not 'a'");
            if (memory->readd(MESSAGE + 24) != K_MSG_TRUNC) {
                testFail("native UDP recvmsg truncation expected MSG_TRUNC, got 0x%X", memory->readd(MESSAGE + 24));
            }
        }
        context.process->close(sender);
        context.process->close(receiver);
    }
#endif
}

void testNativeStreamRecvmsgZeroIovDoesNotConsumeData() {
#ifndef __EMSCRIPTEN__
    TestContext& context = testContext();
    KThread* thread = context.thread;
    KMemory* memory = context.memory;
    constexpr U32 SEND_DATA = TEST_HEAP_ADDRESS + 0xc40;
    constexpr U32 RECV_DATA = TEST_HEAP_ADDRESS + 0xc50;
    constexpr U32 MESSAGE = TEST_HEAP_ADDRESS + 0xc60;
    U32 client;
    U32 server;

    if (!createNativeTcpTestPair(context, client, server)) {
        return;
    }

    memory->writeb(SEND_DATA, 'z');
    U32 sendResult = ksend(thread, client, SEND_DATA, 1, 0);
    if (sendResult != 1) {
        testFail("native TCP zero-iov setup send expected 1, got %d", (S32)sendResult);
    } else if (waitForNativeSocketRead(context, server, "TCP server")) {
        KFileDescriptorPtr serverDescriptor = context.process->getFileDescriptor(server);
        std::shared_ptr<KNativeSocketObject> serverSocket =
            serverDescriptor ? std::dynamic_pointer_cast<KNativeSocketObject>(serverDescriptor->kobject) : nullptr;
        if (!serverSocket) {
            testFail("native TCP zero-iov recvmsg could not access accepted native socket");
        } else {
            serverSocket->error = K_EIO;
            memory->writed(MESSAGE, TEST_HEAP_ADDRESS + 0xc80);
            memory->writed(MESSAGE + 4, 0x11111111);
            memory->writed(MESSAGE + 8, 0);
            memory->writed(MESSAGE + 12, 0);
            memory->writed(MESSAGE + 16, TEST_HEAP_ADDRESS + 0xc90);
            memory->writed(MESSAGE + 20, 0x22222222);
            memory->writed(MESSAGE + 24, 0x33333333);

            U32 recvmsgResult = krecvmsg(thread, server, MESSAGE, 0);
            if (recvmsgResult != 0) {
                testFail("native TCP zero-iov recvmsg expected 0, got %d (0x%X)", (S32)recvmsgResult,
                    recvmsgResult);
            }
            if (memory->readd(MESSAGE + 4) != 0) {
                testFail("native TCP zero-iov recvmsg did not clear msg_namelen");
            }
            if (memory->readd(MESSAGE + 20) != 0) {
                testFail("native TCP zero-iov recvmsg did not clear msg_controllen");
            }
            if (memory->readd(MESSAGE + 24) != 0) {
                testFail("native TCP zero-iov recvmsg did not clear msg_flags");
            }
            if (serverSocket->error != 0) {
                testFail("native TCP zero-iov recvmsg did not clear socket error");
            }

            memory->writeb(RECV_DATA, 0);
            U32 recvResult = krecv(thread, server, RECV_DATA, 1, 0);
            if (recvResult != 1) {
                testFail("native TCP recv after zero-iov recvmsg expected 1, got %d", (S32)recvResult);
            } else if (memory->readb(RECV_DATA) != 'z') {
                testFail("native TCP zero-iov recvmsg consumed queued stream data");
            }
        }
    }

    context.process->close(server);
    context.process->close(client);
#endif
}

#endif
