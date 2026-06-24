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

#define private public
#include "ksocket.h"
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

    if (connected->isReadReady()) {
        testFail("non-listening stream socket became readable from pending connection queue");
        return;
    }

    std::shared_ptr<KUnixSocketObject> listener = std::make_shared<KUnixSocketObject>(K_AF_UNIX, K_SOCK_STREAM, 0);
    listener->listening = true;
    listener->pendingConnections.push_back(pending);

    if (!listener->isReadReady()) {
        testFail("listening socket with pending connection was not readable");
    }
}

#endif
