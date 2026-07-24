/*
 *  Copyright (C) 2012-2026  The BoxedWine Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 */

#include "boxedwine.h"
#include "kinotify.h"
#include "kpoll.h"
#include "ksignal.h"
#include "kstat.h"
#include "../../io/fsfilenode.h"
#include "../../io/fsmemnode.h"
#include "../../io/fsmemopennode.h"
#include "../../util/bnativeheap.h"
#ifdef BOXEDWINE_JIT
#include "../../emulation/cpu/jit/jitCodeGen.h"
#ifdef BOXEDWINE_WASM_JIT
#include "../../emulation/cpu/wasm/jitWasmCodeGen.h"
#include "../../emulation/cpu/wasm/wasmJitBatchPolicy.h"
#endif
#endif

#ifdef __TEST

#include "../cpu/testCPU.h"
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <filesystem>
#include <mutex>
#include <thread>

namespace {

constexpr U32 STAT_A = TEST_HEAP_ADDRESS + 0x100;
constexpr U32 STAT_B = TEST_HEAP_ADDRESS + 0x200;
constexpr U32 BUFFER = TEST_HEAP_ADDRESS + 0x300;
constexpr U32 TIMES = TEST_HEAP_ADDRESS + 0x400;
constexpr U32 UTIME_OMIT = 0x3ffffffe;
constexpr U32 TEST_CLONE_VM = 0x00000100;

struct NoThrowCodeWriteTestContext {
    U8* address;
    U32 value;
};

void writeNoThrowCodeForTest(void* context) noexcept {
    NoThrowCodeWriteTestContext* writeContext =
        static_cast<NoThrowCodeWriteTestContext*>(context);
    memcpy(writeContext->address, &writeContext->value,
        sizeof(writeContext->value));
}

void expectZero(const char* label, U32 value) {
    if (value) {
        testFail("%s expected 0, got %d (0x%X)", label, (S32)value, value);
    }
}

void expectU32(const char* label, U32 actual, U32 expected) {
    if (actual != expected) {
        testFail("%s expected %u (0x%X), got %u (0x%X)", label, expected, expected, actual, actual);
    }
}

void expectBytes(const char* label, const std::vector<U8>& actual, const U8* expected, U32 expectedLen) {
    if (actual.size() != expectedLen || memcmp(actual.data(), expected, expectedLen)) {
        testFail("%s bytes did not match", label);
    }
}

U32 stat64Inode(KMemory* memory, U32 address) {
    return memory->readd(address + 12);
}

U32 stat64LinkCount(KMemory* memory, U32 address) {
    return memory->readd(address + 20);
}

U32 stat64AccessSeconds(KMemory* memory, U32 address) {
    return memory->readd(address + 64);
}

U32 stat64AccessNanos(KMemory* memory, U32 address) {
    return memory->readd(address + 68);
}

void cleanupRoot(const BString& root) {
    KSystem::setProcNode(nullptr);
    Fs::shutDown();
    Fs::deleteNativeDirAndAllFilesInDir(root);
}

void initTestFileSystem(const BString& root) {
    std::filesystem::path rootPath(root.c_str());
    std::filesystem::path parentPath = rootPath.parent_path();
    if (!parentPath.empty()) {
        std::error_code error;
        std::filesystem::create_directories(parentPath, error);
        if (error) {
            testFail("native root parent %s could not be created: %s", parentPath.string().c_str(), error.message().c_str());
        }
    }
    Fs::initFileSystem(root);
}

void setTestProcNode() {
    KSystem::setProcNode(
        Fs::getNodeFromLocalPath(B(""), B("/proc"), false));
}

KProcessPtr createProcessWithMemory(KThread** thread) {
    KProcessPtr process = KProcess::create();
    process->memory = KMemory::create(process.get());
    *thread = process->createThread();
    return process;
}

std::shared_ptr<FsNode> addRegularFile(const BString& path) {
    std::shared_ptr<FsNode> parent = Fs::getNodeFromLocalPath(B(""), Fs::getParentPath(path), false);
    if (!parent) {
        return nullptr;
    }
    BString fileName = Fs::getFileNameFromPath(path);
    return Fs::addFileNode(path, B(""), Fs::getNativePathFromParentAndLocalFilename(parent, fileName), false, parent);
}

void expectU64(const char* label, U64 actual, U64 expected) {
    if (actual != expected) {
        testFail("%s expected %llu (0x%llX), got %llu (0x%llX)", label,
            (unsigned long long)expected, (unsigned long long)expected,
            (unsigned long long)actual, (unsigned long long)actual);
    }
}

class TestEvent {
public:
    void signal() {
        std::lock_guard<std::mutex> lock(mutex);
        signaled = true;
        condition.notify_all();
    }

    bool wait(U32 timeoutMs) {
        std::unique_lock<std::mutex> lock(mutex);
        return condition.wait_for(lock, std::chrono::milliseconds(timeoutMs), [this]() { return signaled; });
    }

private:
    std::mutex mutex;
    std::condition_variable condition;
    bool signaled = false;
};

class TestPausePoint {
public:
    void pause() {
        std::unique_lock<std::mutex> lock(mutex);
        reached = true;
        condition.notify_all();
        condition.wait(lock, [this]() { return released; });
    }

    bool waitUntilReached(U32 timeoutMs) {
        std::unique_lock<std::mutex> lock(mutex);
        return condition.wait_for(lock, std::chrono::milliseconds(timeoutMs), [this]() { return reached; });
    }

    void release() {
        std::lock_guard<std::mutex> lock(mutex);
        released = true;
        condition.notify_all();
    }

private:
    std::mutex mutex;
    std::condition_variable condition;
    bool reached = false;
    bool released = false;
};

class TestBarrier {
public:
    bool arriveAndWait(U32 participantCount, U32 timeoutMs) {
        std::unique_lock<std::mutex> lock(mutex);
        ++arrived;
        condition.notify_all();
        return condition.wait_for(lock, std::chrono::milliseconds(timeoutMs),
            [&]() { return arrived >= participantCount; });
    }

private:
    std::mutex mutex;
    std::condition_variable condition;
    U32 arrived = 0;
};

class RehashThrowValue {
public:
    RehashThrowValue() = default;

    explicit RehashThrowValue(U32 value) : value(value) {
    }

    RehashThrowValue(const RehashThrowValue& other) : value(other.value) {
        if (failTransfer) {
            throw std::bad_alloc();
        }
    }

    RehashThrowValue(RehashThrowValue&& other) : value(other.value) {
        if (failTransfer) {
            throw std::bad_alloc();
        }
    }

    RehashThrowValue& operator=(const RehashThrowValue&) = default;
    RehashThrowValue& operator=(RehashThrowValue&&) = default;

    static bool failTransfer;
    U32 value = 0;
};

bool RehashThrowValue::failTransfer = false;

class BHashConstructorThrowValue {
public:
    BHashConstructorThrowValue() {
        if (failConstruction) {
            throw std::bad_alloc();
        }
    }

    static bool failConstruction;
};

bool BHashConstructorThrowValue::failConstruction = false;

class BHashThrowingMoveValue {
public:
    BHashThrowingMoveValue() = default;
    BHashThrowingMoveValue(const BHashThrowingMoveValue&) = default;
    BHashThrowingMoveValue(BHashThrowingMoveValue&&) noexcept(false) {}
    BHashThrowingMoveValue& operator=(const BHashThrowingMoveValue&) = default;
    BHashThrowingMoveValue& operator=(BHashThrowingMoveValue&&) noexcept(false) {
        return *this;
    }
};

class PositionedReadTestOpenNode final : public FsOpenNode {
public:
    static constexpr S64 HIGH_OFFSET = 0x100000000000LL;

    explicit PositionedReadTestOpenNode(const std::shared_ptr<FsNode>& node)
        : FsOpenNode(node, K_O_RDWR) {}

    S64 length() override { return HIGH_OFFSET + 16; }
    bool setLength(S64) override { return true; }
    S64 getFilePointer() override {
        ++getPositionCalls;
        if (failNextGetFilePointer) {
            failNextGetFilePointer = false;
            return -1;
        }
        return position;
    }
    S64 seek(S64 pos) override {
        ++seekCalls;
        if (failNextSeek) {
            failNextSeek = false;
            return -1;
        }
        if (pos == failSeekPosition) {
            failSeekPosition = std::numeric_limits<S64>::min();
            return -1;
        }
        position = pos;
        return position;
    }
    U32 map(KThread*, U32, U32, S32, S32, U64) override { return 0; }
    bool canMap() override { return true; }
    U32 ioctl(KThread*, U32) override { return -K_EINVAL; }
    void setAsync(bool value) override { async = value; }
    bool isAsync() override { return async; }
    void waitForEvents(BOXEDWINE_CONDITION&, U32) override {}
    bool isWriteReady() override { return true; }
    bool isReadReady() override { return true; }
    U32 readNative(U8* buffer, U32 len) override {
        ++readCalls;
        for (U32 i = 0; i < len; ++i) {
            S64 current = position + i;
            buffer[i] = current >= HIGH_OFFSET && current < HIGH_OFFSET + 4
                ? highBytes[(size_t)(current - HIGH_OFFSET)]
                : priorCursorByte;
        }
        position += len;
        return len;
    }
    U32 writeNative(U8*, U32) override { return -K_EIO; }
    void close() override { open = false; }
    void reopen() override { open = true; }
    bool isOpen() override { return open; }

    S64 position = 0;
    bool failNextSeek = false;
    bool failNextGetFilePointer = false;
    S64 failSeekPosition = std::numeric_limits<S64>::min();
    U32 getPositionCalls = 0;
    U32 seekCalls = 0;
    U32 readCalls = 0;
    U8 priorCursorByte = 0x31;
    U8 highBytes[4] = {0x91, 0x82, 0x73, 0x64};

private:
    bool async = false;
    bool open = true;
};

class RetirementWriteTestOpenNode final : public FsOpenNode {
public:
    enum class FailureMode {
        None,
        Error,
        ShortWrite
    };

    explicit RetirementWriteTestOpenNode(const std::shared_ptr<FsNode>& node)
        : FsOpenNode(node, K_O_RDWR), bytes(K_PAGE_SIZE, 0) {}

    S64 length() override { return (S64)bytes.size(); }
    bool setLength(S64 len) override {
        if (len < 0) {
            return false;
        }
        bytes.resize((size_t)len);
        if (position > len) {
            position = len;
        }
        return true;
    }
    S64 getFilePointer() override { return position; }
    S64 seek(S64 pos) override {
        if (pos < 0) {
            return -1;
        }
        position = pos;
        return position;
    }
    U32 map(KThread*, U32, U32, S32, S32, U64) override { return 0; }
    bool canMap() override { return true; }
    U32 ioctl(KThread*, U32) override { return -K_EINVAL; }
    void setAsync(bool value) override { async = value; }
    bool isAsync() override { return async; }
    void waitForEvents(BOXEDWINE_CONDITION&, U32) override {}
    bool isWriteReady() override { return true; }
    bool isReadReady() override { return true; }
    U32 readNative(U8* buffer, U32 len) override {
        if (position < 0 || (U64)position >= bytes.size()) {
            return 0;
        }
        U32 count = (U32)std::min<U64>(len, bytes.size() - (U64)position);
        memcpy(buffer, bytes.data() + (size_t)position, count);
        position += count;
        return count;
    }
    U32 writeNative(U8* buffer, U32 len) override {
        FsWriteResult result = writeNativeAt(buffer, (U64)position, len);
        position += (S64)result.bytesWritten;
        return result.error ? (U32)result.error : (U32)result.bytesWritten;
    }
    FsWriteResult writeNativeAt(U8* buffer, U64 offset, U32 len) override {
        ++writeCalls;
        FsWriteResult result;
        if (failureMode == FailureMode::Error) {
            result.error = -K_EIO;
            return result;
        }
        if (offset >= bytes.size()) {
            return result;
        }
        U32 available = (U32)std::min<U64>(len, bytes.size() - offset);
        U32 count = available;
        if (failureMode == FailureMode::ShortWrite) {
            count = std::min(count, shortWriteBytes);
        }
        if (count) {
            memcpy(bytes.data() + (size_t)offset, buffer, count);
        }
        result.bytesWritten = count;
        if (beforeWriteReturn) {
            beforeWriteReturn();
        }
        return result;
    }
    void close() override { open = false; }
    void reopen() override { open = true; }
    bool isOpen() override { return open; }

    std::vector<U8> bytes;
    FailureMode failureMode = FailureMode::None;
    U32 shortWriteBytes = 0;
    U32 writeCalls = 0;
    std::function<void()> beforeWriteReturn;

private:
    S64 position = 0;
    bool async = false;
    bool open = true;
};

} // namespace

void testFileCacheIdentitySurvivesRenameAndHardLink() {
    TestContext& context = testContext();
    const BString root = B("tmp/test-file-cache-identity-root");
    cleanupRoot(root);
    initTestFileSystem(root);
    Fs::makeLocalDirs(B("/tmp"));

    std::shared_ptr<FsNode> source = addRegularFile(B("/tmp/source"));
    if (!source) {
        testFail("file-cache identity source was not created");
        cleanupRoot(root);
        return;
    }
    FsOpenNode* openNode = source->open(K_O_CREAT | K_O_RDWR);
    if (!openNode) {
        testFail("file-cache identity source could not be opened");
        cleanupRoot(root);
        return;
    }
    delete openNode;
    std::shared_ptr<FsFileIdentity> identity = source->getFileIdentity();
    expectZero("rename identity source", context.process->rename(B("/tmp/source"), B("/tmp/renamed")));
    source = Fs::getNodeFromLocalPath(B(""), B("/tmp/renamed"), false);
    if (!source || source->getFileIdentity() != identity) {
        testFail("rename changed runtime file identity");
    }

    expectZero("link identity source", context.process->link(B("/tmp/renamed"), B("/tmp/alias")));
    std::shared_ptr<FsNode> alias = Fs::getNodeFromLocalPath(B(""), B("/tmp/alias"), false);
    if (!source || !alias || source->getFileIdentity() != alias->getFileIdentity()) {
        testFail("hard-link aliases did not share runtime file identity");
    }

    std::shared_ptr<FsMemNode> memA = std::make_shared<FsMemNode>(1, 1, B("same-name"));
    std::shared_ptr<FsMemNode> memB = std::make_shared<FsMemNode>(1, 1, B("same-name"));
    if (memA->getFileIdentity() == memB->getFileIdentity()) {
        testFail("distinct memory files shared runtime file identity");
    }
    cleanupRoot(root);
}

void testHardLinksShareIdentityDataAndXattrs() {
    TestContext& context = testContext();
    KProcessPtr process = context.process;
    KMemory* memory = context.memory;

    BString root = B("tmp/test-hardlinks-root");
    cleanupRoot(root);
    initTestFileSystem(root);
    Fs::makeLocalDirs(B("/tmp"));

    const char original[] = "source";
    std::shared_ptr<FsNode> nodeA = addRegularFile(B("/tmp/hardlink-a"));
    if (!nodeA) {
        testFail("source node was not created");
        cleanupRoot(root);
        return;
    }
    FsOpenNode* openNode = nodeA->open(K_O_CREAT | K_O_RDWR);
    if (!openNode) {
        testFail("source file could not be opened");
        cleanupRoot(root);
        return;
    }
    expectU32("write source", openNode->writeNative((U8*)original, sizeof(original) - 1), sizeof(original) - 1);
    delete openNode;

    const U8 reparseData[] = { 0x11, 0x22, 0x00, 0x33, 0x44 };
    expectZero("set source xattr", Fs::setXAttr(nodeA, B("user.WINEREPARSE"), reparseData, sizeof(reparseData)));
    FsOpenNode* heldSourceOpen = nodeA->open(K_O_RDONLY);
    if (!heldSourceOpen) {
        testFail("source file could not be held open before link");
        cleanupRoot(root);
        return;
    }
    expectZero("link source", process->link(B("/tmp/hardlink-a"), B("/tmp/hardlink-b")));

    expectZero("stat source", process->stat64(B("/tmp/hardlink-a"), STAT_A));
    expectZero("stat link", process->stat64(B("/tmp/hardlink-b"), STAT_B));
    expectU32("source link count", stat64LinkCount(memory, STAT_A), 2);
    expectU32("link link count", stat64LinkCount(memory, STAT_B), 2);
    expectU32("shared inode", stat64Inode(memory, STAT_B), stat64Inode(memory, STAT_A));

    std::shared_ptr<FsNode> nodeB = Fs::getNodeFromLocalPath(B(""), B("/tmp/hardlink-b"), false);
    if (!nodeB) {
        testFail("link node was not created");
        cleanupRoot(root);
        return;
    }

    std::vector<U8> value;
    expectZero("get link xattr", Fs::getXAttr(nodeB, B("user.WINEREPARSE"), value));
    expectBytes("shared xattr", value, reparseData, sizeof(reparseData));
    char heldData[sizeof(original) - 1];
    memset(heldData, 0, sizeof(heldData));
    expectU32("read held source after link", heldSourceOpen->readNative((U8*)heldData, sizeof(heldData)), sizeof(heldData));
    if (memcmp(heldData, original, sizeof(heldData))) {
        testFail("held source handle did not survive hard link conversion");
    }
    delete heldSourceOpen;

    nodeA = nullptr;
    nodeB = nullptr;
    Fs::shutDown();
    initTestFileSystem(root);

    nodeA = Fs::getNodeFromLocalPath(B(""), B("/tmp/hardlink-a"), false);
    nodeB = Fs::getNodeFromLocalPath(B(""), B("/tmp/hardlink-b"), false);
    if (!nodeA || !nodeB) {
        testFail("hard links did not reload from hidden metadata");
        cleanupRoot(root);
        return;
    }
    std::shared_ptr<FsNode> tmpDir = Fs::getNodeFromLocalPath(B(""), B("/tmp"), false);
    if (tmpDir) {
        std::vector<std::shared_ptr<FsNode> > children;
        tmpDir->getAllChildren(children);
        expectU32("hidden hard link metadata child count", (U32)children.size(), 2);
    }
    expectZero("stat reloaded source", process->stat64(B("/tmp/hardlink-a"), STAT_A));
    expectZero("stat reloaded link", process->stat64(B("/tmp/hardlink-b"), STAT_B));
    expectU32("reloaded source link count", stat64LinkCount(memory, STAT_A), 2);
    expectU32("reloaded link link count", stat64LinkCount(memory, STAT_B), 2);
    expectU32("reloaded shared inode", stat64Inode(memory, STAT_B), stat64Inode(memory, STAT_A));
    value.clear();
    expectZero("get reloaded link xattr", Fs::getXAttr(nodeB, B("user.WINEREPARSE"), value));
    expectBytes("reloaded shared xattr", value, reparseData, sizeof(reparseData));

    const char replacement[] = "target";
    openNode = nodeB->open(K_O_RDWR | K_O_TRUNC);
    if (!openNode) {
        testFail("link file could not be opened");
        cleanupRoot(root);
        return;
    }
    expectU32("write link", openNode->writeNative((U8*)replacement, sizeof(replacement) - 1), sizeof(replacement) - 1);
    delete openNode;

    openNode = nodeA->open(K_O_RDONLY);
    if (!openNode) {
        testFail("source file could not be reopened");
        cleanupRoot(root);
        return;
    }
    char actual[sizeof(replacement) - 1];
    memset(actual, 0, sizeof(actual));
    expectU32("read source through link data", openNode->readNative((U8*)actual, sizeof(actual)), sizeof(actual));
    if (memcmp(actual, replacement, sizeof(actual))) {
        testFail("source did not read data written through link");
    }
    delete openNode;

    expectZero("unlink reloaded link", process->unlinkFile(B("/tmp/hardlink-b")));
    if (Fs::getNodeFromLocalPath(B(""), B("/tmp/hardlink-b"), false)) {
        testFail("unlinked hard link was still visible");
    }
    expectZero("stat source after unlink", process->stat64(B("/tmp/hardlink-a"), STAT_A));
    expectU32("source link count after unlink", stat64LinkCount(memory, STAT_A), 1);
    value.clear();
    expectZero("get source xattr after unlink", Fs::getXAttr(nodeA, B("user.WINEREPARSE"), value));
    expectBytes("source xattr after unlink", value, reparseData, sizeof(reparseData));

    cleanupRoot(root);
}

void testSharedFileMappingGrowthKeepsPagesShared() {
    TestContext& context = testContext();
    constexpr U32 INITIAL_SIZE = 0x10000;
    constexpr U32 GROWN_SIZE = 0x200000;
    constexpr U32 GROWN_OFFSET = 0x140000;
    constexpr U32 CACHE_PRIME_ADDRESS = 0x02000000;
    constexpr U32 WRITER_ADDRESS = 0x02100000;
    constexpr U32 READER_ADDRESS = 0x02200000;
    constexpr U32 EXPECTED = 0x13579bdf;
    const BString path = B("/tmp/shared-map-growth");
    const BString root = B("tmp/test-shared-map-growth-root");

    cleanupRoot(root);
    initTestFileSystem(root);
    Fs::makeLocalDirs(B("/tmp"));
    Fs::makeLocalDirs(B("/proc"));
    setTestProcNode();
    KSystem::eraseFileCache(path);

    KThread* writerThread = nullptr;
    KThread* readerThread = nullptr;
    KProcessPtr writer = createProcessWithMemory(&writerThread);
    KProcessPtr reader = createProcessWithMemory(&readerThread);
    U32 actual = 0;

    U32 writerFd = writer->open(path, K_O_CREAT | K_O_TRUNC | K_O_RDWR, 0666);
    U32 readerFd = reader->open(path, K_O_RDWR, 0666);
    if ((S32)writerFd < 0 || (S32)readerFd < 0) {
        testFail("shared-map-growth open failed writer=%d reader=%d", (S32)writerFd, (S32)readerFd);
        goto done;
    }

    if (writer->ftruncate64(writerFd, INITIAL_SIZE)) {
        testFail("shared-map-growth initial truncate failed");
        goto done;
    }

    KThread::setCurrentThread(writerThread);
    if (writer->memory->mmap(writerThread, CACHE_PRIME_ADDRESS, K_PAGE_SIZE, K_PROT_READ | K_PROT_WRITE,
        K_MAP_SHARED | K_MAP_FIXED, writerFd, 0) != CACHE_PRIME_ADDRESS) {
        testFail("shared-map-growth initial map failed");
        goto done;
    }

    if (writer->ftruncate64(writerFd, GROWN_SIZE)) {
        testFail("shared-map-growth grow truncate failed");
        goto done;
    }

    if (writer->memory->mmap(writerThread, WRITER_ADDRESS, K_PAGE_SIZE, K_PROT_READ | K_PROT_WRITE,
        K_MAP_SHARED | K_MAP_FIXED, writerFd, GROWN_OFFSET) != WRITER_ADDRESS) {
        testFail("shared-map-growth writer map failed");
        goto done;
    }

    KThread::setCurrentThread(readerThread);
    if (reader->memory->mmap(readerThread, READER_ADDRESS, K_PAGE_SIZE, K_PROT_READ | K_PROT_WRITE,
        K_MAP_SHARED | K_MAP_FIXED, readerFd, GROWN_OFFSET) != READER_ADDRESS) {
        testFail("shared-map-growth reader map failed");
        goto done;
    }

    KThread::setCurrentThread(writerThread);
    writer->memory->writed(WRITER_ADDRESS, EXPECTED);

    KThread::setCurrentThread(readerThread);
    actual = reader->memory->readd(READER_ADDRESS);
    if (actual != EXPECTED) {
        testFail("shared-map-growth reader expected 0x%X, got 0x%X", EXPECTED, actual);
    }

done:
    KThread::setCurrentThread(context.thread);
    if (writer) {
        if ((S32)writerFd >= 0) writer->close(writerFd);
        KSystem::eraseProcess(writer->id);
        writer = nullptr;
    }
    if (reader) {
        if ((S32)readerFd >= 0) reader->close(readerFd);
        KSystem::eraseProcess(reader->id);
        reader = nullptr;
    }
    KSystem::eraseFileCache(path);
    cleanupRoot(root);
}

void testSharedFileMappingTruncateClearsResidentBytes() {
    TestContext& context = testContext();
    constexpr U32 WRITER_ADDRESS = 0x02b00000;
    constexpr U32 OBSERVER_ADDRESS = 0x02c00000;
    constexpr U32 FILE_SIZE = K_PAGE_SIZE * 2;
    constexpr U32 SHRUNK_SIZE = K_PAGE_SIZE / 2;
    constexpr U32 SECOND_PAGE_OFFSET = K_PAGE_SIZE + 0x180;
    constexpr U32 FIRST_PAGE_OFFSET = 0x240;
    constexpr U32 SECOND_PAGE_VALUE = 0x89abcdef;
    constexpr U32 FIRST_PAGE_VALUE = 0x76543210;
    const BString path = B("/tmp/shared-map-truncate");
    const BString root = B("tmp/test-shared-map-truncate-root");

    cleanupRoot(root);
    initTestFileSystem(root);
    Fs::makeLocalDirs(B("/tmp"));
    Fs::makeLocalDirs(B("/proc"));
    setTestProcNode();

    KThread* writerThread = nullptr;
    KThread* observerThread = nullptr;
    KProcessPtr writer = createProcessWithMemory(&writerThread);
    KProcessPtr observer = createProcessWithMemory(&observerThread);
    U32 writerFd = writer->open(path, K_O_CREAT | K_O_TRUNC | K_O_RDWR, 0666);
    U32 observerFd = observer->open(path, K_O_RDWR, 0666);
    U32 truncateFd = (U32)-1;
    if ((S32)writerFd < 0 || (S32)observerFd < 0) {
        testFail("shared map truncate open failed writer=%d observer=%d", (S32)writerFd, (S32)observerFd);
        goto done;
    }
    if (writer->ftruncate64(writerFd, FILE_SIZE)) {
        testFail("shared map truncate initial growth failed");
        goto done;
    }

    KThread::setCurrentThread(writerThread);
    if (writer->memory->mmap(writerThread, WRITER_ADDRESS, FILE_SIZE, K_PROT_READ | K_PROT_WRITE,
        K_MAP_SHARED | K_MAP_FIXED, writerFd, 0) != WRITER_ADDRESS) {
        testFail("shared map truncate writer mapping failed");
        goto done;
    }

    KThread::setCurrentThread(observerThread);
    if (observer->memory->mmap(observerThread, OBSERVER_ADDRESS, FILE_SIZE, K_PROT_READ | K_PROT_WRITE,
        K_MAP_SHARED | K_MAP_FIXED, observerFd, 0) != OBSERVER_ADDRESS) {
        testFail("shared map truncate observer mapping failed");
        goto done;
    }

    KThread::setCurrentThread(writerThread);
    writer->memory->writed(WRITER_ADDRESS + SECOND_PAGE_OFFSET, SECOND_PAGE_VALUE);
    KThread::setCurrentThread(observerThread);
    expectU32("shared map value before shrink",
        observer->memory->readd(OBSERVER_ADDRESS + SECOND_PAGE_OFFSET), SECOND_PAGE_VALUE);
    expectZero("shared map shrink", writer->ftruncate64(writerFd, SHRUNK_SIZE));
    expectZero("shared map regrow", writer->ftruncate64(writerFd, FILE_SIZE));
    expectU32("shared map stale second-page bytes after shrink and regrow",
        observer->memory->readd(OBSERVER_ADDRESS + SECOND_PAGE_OFFSET), 0);

    KThread::setCurrentThread(writerThread);
    writer->memory->writed(WRITER_ADDRESS + FIRST_PAGE_OFFSET, FIRST_PAGE_VALUE);
    KThread::setCurrentThread(observerThread);
    expectU32("shared map first-page value before O_TRUNC",
        observer->memory->readd(OBSERVER_ADDRESS + FIRST_PAGE_OFFSET), FIRST_PAGE_VALUE);
    KThread::setCurrentThread(writerThread);
    truncateFd = writer->open(path, K_O_RDWR | K_O_TRUNC, 0666);
    if ((S32)truncateFd < 0) {
        testFail("shared map O_TRUNC open failed fd=%d", (S32)truncateFd);
        goto done;
    }
    expectZero("shared map growth after O_TRUNC", writer->ftruncate64(truncateFd, FILE_SIZE));
    KThread::setCurrentThread(observerThread);
    expectU32("shared map stale first-page bytes after O_TRUNC and regrow",
        observer->memory->readd(OBSERVER_ADDRESS + FIRST_PAGE_OFFSET), 0);

done:
    KThread::setCurrentThread(context.thread);
    if (writer) {
        if ((S32)truncateFd >= 0) writer->close(truncateFd);
        if ((S32)writerFd >= 0) writer->close(writerFd);
        KSystem::eraseProcess(writer->id);
        writer = nullptr;
    }
    if (observer) {
        if ((S32)observerFd >= 0) observer->close(observerFd);
        KSystem::eraseProcess(observer->id);
        observer = nullptr;
    }
    cleanupRoot(root);
}

void testSharedMappedWriteIsVisibleToPread() {
    TestContext& context = testContext();
    constexpr U32 MAP_ADDRESS = 0x02300000;
    constexpr U32 BUFFER_ADDRESS = 0x02400000;
    constexpr U32 FILE_SIZE = K_PAGE_SIZE;
    constexpr U32 FILE_OFFSET = 0x180;
    constexpr U32 EXPECTED = 0x78563412;
    const BString path = B("/tmp/mapped-pread");
    const BString root = B("tmp/test-mapped-pread-root");

    cleanupRoot(root);
    initTestFileSystem(root);
    Fs::makeLocalDirs(B("/tmp"));
    Fs::makeLocalDirs(B("/proc"));
    setTestProcNode();

    KThread* thread = nullptr;
    KProcessPtr process = createProcessWithMemory(&thread);
    U32 fd = process->open(path, K_O_CREAT | K_O_TRUNC | K_O_RDWR, 0666);
    if ((S32)fd < 0) {
        testFail("shared map pread open failed fd=%d", (S32)fd);
        goto done;
    }
    if (process->ftruncate64(fd, FILE_SIZE)) {
        testFail("shared map pread truncate failed");
        goto done;
    }

    KThread::setCurrentThread(thread);
    if (process->memory->mmap(thread, BUFFER_ADDRESS, K_PAGE_SIZE, K_PROT_READ | K_PROT_WRITE,
        K_MAP_PRIVATE | K_MAP_ANONYMOUS | K_MAP_FIXED, -1, 0) != BUFFER_ADDRESS) {
        testFail("shared map pread buffer map failed");
        goto done;
    }
    if (process->memory->mmap(thread, MAP_ADDRESS, K_PAGE_SIZE, K_PROT_READ | K_PROT_WRITE,
        K_MAP_SHARED | K_MAP_FIXED, fd, 0) != MAP_ADDRESS) {
        testFail("shared map pread file map failed");
        goto done;
    }

    process->memory->writed(MAP_ADDRESS + FILE_OFFSET, EXPECTED);
    expectU32("shared map pread before unmap count",
        process->pread64(thread, fd, BUFFER_ADDRESS, sizeof(U32), FILE_OFFSET), sizeof(U32));
    expectU32("shared map pread before unmap value", process->memory->readd(BUFFER_ADDRESS), EXPECTED);
    process->memory->unmap(MAP_ADDRESS, K_PAGE_SIZE);
    process->memory->writed(BUFFER_ADDRESS, 0);
    expectU32("shared map pread after unmap count",
        process->pread64(thread, fd, BUFFER_ADDRESS, sizeof(U32), FILE_OFFSET), sizeof(U32));
    expectU32("shared map pread after unmap value", process->memory->readd(BUFFER_ADDRESS), EXPECTED);

done:
    KThread::setCurrentThread(context.thread);
    if (process) {
        if ((S32)fd >= 0) process->close(fd);
        KSystem::eraseProcess(process->id);
        process = nullptr;
    }
    cleanupRoot(root);
}

void testPwriteUpdatesResidentSharedMapping() {
    TestContext& context = testContext();
    constexpr U32 MAP_ADDRESS = 0x02500000;
    constexpr U32 BUFFER_ADDRESS = 0x02600000;
    constexpr U32 FILE_OFFSET = 0x240;
    constexpr U32 EXPECTED = 0xa1b2c3d4;
    const BString path = B("/tmp/pwrite-updates-shared-map");
    const BString root = B("tmp/test-pwrite-updates-shared-map-root");

    cleanupRoot(root);
    initTestFileSystem(root);
    Fs::makeLocalDirs(B("/tmp"));
    Fs::makeLocalDirs(B("/proc"));
    setTestProcNode();

    KThread* thread = nullptr;
    KProcessPtr process = createProcessWithMemory(&thread);
    U32 fd = process->open(path, K_O_CREAT | K_O_TRUNC | K_O_RDWR, 0666);
    if ((S32)fd < 0) {
        testFail("pwrite shared map open failed fd=%d", (S32)fd);
        goto done;
    }
    if (process->ftruncate64(fd, K_PAGE_SIZE)) {
        testFail("pwrite shared map truncate failed");
        goto done;
    }

    KThread::setCurrentThread(thread);
    if (process->memory->mmap(thread, BUFFER_ADDRESS, K_PAGE_SIZE, K_PROT_READ | K_PROT_WRITE,
        K_MAP_PRIVATE | K_MAP_ANONYMOUS | K_MAP_FIXED, -1, 0) != BUFFER_ADDRESS) {
        testFail("pwrite shared map buffer map failed");
        goto done;
    }
    if (process->memory->mmap(thread, MAP_ADDRESS, K_PAGE_SIZE, K_PROT_READ | K_PROT_WRITE,
        K_MAP_SHARED | K_MAP_FIXED, fd, 0) != MAP_ADDRESS) {
        testFail("pwrite shared map file map failed");
        goto done;
    }

    process->memory->readd(MAP_ADDRESS + FILE_OFFSET);
    process->memory->writed(BUFFER_ADDRESS, EXPECTED);
    expectU32("pwrite shared map count",
        process->pwrite64(thread, fd, BUFFER_ADDRESS, sizeof(U32), FILE_OFFSET), sizeof(U32));
    expectU32("pwrite updated resident shared mapping", process->memory->readd(MAP_ADDRESS + FILE_OFFSET), EXPECTED);

done:
    KThread::setCurrentThread(context.thread);
    if (process) {
        if ((S32)fd >= 0) process->close(fd);
        KSystem::eraseProcess(process->id);
        process = nullptr;
    }
    cleanupRoot(root);
}

void testExtendingPwriteAdvancesMappedFileCacheLength() {
    TestContext& context = testContext();
    constexpr U32 MAP_ADDRESS = 0x02d00000;
    constexpr U32 BUFFER_ADDRESS = 0x02e00000;
    constexpr U32 WRITER_ADDRESS = 0x02f00000;
    constexpr U32 INITIAL_SIZE = 0x100;
    constexpr U32 HOLE_OFFSET = 0x180;
    constexpr U32 WRITE_OFFSET = 0x200;
    constexpr U32 EXTENDED_SIZE = WRITE_OFFSET + sizeof(U32);
    constexpr U32 STALE_HOLE_VALUE = 0x55667788;
    constexpr U32 EXPECTED = 0xc1d2e3f4;
    const BString path = B("/tmp/extending-pwrite-cache-length");
    const BString root = B("tmp/test-extending-pwrite-cache-length-root");

    cleanupRoot(root);
    initTestFileSystem(root);
    Fs::makeLocalDirs(B("/tmp"));
    Fs::makeLocalDirs(B("/proc"));
    setTestProcNode();

    KThread* thread = nullptr;
    KThread* mappedWriterThread = nullptr;
    KProcessPtr process = createProcessWithMemory(&thread);
    KProcessPtr mappedWriter = createProcessWithMemory(&mappedWriterThread);
    U32 fd = process->open(path, K_O_CREAT | K_O_TRUNC | K_O_RDWR, 0666);
    U32 mappedWriterFd = mappedWriter->open(path, K_O_RDWR, 0666);
    if ((S32)fd < 0 || (S32)mappedWriterFd < 0) {
        testFail("extending pwrite cache length open failed fd=%d writer=%d", (S32)fd, (S32)mappedWriterFd);
        goto done;
    }
    if (process->ftruncate64(fd, INITIAL_SIZE)) {
        testFail("extending pwrite cache length initial truncate failed");
        goto done;
    }

    KThread::setCurrentThread(thread);
    if (process->memory->mmap(thread, BUFFER_ADDRESS, K_PAGE_SIZE, K_PROT_READ | K_PROT_WRITE,
        K_MAP_PRIVATE | K_MAP_ANONYMOUS | K_MAP_FIXED, -1, 0) != BUFFER_ADDRESS) {
        testFail("extending pwrite cache length buffer map failed");
        goto done;
    }
    if (process->memory->mmap(thread, MAP_ADDRESS, K_PAGE_SIZE, K_PROT_READ | K_PROT_WRITE,
        K_MAP_SHARED | K_MAP_FIXED, fd, 0) != MAP_ADDRESS) {
        testFail("extending pwrite cache length file map failed");
        goto done;
    }

    KThread::setCurrentThread(mappedWriterThread);
    if (mappedWriter->memory->mmap(mappedWriterThread, WRITER_ADDRESS, K_PAGE_SIZE, K_PROT_READ | K_PROT_WRITE,
        K_MAP_SHARED | K_MAP_FIXED, mappedWriterFd, 0) != WRITER_ADDRESS) {
        testFail("extending pwrite cache length writer map failed");
        goto done;
    }
    mappedWriter->memory->writed(WRITER_ADDRESS + HOLE_OFFSET, STALE_HOLE_VALUE);

    KThread::setCurrentThread(thread);
    expectU32("stale resident sparse-hole value before extending pwrite",
        process->memory->readd(MAP_ADDRESS + HOLE_OFFSET), STALE_HOLE_VALUE);
    process->memory->readd(MAP_ADDRESS + WRITE_OFFSET);
    process->memory->writed(BUFFER_ADDRESS, EXPECTED);
    expectU32("extending pwrite cache length count",
        process->pwrite64(thread, fd, BUFFER_ADDRESS, sizeof(U32), WRITE_OFFSET), sizeof(U32));
    expectU32("extending pwrite visible before same-length truncate",
        process->memory->readd(MAP_ADDRESS + WRITE_OFFSET), EXPECTED);
    expectU32("extending pwrite clears resident sparse hole",
        process->memory->readd(MAP_ADDRESS + HOLE_OFFSET), 0);
    expectZero("same-length truncate after extending pwrite", process->ftruncate64(fd, EXTENDED_SIZE));
    expectU32("extending pwrite survives same-length truncate",
        process->memory->readd(MAP_ADDRESS + WRITE_OFFSET), EXPECTED);

done:
    KThread::setCurrentThread(context.thread);
    if (process) {
        if ((S32)fd >= 0) process->close(fd);
        KSystem::eraseProcess(process->id);
        process = nullptr;
    }
    if (mappedWriter) {
        if ((S32)mappedWriterFd >= 0) mappedWriter->close(mappedWriterFd);
        KSystem::eraseProcess(mappedWriter->id);
        mappedWriter = nullptr;
    }
    cleanupRoot(root);
}

void testMappedFileMutationOperationOrdering() {
#if !defined(BOXEDWINE_MULTI_THREADED)
    return;
#else
    TestContext& context = testContext();
    constexpr U32 MAP_ADDRESS = 0x03500000;
    constexpr U32 FILE_OFFSET = 0x280;
    constexpr U32 FIRST_VALUE = 0x11223344;
    constexpr U32 SECOND_VALUE = 0xaabbccdd;
    const BString path = B("/tmp/mapped-mutation-operation-ordering");
    const BString root = B("tmp/test-mapped-mutation-operation-ordering-root");

    cleanupRoot(root);
    initTestFileSystem(root);
    Fs::makeLocalDirs(B("/tmp"));
    Fs::makeLocalDirs(B("/proc"));
    setTestProcNode();

    KThread* thread = nullptr;
    KProcessPtr process = createProcessWithMemory(&thread);
    U32 firstFd = process->open(path, K_O_CREAT | K_O_TRUNC | K_O_RDWR, 0666);
    U32 secondFd = process->open(path, K_O_RDWR, 0666);
    std::shared_ptr<KFile> firstFile;
    std::shared_ptr<KFile> secondFile;
    std::shared_ptr<FsFileIdentity> identity;
    std::thread firstWorker;
    std::thread secondWorker;
    std::atomic<U32> hookCalls(0);
    std::atomic<U32> firstResult((U32)-1);
    std::atomic<U32> secondResult((U32)-1);
    TestPausePoint firstCallbackPause;
    TestEvent secondStarted;
    TestEvent secondDone;

    if ((S32)firstFd < 0 || (S32)secondFd < 0) {
        testFail("mapped mutation ordering open failed first=%d second=%d", (S32)firstFd, (S32)secondFd);
        goto done;
    }
    expectZero("mapped mutation ordering initial truncate", process->ftruncate64(firstFd, K_PAGE_SIZE));

    KThread::setCurrentThread(thread);
    if (process->memory->mmap(thread, MAP_ADDRESS, K_PAGE_SIZE, K_PROT_READ | K_PROT_WRITE,
        K_MAP_SHARED | K_MAP_FIXED, firstFd, 0) != MAP_ADDRESS) {
        testFail("mapped mutation ordering map failed");
        goto done;
    }
    process->memory->readd(MAP_ADDRESS + FILE_OFFSET);

    firstFile = std::dynamic_pointer_cast<KFile>(process->getFileDescriptor(firstFd)->kobject);
    secondFile = std::dynamic_pointer_cast<KFile>(process->getFileDescriptor(secondFd)->kobject);
    identity = firstFile->openFile->node->getFileIdentity();
    identity->testAfterBackingMutationBeforeCacheNotification = [&]() {
        if (hookCalls.fetch_add(1, std::memory_order_acq_rel) == 0) {
            firstCallbackPause.pause();
        }
    };

    firstWorker = std::thread([&]() {
        U32 value = FIRST_VALUE;
        firstResult.store(firstFile->pwriteNative((U8*)&value, FILE_OFFSET, sizeof(value)), std::memory_order_release);
    });
    if (!firstCallbackPause.waitUntilReached(2000)) {
        testFail("first mutation did not reach delayed cache callback");
        firstCallbackPause.release();
        goto done;
    }

    secondWorker = std::thread([&]() {
        U32 value = SECOND_VALUE;
        secondStarted.signal();
        secondResult.store(secondFile->pwriteNative((U8*)&value, FILE_OFFSET, sizeof(value)), std::memory_order_release);
        secondDone.signal();
    });
    if (!secondStarted.wait(2000)) {
        testFail("later mutation worker did not start");
    }
    if (secondDone.wait(250)) {
        testFail("later backing mutation completed while an earlier same-identity callback was delayed");
    }
    firstCallbackPause.release();
    firstWorker.join();
    secondWorker.join();
    identity->testAfterBackingMutationBeforeCacheNotification = nullptr;

    expectU32("first ordered mutation count", firstResult.load(std::memory_order_acquire), sizeof(U32));
    expectU32("second ordered mutation count", secondResult.load(std::memory_order_acquire), sizeof(U32));
    expectU32("ordered mutation final mapped value", process->memory->readd(MAP_ADDRESS + FILE_OFFSET), SECOND_VALUE);
    {
        U32 hostValue = 0;
        expectU32("ordered mutation final host read count",
            firstFile->preadNativeUncached((U8*)&hostValue, FILE_OFFSET, sizeof(hostValue)), sizeof(hostValue));
        expectU32("ordered mutation final host value", hostValue, SECOND_VALUE);
    }

done:
    firstCallbackPause.release();
    if (firstWorker.joinable()) firstWorker.join();
    if (secondWorker.joinable()) secondWorker.join();
    if (identity) identity->testAfterBackingMutationBeforeCacheNotification = nullptr;
    KThread::setCurrentThread(context.thread);
    if (process) {
        if ((S32)secondFd >= 0) process->close(secondFd);
        if ((S32)firstFd >= 0) process->close(firstFd);
        KSystem::eraseProcess(process->id);
        process = nullptr;
    }
    cleanupRoot(root);
#endif
}

void testMmapLengthReconcileOrdersWithFileMutation() {
#if !defined(BOXEDWINE_MULTI_THREADED)
    return;
#else
    TestContext& context = testContext();
    constexpr U32 FIRST_MAP_ADDRESS = 0x03700000;
    constexpr U32 SECOND_MAP_ADDRESS = 0x03800000;
    constexpr U32 INITIAL_SIZE = 0x100;
    constexpr U32 WRITE_OFFSET = 0x200;
    constexpr U32 UPDATED_VALUE = 0x89abcdef;
    const BString path = B("/tmp/mmap-length-reconcile-ordering");
    const BString root = B("tmp/test-mmap-length-reconcile-ordering-root");

    cleanupRoot(root);
    initTestFileSystem(root);
    Fs::makeLocalDirs(B("/tmp"));
    Fs::makeLocalDirs(B("/proc"));
    setTestProcNode();

    KThread* thread = nullptr;
    KProcessPtr process = createProcessWithMemory(&thread);
    KThread* mapperThread = process->createThread();
    U32 mapFd = process->open(path, K_O_CREAT | K_O_TRUNC | K_O_RDWR, 0666);
    U32 writeFd = process->open(path, K_O_RDWR, 0666);
    std::shared_ptr<KFile> writeFile;
    std::shared_ptr<FsFileIdentity> identity;
    std::thread mapper;
    std::thread writer;
    std::atomic<U32> mapResult((U32)-1);
    std::atomic<U32> writeResult((U32)-1);
    TestPausePoint lengthReadPause;
    TestEvent writerStarted;
    TestEvent writerDone;

    if ((S32)mapFd < 0 || (S32)writeFd < 0) {
        testFail("mmap reconcile ordering open failed map=%d write=%d", (S32)mapFd, (S32)writeFd);
        goto done;
    }
    expectZero("mmap reconcile ordering initial truncate", process->ftruncate64(mapFd, INITIAL_SIZE));
    KThread::setCurrentThread(thread);
    if (process->memory->mmap(thread, FIRST_MAP_ADDRESS, K_PAGE_SIZE, K_PROT_READ | K_PROT_WRITE,
        K_MAP_SHARED | K_MAP_FIXED, mapFd, 0) != FIRST_MAP_ADDRESS) {
        testFail("mmap reconcile ordering initial map failed");
        goto done;
    }
    process->memory->readd(FIRST_MAP_ADDRESS + WRITE_OFFSET);

    writeFile = std::dynamic_pointer_cast<KFile>(process->getFileDescriptor(writeFd)->kobject);
    identity = writeFile->openFile->node->getFileIdentity();
    identity->testAfterMappedLengthReadBeforeCacheReconcile = [&]() { lengthReadPause.pause(); };

    mapper = std::thread([&]() {
        KThread::setCurrentThread(mapperThread);
        mapResult.store(process->memory->mmap(mapperThread, SECOND_MAP_ADDRESS, K_PAGE_SIZE,
            K_PROT_READ | K_PROT_WRITE, K_MAP_SHARED | K_MAP_FIXED, mapFd, 0), std::memory_order_release);
        KThread::setCurrentThread(nullptr);
    });
    if (!lengthReadPause.waitUntilReached(2000)) {
        testFail("mmap reconcile ordering did not pause after backing-length read");
        lengthReadPause.release();
        goto done;
    }
    writer = std::thread([&]() {
        U32 value = UPDATED_VALUE;
        writerStarted.signal();
        writeResult.store(writeFile->pwriteNative((U8*)&value, WRITE_OFFSET, sizeof(value)), std::memory_order_release);
        writerDone.signal();
    });
    if (!writerStarted.wait(2000)) {
        testFail("mmap reconcile ordering writer did not start");
    }
    if (writerDone.wait(250)) {
        testFail("descriptor mutation completed while mmap held a stale backing length");
    }
    lengthReadPause.release();
    mapper.join();
    writer.join();
    identity->testAfterMappedLengthReadBeforeCacheReconcile = nullptr;

    expectU32("mmap reconcile ordering second map result", mapResult.load(std::memory_order_acquire), SECOND_MAP_ADDRESS);
    expectU32("mmap reconcile ordering write count", writeResult.load(std::memory_order_acquire), sizeof(U32));
    expectU32("mmap stale length did not erase completed write",
        process->memory->readd(FIRST_MAP_ADDRESS + WRITE_OFFSET), UPDATED_VALUE);

done:
    lengthReadPause.release();
    if (mapper.joinable()) mapper.join();
    if (writer.joinable()) writer.join();
    if (identity) identity->testAfterMappedLengthReadBeforeCacheReconcile = nullptr;
    KThread::setCurrentThread(context.thread);
    if (process) {
        if ((S32)writeFd >= 0) process->close(writeFd);
        if ((S32)mapFd >= 0) process->close(mapFd);
        process->deleteThread(mapperThread);
        KSystem::eraseProcess(process->id);
        process = nullptr;
    }
    cleanupRoot(root);
#endif
}

void testWritebackPreparationOrdersWithFileMutation() {
#if !defined(BOXEDWINE_MULTI_THREADED)
    return;
#else
    TestContext& context = testContext();
    constexpr U32 MAP_ADDRESS = 0x03900000;
    constexpr U32 FILE_OFFSET = 0x340;
    constexpr U32 PREPARED_VALUE = 0x11223344;
    constexpr U32 DESCRIPTOR_VALUE = 0xaabbccdd;
    const BString path = B("/tmp/writeback-preparation-ordering");
    const BString root = B("tmp/test-writeback-preparation-ordering-root");

    cleanupRoot(root);
    initTestFileSystem(root);
    Fs::makeLocalDirs(B("/tmp"));
    Fs::makeLocalDirs(B("/proc"));
    setTestProcNode();

    KThread* thread = nullptr;
    KProcessPtr process = createProcessWithMemory(&thread);
    U32 mapFd = process->open(path, K_O_CREAT | K_O_TRUNC | K_O_RDWR, 0666);
    U32 writeFd = process->open(path, K_O_RDWR, 0666);
    std::shared_ptr<KFile> mapFile;
    std::shared_ptr<KFile> writeFile;
    std::shared_ptr<MappedFileCache> cache;
    std::thread writeback;
    std::thread writer;
    std::atomic<S32> writebackPreparationError(-1);
    std::atomic<S32> writebackIoError(-1);
    std::atomic<U64> writebackBytes((U64)-1);
    std::atomic<U32> writeResult((U32)-1);
    TestPausePoint preparationPause;
    TestEvent writerStarted;
    TestEvent writerDone;

    if ((S32)mapFd < 0 || (S32)writeFd < 0) {
        testFail("writeback ordering open failed map=%d write=%d", (S32)mapFd, (S32)writeFd);
        goto done;
    }
    expectZero("writeback ordering initial truncate", process->ftruncate64(mapFd, K_PAGE_SIZE));
    KThread::setCurrentThread(thread);
    if (process->memory->mmap(thread, MAP_ADDRESS, K_PAGE_SIZE, K_PROT_READ | K_PROT_WRITE,
        K_MAP_SHARED | K_MAP_FIXED, mapFd, 0) != MAP_ADDRESS) {
        testFail("writeback ordering map failed");
        goto done;
    }
    process->memory->readd(MAP_ADDRESS + FILE_OFFSET);
    mapFile = std::dynamic_pointer_cast<KFile>(process->getFileDescriptor(mapFd)->kobject);
    writeFile = std::dynamic_pointer_cast<KFile>(process->getFileDescriptor(writeFd)->kobject);
    cache = KSystem::getFileCache(mapFile->openFile->node->getFileIdentity());
    if (!cache) {
        testFail("writeback ordering cache was not created");
        goto done;
    }

    writeback = std::thread([&]() {
        U32 value = PREPARED_VALUE;
        KWritebackResult result = cache->testWritebackPreparedBytes(FILE_OFFSET, (U8*)&value, sizeof(value),
            [&]() { preparationPause.pause(); });
        writebackPreparationError.store(result.preparationError, std::memory_order_release);
        writebackIoError.store(result.ioError, std::memory_order_release);
        writebackBytes.store(result.bytesWritten, std::memory_order_release);
    });
    if (!preparationPause.waitUntilReached(2000)) {
        testFail("writeback ordering did not pause after preparation");
        preparationPause.release();
        goto done;
    }
    writer = std::thread([&]() {
        U32 value = DESCRIPTOR_VALUE;
        writerStarted.signal();
        writeResult.store(writeFile->pwriteNative((U8*)&value, FILE_OFFSET, sizeof(value)), std::memory_order_release);
        writerDone.signal();
    });
    if (!writerStarted.wait(2000)) {
        testFail("writeback ordering descriptor writer did not start");
    }
    if (writerDone.wait(250)) {
        testFail("descriptor mutation completed after stale writeback preparation and before its backing write");
    }
    preparationPause.release();
    writeback.join();
    writer.join();

    expectU32("writeback ordering preparation status",
        (U32)writebackPreparationError.load(std::memory_order_acquire), 0);
    expectU32("writeback ordering I/O status",
        (U32)writebackIoError.load(std::memory_order_acquire), 0);
    expectU64("writeback ordering prepared write count",
        writebackBytes.load(std::memory_order_acquire), sizeof(U32));
    expectU32("writeback ordering descriptor write count", writeResult.load(std::memory_order_acquire), sizeof(U32));
    {
        U32 hostValue = 0;
        expectU32("writeback ordering host read count",
            mapFile->preadNativeUncached((U8*)&hostValue, FILE_OFFSET, sizeof(hostValue)), sizeof(hostValue));
        expectU32("stale writeback did not overwrite later descriptor mutation", hostValue, DESCRIPTOR_VALUE);
    }
    expectU32("writeback ordering mapped cache value",
        process->memory->readd(MAP_ADDRESS + FILE_OFFSET), DESCRIPTOR_VALUE);

done:
    preparationPause.release();
    if (writeback.joinable()) writeback.join();
    if (writer.joinable()) writer.join();
    KThread::setCurrentThread(context.thread);
    if (process) {
        if ((S32)writeFd >= 0) process->close(writeFd);
        if ((S32)mapFd >= 0) process->close(mapFd);
        KSystem::eraseProcess(process->id);
        process = nullptr;
    }
    cleanupRoot(root);
#endif
}

void testMappedAppendDescriptorWritebackIsPositioned() {
    TestContext& context = testContext();
    constexpr U32 MAP_ADDRESS = 0x03a00000;
    constexpr U32 FILE_OFFSET = 0x240;
    constexpr U32 GUEST_OFFSET = 0x80;
    constexpr U32 WRITEBACK_VALUE = 0x12345678;
    constexpr U32 APPEND_VALUE = 0xa1b2c3d4;
    const BString path = B("/tmp/mapped-append-writeback");
    const BString root = B("tmp/test-mapped-append-writeback-root");

    cleanupRoot(root);
    initTestFileSystem(root);
    Fs::makeLocalDirs(B("/tmp"));
    Fs::makeLocalDirs(B("/proc"));
    setTestProcNode();

    KThread* thread = nullptr;
    KProcessPtr process = createProcessWithMemory(&thread);
    U32 fd = process->open(path, K_O_CREAT | K_O_TRUNC | K_O_RDWR | K_O_APPEND, 0666);
    std::shared_ptr<KFile> file;
    std::shared_ptr<MappedFileCache> cache;

    if ((S32)fd < 0) {
        testFail("mapped append writeback open failed fd=%d", (S32)fd);
        goto done;
    }
    expectZero("mapped append writeback initial truncate", process->ftruncate64(fd, K_PAGE_SIZE));
    // Invalidate any pathname reopen while keeping the already-open descriptor
    // writable, matching unlink/rename/permission-change lifetime semantics.
    expectZero("mapped append writeback invalidates pathname reopen",
        process->fchmod(fd, K__S_IREAD));
    KThread::setCurrentThread(thread);
    if (process->memory->mmap(thread, MAP_ADDRESS, K_PAGE_SIZE, K_PROT_READ | K_PROT_WRITE,
        K_MAP_SHARED | K_MAP_FIXED, fd, 0) != MAP_ADDRESS) {
        testFail("mapped append writeback map failed");
        goto done;
    }
    process->memory->writed(MAP_ADDRESS + FILE_OFFSET, WRITEBACK_VALUE);
    file = std::dynamic_pointer_cast<KFile>(process->getFileDescriptor(fd)->kobject);
    cache = KSystem::getFileCache(file->openFile->node->getFileIdentity());
    if (!cache) {
        testFail("mapped append writeback cache was not created");
        goto done;
    }
    expectU64("mapped append writeback establishes guest offset", (U64)file->seek(GUEST_OFFSET), GUEST_OFFSET);
    {
        KWritebackResult result = cache->testWritebackPreparedBytes(FILE_OFFSET,
            (const U8*)&WRITEBACK_VALUE, sizeof(WRITEBACK_VALUE), []() {});
        expectU32("mapped append writeback preparation status", (U32)result.preparationError, 0);
        expectU32("mapped append writeback I/O status", (U32)result.ioError, 0);
        expectU64("mapped append positioned write count", result.bytesWritten, sizeof(WRITEBACK_VALUE));
    }
    expectU64("mapped append writeback preserves file length", (U64)file->length(), K_PAGE_SIZE);
    expectU64("mapped append writeback preserves guest descriptor offset", (U64)file->getPos(), GUEST_OFFSET);
    {
        U32 hostValue = 0;
        expectU32("mapped append positioned read count",
            file->preadNativeUncached((U8*)&hostValue, FILE_OFFSET, sizeof(hostValue)), sizeof(hostValue));
        expectU32("mapped append writeback replaced mapped offset", hostValue, WRITEBACK_VALUE);
    }

    expectU32("mapped append guest append count",
        file->writeNative((U8*)&APPEND_VALUE, sizeof(APPEND_VALUE)), sizeof(APPEND_VALUE));
    expectU64("mapped append guest write advances to appended end",
        (U64)file->getPos(), K_PAGE_SIZE + sizeof(APPEND_VALUE));
    expectU64("mapped append guest write grows file",
        (U64)file->length(), K_PAGE_SIZE + sizeof(APPEND_VALUE));
    {
        U32 hostValue = 0;
        expectU32("mapped append guest value read count",
            file->preadNativeUncached((U8*)&hostValue, K_PAGE_SIZE, sizeof(hostValue)), sizeof(hostValue));
        expectU32("mapped append guest behavior remains append", hostValue, APPEND_VALUE);
    }

done:
    KThread::setCurrentThread(context.thread);
    if (process) {
        if ((S32)fd >= 0) process->close(fd);
        KSystem::eraseProcess(process->id);
        process = nullptr;
    }
    cleanupRoot(root);
}

void testMappedWritebackResultSemantics() {
    KWritebackResult overFourGiB;
    overFourGiB.recordIo(std::numeric_limits<U32>::max(), 0);
    overFourGiB.recordIo(2, 0);
    expectU64("writeback result keeps U64 progress",
        overFourGiB.bytesWritten, 0x100000001ULL);
    expectU32("writeback result U64 progress has no I/O error",
        (U32)overFourGiB.ioError, 0);

    KWritebackResult errorBeforeProgress;
    errorBeforeProgress.recordIo(0, -K_EIO);
    expectU64("writeback error before progress keeps zero bytes",
        errorBeforeProgress.bytesWritten, 0);
    expectU32("writeback error before progress remains separate",
        (U32)errorBeforeProgress.ioError, -K_EIO);

    KWritebackResult errorAfterProgress;
    errorAfterProgress.recordIo(7, 0);
    errorAfterProgress.recordIo(3, -K_ENOSPC);
    expectU64("writeback error after progress keeps partial bytes",
        errorAfterProgress.bytesWritten, 10);
    expectU32("writeback error after progress remains separate",
        (U32)errorAfterProgress.ioError, -K_ENOSPC);

    KWritebackResult preparationFailure;
    preparationFailure.preparationError = -K_ENOMEM;
    expectU32("writeback preparation error remains separate",
        (U32)preparationFailure.preparationError, -K_ENOMEM);
    expectU32("writeback preparation failure has no I/O error",
        (U32)preparationFailure.ioError, 0);
    expectU64("writeback preparation failure has no progress",
        preparationFailure.bytesWritten, 0);
}

void testMappedAppendWithoutPositionedWriteTargetFailsCleanly() {
    TestContext& context = testContext();
    constexpr U32 MAP_ADDRESS = 0x03b00000;
    const BString path = B("/tmp/mapped-append-no-positioned-target");
    const BString root = B("tmp/test-mapped-append-no-positioned-target-root");

    cleanupRoot(root);
    initTestFileSystem(root);
    Fs::makeLocalDirs(B("/tmp"));
    Fs::makeLocalDirs(B("/proc"));
    setTestProcNode();

    KThread* thread = nullptr;
    KProcessPtr process = createProcessWithMemory(&thread);
    U32 fd = process->open(path, K_O_CREAT | K_O_TRUNC | K_O_RDWR | K_O_APPEND, 0666);
    std::shared_ptr<KFile> file;

    if ((S32)fd < 0) {
        testFail("mapped append unavailable target open failed fd=%d", (S32)fd);
        goto done;
    }
    expectZero("mapped append unavailable target initial truncate", process->ftruncate64(fd, K_PAGE_SIZE));
    file = std::dynamic_pointer_cast<KFile>(process->getFileDescriptor(fd)->kobject);
    file->openFile->testForceWriteNativeAtUnavailable = true;
    KThread::setCurrentThread(thread);
    expectU32("mapped append unavailable target mmap error",
        process->memory->mmap(thread, MAP_ADDRESS, K_PAGE_SIZE, K_PROT_READ | K_PROT_WRITE,
            K_MAP_SHARED | K_MAP_FIXED, fd, 0), -K_EIO);
    expectU32("mapped append unavailable target installs no mapped page",
        process->memory->isPageMapped(MAP_ADDRESS >> K_PAGE_SHIFT), 0);
    expectU32("mapped append unavailable target creates no system cache",
        KSystem::getFileCache(file->openFile->node->getFileIdentity()) ? 1 : 0, 0);

done:
    KThread::setCurrentThread(context.thread);
    if (process) {
        if ((S32)fd >= 0) process->close(fd);
        KSystem::eraseProcess(process->id);
        process = nullptr;
    }
    cleanupRoot(root);
}

void testExactEndMsyncPersistsSharedMappedBytes() {
    TestContext& context = testContext();
    constexpr U32 MAP_ADDRESS = 0x03c00000;
    constexpr U32 FILE_OFFSET = K_PAGE_SIZE - sizeof(U32);
    constexpr U32 EXPECTED = 0x13579bdf;
    const BString path = B("/tmp/exact-end-msync");
    const BString root = B("tmp/test-exact-end-msync-root");

    cleanupRoot(root);
    initTestFileSystem(root);
    Fs::makeLocalDirs(B("/tmp"));
    Fs::makeLocalDirs(B("/proc"));
    setTestProcNode();

    KThread* thread = nullptr;
    KProcessPtr process = createProcessWithMemory(&thread);
    U32 fd = process->open(path, K_O_CREAT | K_O_TRUNC | K_O_RDWR, 0666);
    std::shared_ptr<KFile> file;

    if ((S32)fd < 0) {
        testFail("exact-end msync open failed fd=%d", (S32)fd);
        goto done;
    }
    expectZero("exact-end msync initial truncate", process->ftruncate64(fd, K_PAGE_SIZE));
    KThread::setCurrentThread(thread);
    if (process->memory->mmap(thread, MAP_ADDRESS, K_PAGE_SIZE, K_PROT_READ | K_PROT_WRITE,
        K_MAP_SHARED | K_MAP_FIXED, fd, 0) != MAP_ADDRESS) {
        testFail("exact-end msync map failed");
        goto done;
    }
    process->memory->writed(MAP_ADDRESS + FILE_OFFSET, EXPECTED);
    expectZero("exact-end msync result", process->msync(thread, MAP_ADDRESS, K_PAGE_SIZE, 0));

    file = std::dynamic_pointer_cast<KFile>(process->getFileDescriptor(fd)->kobject);
    {
        U32 hostValue = 0;
        expectU32("exact-end msync uncached backing read count",
            file->preadNativeUncached((U8*)&hostValue, FILE_OFFSET, sizeof(hostValue)), sizeof(hostValue));
        expectU32("exact-end msync persisted mapped bytes", hostValue, EXPECTED);
    }

done:
    KThread::setCurrentThread(context.thread);
    if (process) {
        if ((S32)fd >= 0) process->close(fd);
        KSystem::eraseProcess(process->id);
        process = nullptr;
    }
    cleanupRoot(root);
}

void testPrivateMsyncDoesNotPersistCowBytes() {
    TestContext& context = testContext();
    constexpr U32 MAP_ADDRESS = 0x03d00000;
    constexpr U32 FILE_OFFSET = 0x280;
    constexpr U32 ORIGINAL = 0x10203040;
    constexpr U32 PRIVATE_VALUE = 0xa1b2c3d4;
    const BString path = B("/tmp/private-msync");
    const BString root = B("tmp/test-private-msync-root");

    cleanupRoot(root);
    initTestFileSystem(root);
    Fs::makeLocalDirs(B("/tmp"));
    Fs::makeLocalDirs(B("/proc"));
    setTestProcNode();

    KThread* thread = nullptr;
    KProcessPtr process = createProcessWithMemory(&thread);
    U32 fd = process->open(path, K_O_CREAT | K_O_TRUNC | K_O_RDWR, 0666);
    std::shared_ptr<KFile> file;

    if ((S32)fd < 0) {
        testFail("private msync open failed fd=%d", (S32)fd);
        goto done;
    }
    expectZero("private msync initial truncate", process->ftruncate64(fd, K_PAGE_SIZE));
    file = std::dynamic_pointer_cast<KFile>(process->getFileDescriptor(fd)->kobject);
    expectU32("private msync initial backing write count",
        file->pwriteNative((U8*)&ORIGINAL, FILE_OFFSET, sizeof(ORIGINAL)), sizeof(ORIGINAL));

    KThread::setCurrentThread(thread);
    if (process->memory->mmap(thread, MAP_ADDRESS, K_PAGE_SIZE, K_PROT_READ | K_PROT_WRITE,
        K_MAP_PRIVATE | K_MAP_FIXED, fd, 0) != MAP_ADDRESS) {
        testFail("private msync map failed");
        goto done;
    }
    expectU32("private msync initial mapped value", process->memory->readd(MAP_ADDRESS + FILE_OFFSET), ORIGINAL);
    process->memory->writed(MAP_ADDRESS + FILE_OFFSET, PRIVATE_VALUE);
    expectZero("private msync result", process->msync(thread, MAP_ADDRESS, K_PAGE_SIZE, 0));
    expectU32("private msync preserves private value", process->memory->readd(MAP_ADDRESS + FILE_OFFSET), PRIVATE_VALUE);
    {
        U32 hostValue = 0;
        expectU32("private msync uncached backing read count",
            file->preadNativeUncached((U8*)&hostValue, FILE_OFFSET, sizeof(hostValue)), sizeof(hostValue));
        expectU32("private msync did not persist COW bytes", hostValue, ORIGINAL);
    }

done:
    KThread::setCurrentThread(context.thread);
    if (process) {
        if ((S32)fd >= 0) process->close(fd);
        KSystem::eraseProcess(process->id);
        process = nullptr;
    }
    cleanupRoot(root);
}

void testMappedCacheTeardownPersistsSharedBytes() {
    TestContext& context = testContext();
    constexpr U32 MAP_ADDRESS = 0x03e00000;
    constexpr U32 FILE_OFFSET = 0x380;
    constexpr U32 EXPECTED = 0x89abcdef;
    const BString path = B("/tmp/mapped-cache-teardown");
    const BString root = B("tmp/test-mapped-cache-teardown-root");

    cleanupRoot(root);
    initTestFileSystem(root);
    Fs::makeLocalDirs(B("/tmp"));
    Fs::makeLocalDirs(B("/proc"));
    setTestProcNode();

    KThread* thread = nullptr;
    KProcessPtr process = createProcessWithMemory(&thread);
    U32 fd = process->open(path, K_O_CREAT | K_O_TRUNC | K_O_RDWR, 0666);
    U32 processId = process->id;
    std::shared_ptr<FsNode> node;
    std::shared_ptr<FsFileIdentity> identity;

    if ((S32)fd < 0) {
        testFail("mapped cache teardown open failed fd=%d", (S32)fd);
        goto done;
    }
    expectZero("mapped cache teardown initial truncate", process->ftruncate64(fd, K_PAGE_SIZE));
    node = std::dynamic_pointer_cast<KFile>(process->getFileDescriptor(fd)->kobject)->openFile->node;
    identity = node->getFileIdentity();
    KThread::setCurrentThread(thread);
    if (process->memory->mmap(thread, MAP_ADDRESS, K_PAGE_SIZE, K_PROT_READ | K_PROT_WRITE,
        K_MAP_SHARED | K_MAP_FIXED, fd, 0) != MAP_ADDRESS) {
        testFail("mapped cache teardown map failed");
        goto done;
    }
    process->memory->writed(MAP_ADDRESS + FILE_OFFSET, EXPECTED);
    process->close(fd);
    fd = (U32)-1;

    KThread::setCurrentThread(context.thread);
    process->deleteThread(thread);
    thread = nullptr;
    KSystem::eraseProcess(processId);
    process = nullptr;
    if (KSystem::getFileCache(identity)) {
        testFail("mapped cache teardown left a live cache owner");
        goto done;
    }
    {
        FsOpenNode* reader = node->open(K_O_RDONLY);
        if (!reader) {
            testFail("mapped cache teardown backing reopen failed");
        } else {
            U32 hostValue = 0;
            expectU64("mapped cache teardown backing seek", (U64)reader->seek(FILE_OFFSET), FILE_OFFSET);
            expectU32("mapped cache teardown backing read count",
                reader->readNative((U8*)&hostValue, sizeof(hostValue)), sizeof(hostValue));
            expectU32("mapped cache teardown persisted mapped bytes", hostValue, EXPECTED);
            delete reader;
        }
    }

done:
    KThread::setCurrentThread(context.thread);
    if (process) {
        if ((S32)fd >= 0) process->close(fd);
        if (thread) {
            process->deleteThread(thread);
            thread = nullptr;
        }
        KSystem::eraseProcess(processId);
        process = nullptr;
    }
    cleanupRoot(root);
}

void testFinalMappedRetirementOrdersWithDescriptorMutation() {
#if !defined(BOXEDWINE_MULTI_THREADED)
    return;
#else
    TestContext& context = testContext();
    constexpr U32 MAP_ADDRESS = 0x03f00000;
    constexpr U32 FILE_OFFSET = 0x340;
    constexpr U32 MAPPED_VALUE = 0x11223344;
    constexpr U32 DESCRIPTOR_VALUE = 0xaabbccdd;
    const BString path = B("/tmp/final-mapped-retirement-ordering");
    const BString root = B("tmp/test-final-mapped-retirement-ordering-root");

    cleanupRoot(root);
    initTestFileSystem(root);
    Fs::makeLocalDirs(B("/tmp"));
    Fs::makeLocalDirs(B("/proc"));
    setTestProcNode();

    KThread* thread = nullptr;
    KProcessPtr process = createProcessWithMemory(&thread);
    U32 mapFd = process->open(path, K_O_CREAT | K_O_TRUNC | K_O_RDWR, 0666);
    U32 writeFd = process->open(path, K_O_RDWR, 0666);
    std::shared_ptr<KFile> mapFile;
    std::shared_ptr<KFile> writeFile;
    std::shared_ptr<MappedFileCache> cache;
    std::thread unmapper;
    std::thread writer;
    std::atomic<U32> unmapResult((U32)-1);
    std::atomic<U32> writeResult((U32)-1);
    TestPausePoint retirementPause;
    TestEvent writerStarted;
    TestEvent writerDone;

    if ((S32)mapFd < 0 || (S32)writeFd < 0) {
        testFail("final retirement ordering open failed map=%d write=%d", (S32)mapFd, (S32)writeFd);
        goto done;
    }
    expectZero("final retirement ordering truncate", process->ftruncate64(mapFd, K_PAGE_SIZE));
    KThread::setCurrentThread(thread);
    if (process->memory->mmap(thread, MAP_ADDRESS, K_PAGE_SIZE, K_PROT_READ | K_PROT_WRITE,
        K_MAP_SHARED | K_MAP_FIXED, mapFd, 0) != MAP_ADDRESS) {
        testFail("final retirement ordering map failed");
        goto done;
    }
    process->memory->writed(MAP_ADDRESS + FILE_OFFSET, MAPPED_VALUE);
    mapFile = std::dynamic_pointer_cast<KFile>(process->getFileDescriptor(mapFd)->kobject);
    writeFile = std::dynamic_pointer_cast<KFile>(process->getFileDescriptor(writeFd)->kobject);
    cache = KSystem::getFileCache(mapFile->openFile->node->getFileIdentity());
    if (!cache) {
        testFail("final retirement ordering cache was not created");
        goto done;
    }
    cache->setTestAfterFinalRetirementPreparationHook([&]() { retirementPause.pause(); });

    unmapper = std::thread([&]() {
        unmapResult.store(process->memory->unmap(MAP_ADDRESS, K_PAGE_SIZE), std::memory_order_release);
    });
    if (!retirementPause.waitUntilReached(2000)) {
        testFail("final retirement did not pause after writeback preparation");
        retirementPause.release();
        goto done;
    }
    writer = std::thread([&]() {
        writerStarted.signal();
        U32 value = DESCRIPTOR_VALUE;
        writeResult.store(writeFile->pwriteNative((U8*)&value, FILE_OFFSET, sizeof(value)), std::memory_order_release);
        writerDone.signal();
    });
    if (!writerStarted.wait(2000)) {
        testFail("final retirement descriptor writer did not start");
    }
    if (writerDone.wait(250)) {
        testFail("descriptor mutation overtook paused final retirement writeback");
    }
    retirementPause.release();
    unmapper.join();
    writer.join();

    expectZero("final retirement unmap result", unmapResult.load(std::memory_order_acquire));
    expectU32("final retirement descriptor write count", writeResult.load(std::memory_order_acquire), sizeof(U32));
    {
        U32 hostValue = 0;
        expectU32("final retirement host read count",
            mapFile->preadNativeUncached((U8*)&hostValue, FILE_OFFSET, sizeof(hostValue)), sizeof(hostValue));
        expectU32("descriptor mutation remains newer than final retirement", hostValue, DESCRIPTOR_VALUE);
    }

done:
    retirementPause.release();
    if (unmapper.joinable()) unmapper.join();
    if (writer.joinable()) writer.join();
    if (cache) cache->setTestAfterFinalRetirementPreparationHook(nullptr);
    KThread::setCurrentThread(context.thread);
    if (process) {
        if ((S32)writeFd >= 0) process->close(writeFd);
        if ((S32)mapFd >= 0) process->close(mapFd);
        KSystem::eraseProcess(process->id);
        process = nullptr;
    }
    cleanupRoot(root);
#endif
}

void testMappedWritebackPreparationAllocationFailure() {
    TestContext& context = testContext();
    constexpr U32 MAP_ADDRESS = 0x04000000;
    constexpr U32 RETRY_ADDRESS = 0x04010000;
    constexpr U32 FILE_OFFSET = 0x220;
    constexpr U32 EXPECTED = 0x55667788;
    constexpr U32 RETRY_VALUE = 0x99aabbcc;
    const BString path = B("/tmp/mapped-writeback-allocation-failure");
    const BString root = B("tmp/test-mapped-writeback-allocation-failure-root");

    cleanupRoot(root);
    initTestFileSystem(root);
    Fs::makeLocalDirs(B("/tmp"));
    Fs::makeLocalDirs(B("/proc"));
    setTestProcNode();

    KThread* thread = nullptr;
    KProcessPtr process = createProcessWithMemory(&thread);
    U32 fd = process->open(path, K_O_CREAT | K_O_TRUNC | K_O_RDWR, 0666);
    std::shared_ptr<KFile> file;
    std::shared_ptr<MappedFileCache> cache;

    if ((S32)fd < 0) {
        testFail("mapped writeback allocation open failed fd=%d", (S32)fd);
        goto done;
    }
    expectZero("mapped writeback allocation truncate", process->ftruncate64(fd, K_PAGE_SIZE));
    KThread::setCurrentThread(thread);
    if (process->memory->mmap(thread, MAP_ADDRESS, K_PAGE_SIZE, K_PROT_READ | K_PROT_WRITE,
        K_MAP_SHARED | K_MAP_FIXED, fd, 0) != MAP_ADDRESS) {
        testFail("mapped writeback allocation map failed");
        goto done;
    }
    process->memory->writed(MAP_ADDRESS + FILE_OFFSET, EXPECTED);
    file = std::dynamic_pointer_cast<KFile>(process->getFileDescriptor(fd)->kobject);
    cache = KSystem::getFileCache(file->openFile->node->getFileIdentity());
    if (!cache) {
        testFail("mapped writeback allocation cache was not created");
        goto done;
    }
    cache->setTestFailWritebackPreparationAllocation(true);
    expectU32("mapped writeback allocation failure result",
        process->msync(thread, MAP_ADDRESS, K_PAGE_SIZE, 0), -K_ENOMEM);
    {
        U32 hostValue = 0;
        expectU32("mapped writeback allocation failed host read count",
            file->preadNativeUncached((U8*)&hostValue, FILE_OFFSET, sizeof(hostValue)), sizeof(hostValue));
        expectU32("mapped writeback allocation failure performed no I/O", hostValue, 0);
    }
    cache->setTestFailWritebackPreparationAllocation(false);
    expectZero("mapped writeback allocation retry result",
        process->msync(thread, MAP_ADDRESS, K_PAGE_SIZE, 0));
    {
        U32 hostValue = 0;
        expectU32("mapped writeback allocation retry host read count",
            file->preadNativeUncached((U8*)&hostValue, FILE_OFFSET, sizeof(hostValue)), sizeof(hostValue));
        expectU32("mapped writeback allocation retry persisted bytes", hostValue, EXPECTED);
    }
    process->memory->writed(MAP_ADDRESS + FILE_OFFSET, RETRY_VALUE);
    cache->setTestFailWritebackPreparationAllocation(true);
    expectZero("mapped writeback allocation final retirement containment",
        process->memory->unmap(MAP_ADDRESS, K_PAGE_SIZE));
    expectU32("mapped writeback allocation failed retirement remains retryable",
        cache->getMappingLeaseCountForTest(), 1);
    {
        U32 hostValue = 0;
        expectU32("mapped writeback allocation failed retirement host read",
            file->preadNativeUncached((U8*)&hostValue, FILE_OFFSET, sizeof(hostValue)),
            sizeof(hostValue));
        expectU32("mapped writeback allocation failed retirement performed no I/O",
            hostValue, EXPECTED);
    }
    cache->setTestFailWritebackPreparationAllocation(false);
    if (process->memory->mmap(thread, RETRY_ADDRESS, K_PAGE_SIZE,
        K_PROT_READ | K_PROT_WRITE, K_MAP_PRIVATE | K_MAP_ANONYMOUS | K_MAP_FIXED,
        -1, 0) != RETRY_ADDRESS) {
        testFail("mapped writeback allocation retry trigger map failed");
        goto done;
    }
    expectZero("mapped writeback allocation retry trigger unmap",
        process->memory->unmap(RETRY_ADDRESS, K_PAGE_SIZE));
    expectU32("mapped writeback allocation retried retirement lease count",
        cache->getMappingLeaseCountForTest(), 0);
    {
        U32 hostValue = 0;
        expectU32("mapped writeback allocation retried retirement host read",
            file->preadNativeUncached((U8*)&hostValue, FILE_OFFSET, sizeof(hostValue)),
            sizeof(hostValue));
        expectU32("mapped writeback allocation retried retirement persisted bytes",
            hostValue, RETRY_VALUE);
    }

done:
    if (cache) cache->setTestFailWritebackPreparationAllocation(false);
    KThread::setCurrentThread(context.thread);
    if (process) {
        if ((S32)fd >= 0) process->close(fd);
        KSystem::eraseProcess(process->id);
        process = nullptr;
    }
    cleanupRoot(root);
}

void testMappedRetirementSurvivesProcessDestruction() {
    TestContext& context = testContext();
    constexpr U32 MAP_ADDRESS = 0x04080000;
    constexpr U32 RETRY_ADDRESS = 0x04090000;
    constexpr U32 FILE_OFFSET = 0x180;
    constexpr U32 EXPECTED = 0x46a8ce13;
    const BString path = B("/tmp/mapped-retirement-process-destruction");
    const BString root = B("tmp/test-mapped-retirement-process-destruction-root");

    cleanupRoot(root);
    initTestFileSystem(root);
    Fs::makeLocalDirs(B("/tmp"));
    Fs::makeLocalDirs(B("/proc"));
    setTestProcNode();

    KThread* thread = nullptr;
    KProcessPtr process = createProcessWithMemory(&thread);
    U32 fd = process->open(path, K_O_CREAT | K_O_TRUNC | K_O_RDWR, 0666);
    std::shared_ptr<KFile> file;
    std::shared_ptr<MappedFileCache> cache;

    if ((S32)fd < 0) {
        testFail("mapped retirement process destruction open failed fd=%d", (S32)fd);
        goto done;
    }
    expectZero("mapped retirement process destruction truncate",
        process->ftruncate64(fd, K_PAGE_SIZE));
    KThread::setCurrentThread(thread);
    if (process->memory->mmap(thread, MAP_ADDRESS, K_PAGE_SIZE,
        K_PROT_READ | K_PROT_WRITE, K_MAP_SHARED | K_MAP_FIXED, fd, 0) != MAP_ADDRESS) {
        testFail("mapped retirement process destruction map failed");
        goto done;
    }
    process->memory->writed(MAP_ADDRESS + FILE_OFFSET, EXPECTED);
    file = std::dynamic_pointer_cast<KFile>(process->getFileDescriptor(fd)->kobject);
    cache = KSystem::getFileCache(file->openFile->node->getFileIdentity());
    if (!cache) {
        testFail("mapped retirement process destruction cache was not created");
        goto done;
    }

    cache->setTestFailWritebackPreparationAllocation(true);
    expectZero("mapped retirement process destruction unmap",
        process->memory->unmap(MAP_ADDRESS, K_PAGE_SIZE));
    expectU32("mapped retirement process destruction queued lease",
        cache->getMappingLeaseCountForTest(), 1);

    KThread::setCurrentThread(context.thread);
    process->close(fd);
    fd = (U32)-1;
    KSystem::eraseProcess(process->id);
    process = nullptr;

    cache->setTestFailWritebackPreparationAllocation(false);
    {
        KThread* retryThread = nullptr;
        KProcessPtr retryProcess = createProcessWithMemory(&retryThread);
        KThread::setCurrentThread(retryThread);
        if (retryProcess->memory->mmap(retryThread, RETRY_ADDRESS, K_PAGE_SIZE,
            K_PROT_READ | K_PROT_WRITE, K_MAP_PRIVATE | K_MAP_ANONYMOUS | K_MAP_FIXED,
            -1, 0) != RETRY_ADDRESS) {
            testFail("mapped retirement process destruction retry trigger map failed");
        } else {
            expectZero("mapped retirement process destruction retry trigger unmap",
                retryProcess->memory->unmap(RETRY_ADDRESS, K_PAGE_SIZE));
        }
        KThread::setCurrentThread(context.thread);
        KSystem::eraseProcess(retryProcess->id);
        retryProcess = nullptr;
    }

    expectU32("mapped retirement process destruction retried lease",
        cache->getMappingLeaseCountForTest(), 0);
    {
        U32 hostValue = 0;
        expectU32("mapped retirement process destruction host read count",
            file->preadNativeUncached((U8*)&hostValue, FILE_OFFSET, sizeof(hostValue)),
            sizeof(hostValue));
        expectU32("mapped retirement process destruction persisted bytes",
            hostValue, EXPECTED);
    }

done:
    if (cache) {
        cache->setTestFailWritebackPreparationAllocation(false);
    }
    KThread::setCurrentThread(context.thread);
    if (process) {
        if ((S32)fd >= 0) {
            process->close(fd);
        }
        KSystem::eraseProcess(process->id);
        process = nullptr;
    }
    cleanupRoot(root);
}

void testFinalMappedRetirementIoErrorIsRetryable() {
    TestContext& context = testContext();
    constexpr U32 MAP_ADDRESS = 0x040a0000;
    constexpr U32 RETRY_ADDRESS = 0x040b0000;
    constexpr U32 FILE_OFFSET = 0x180;
    constexpr U32 EXPECTED = 0x7b5d3917;
    const BString root = B("tmp/test-final-retirement-io-error-root");

    cleanupRoot(root);
    initTestFileSystem(root);
    Fs::makeLocalDirs(B("/proc"));
    setTestProcNode();

    KThread* thread = nullptr;
    KProcessPtr process = createProcessWithMemory(&thread);
    auto node = std::make_shared<FsMemNode>(1, 1, B("final-retirement-io-error"));
    auto* open = new RetirementWriteTestOpenNode(node);
    auto file = std::make_shared<KFile>(open);
    KFileDescriptorPtr descriptor =
        process->allocFileDescriptor(file, K_O_RDWR, 0, -1, 0);
    std::shared_ptr<MappedFileCache> cache;
    MappedFilePtr mapping;

    KThread::setCurrentThread(thread);
    if (process->memory->mmap(thread, MAP_ADDRESS, K_PAGE_SIZE,
        K_PROT_READ | K_PROT_WRITE, K_MAP_SHARED | K_MAP_FIXED,
        descriptor->handle, 0) != MAP_ADDRESS) {
        testFail("final retirement I/O error map failed");
        goto done;
    }
    process->memory->writed(MAP_ADDRESS + FILE_OFFSET, EXPECTED);
    mapping = process->getMappedFileForRange(MAP_ADDRESS, K_PAGE_SIZE);
    cache = KSystem::getFileCache(node->getFileIdentity());
    if (!mapping || !cache) {
        testFail("final retirement I/O error did not create mapping state");
        goto done;
    }

    open->failureMode = RetirementWriteTestOpenNode::FailureMode::Error;
    expectZero("final retirement I/O error unmap",
        process->memory->unmap(MAP_ADDRESS, K_PAGE_SIZE));
    expectU32("final retirement I/O error keeps live cache lease",
        cache->getMappingLeaseCountForTest(), 1);
    expectU32("final retirement I/O error did not finalize accounting",
        cache->getRetirementAccountingFinalizeCountForTest(), 0);
    if (mapping->lease->isRetired()) {
        testFail("final retirement I/O error retired the failed lease");
    }
    expectU32("final retirement I/O error stores per-entry error",
        (U32)mapping->getPendingRetirementErrorForTest(), -K_EIO);
    expectU32("final retirement I/O error logs repeated error once",
        mapping->getPendingRetirementErrorReportCountForTest(), 1);
    if (mapping->file->openFile->node->path !=
            B("final-retirement-io-error")) {
        testFail("final retirement I/O error lost queued entry identity");
    }
    {
        U32 backing = 0;
        memcpy(&backing, open->bytes.data() + FILE_OFFSET, sizeof(backing));
        expectU32("final retirement I/O error leaves backing unchanged",
            backing, 0);
    }

    open->failureMode = RetirementWriteTestOpenNode::FailureMode::None;
    {
        U32 writesBeforeRetry = open->writeCalls;
        if (process->memory->mmap(thread, RETRY_ADDRESS, K_PAGE_SIZE,
            K_PROT_READ | K_PROT_WRITE,
            K_MAP_PRIVATE | K_MAP_ANONYMOUS | K_MAP_FIXED,
            -1, 0) != RETRY_ADDRESS) {
            testFail("final retirement I/O error retry trigger map failed");
            goto done;
        }
        expectZero("final retirement I/O error retry trigger unmap",
            process->memory->unmap(RETRY_ADDRESS, K_PAGE_SIZE));
        if (open->writeCalls != writesBeforeRetry + 1) {
            testFail("final retirement I/O error retry did not perform one full writeback");
        }
    }
    expectU32("final retirement I/O error retry commits cache lease",
        cache->getMappingLeaseCountForTest(), 0);
    expectU32("final retirement I/O error retry finalized accounting once",
        cache->getRetirementAccountingFinalizeCountForTest(), 1);
    if (!mapping->lease->isRetired()) {
        testFail("final retirement I/O error retry did not retire the lease");
    }
    expectU32("final retirement I/O error success clears pending error",
        (U32)mapping->getPendingRetirementErrorForTest(), 0);
    {
        U32 backing = 0;
        memcpy(&backing, open->bytes.data() + FILE_OFFSET, sizeof(backing));
        expectU32("final retirement I/O error retry persists complete bytes",
            backing, EXPECTED);
    }
    {
        U32 writesAfterCommit = open->writeCalls;
        if (process->memory->mmap(thread, RETRY_ADDRESS, K_PAGE_SIZE,
            K_PROT_READ | K_PROT_WRITE,
            K_MAP_PRIVATE | K_MAP_ANONYMOUS | K_MAP_FIXED,
            -1, 0) == RETRY_ADDRESS) {
            process->memory->unmap(RETRY_ADDRESS, K_PAGE_SIZE);
        }
        expectU32("final retirement I/O error commits exactly once",
            open->writeCalls, writesAfterCommit);
    }

done:
    open->failureMode = RetirementWriteTestOpenNode::FailureMode::None;
    KThread::setCurrentThread(context.thread);
    if (process) {
        if (descriptor) {
            process->close(descriptor->handle);
        }
        KSystem::eraseProcess(process->id);
        process = nullptr;
    }
    cleanupRoot(root);
}

void testFinalMappedRetirementShortWriteIsRetryable() {
    TestContext& context = testContext();
    constexpr U32 MAP_ADDRESS = 0x040c0000;
    constexpr U32 RETRY_ADDRESS = 0x040d0000;
    constexpr U32 FILE_OFFSET = 0;
    constexpr U64 EXPECTED = 0x1032547698badcfeULL;
    constexpr U32 SHORT_BYTES = 3;
    const BString root = B("tmp/test-final-retirement-short-write-root");

    cleanupRoot(root);
    initTestFileSystem(root);
    Fs::makeLocalDirs(B("/proc"));
    setTestProcNode();

    KThread* thread = nullptr;
    KProcessPtr process = createProcessWithMemory(&thread);
    auto node = std::make_shared<FsMemNode>(1, 1, B("final-retirement-short-write"));
    auto* open = new RetirementWriteTestOpenNode(node);
    auto file = std::make_shared<KFile>(open);
    KFileDescriptorPtr descriptor =
        process->allocFileDescriptor(file, K_O_RDWR, 0, -1, 0);
    std::shared_ptr<MappedFileCache> cache;
    MappedFilePtr mapping;

    KThread::setCurrentThread(thread);
    if (process->memory->mmap(thread, MAP_ADDRESS, K_PAGE_SIZE,
        K_PROT_READ | K_PROT_WRITE, K_MAP_SHARED | K_MAP_FIXED,
        descriptor->handle, 0) != MAP_ADDRESS) {
        testFail("final retirement short-write map failed");
        goto done;
    }
    process->memory->writeq(MAP_ADDRESS + FILE_OFFSET, EXPECTED);
    mapping = process->getMappedFileForRange(MAP_ADDRESS, K_PAGE_SIZE);
    cache = KSystem::getFileCache(node->getFileIdentity());
    if (!mapping || !cache) {
        testFail("final retirement short-write did not create mapping state");
        goto done;
    }

    open->failureMode = RetirementWriteTestOpenNode::FailureMode::ShortWrite;
    open->shortWriteBytes = SHORT_BYTES;
    expectZero("final retirement short-write unmap",
        process->memory->unmap(MAP_ADDRESS, K_PAGE_SIZE));
    expectU32("final retirement short-write keeps live cache lease",
        cache->getMappingLeaseCountForTest(), 1);
    expectU32("final retirement short-write did not finalize accounting",
        cache->getRetirementAccountingFinalizeCountForTest(), 0);
    if (mapping->lease->isRetired()) {
        testFail("final retirement short-write retired the failed lease");
    }
    expectU32("final retirement short-write stores per-entry error",
        (U32)mapping->getPendingRetirementErrorForTest(), -K_EIO);
    expectU32("final retirement short-write logs repeated error once",
        mapping->getPendingRetirementErrorReportCountForTest(), 1);
    if (mapping->file->openFile->node->path !=
            B("final-retirement-short-write")) {
        testFail("final retirement short-write lost queued entry identity");
    }
    if (memcmp(open->bytes.data() + FILE_OFFSET, &EXPECTED, SHORT_BYTES)) {
        testFail("final retirement short-write did not preserve partial backing prefix");
    }
    for (U32 i = SHORT_BYTES; i < sizeof(EXPECTED); ++i) {
        if (open->bytes[FILE_OFFSET + i]) {
            testFail("final retirement short-write changed backing past partial prefix");
            break;
        }
    }

    open->failureMode = RetirementWriteTestOpenNode::FailureMode::None;
    {
        U32 writesBeforeRetry = open->writeCalls;
        if (process->memory->mmap(thread, RETRY_ADDRESS, K_PAGE_SIZE,
            K_PROT_READ | K_PROT_WRITE,
            K_MAP_PRIVATE | K_MAP_ANONYMOUS | K_MAP_FIXED,
            -1, 0) != RETRY_ADDRESS) {
            testFail("final retirement short-write retry trigger map failed");
            goto done;
        }
        expectZero("final retirement short-write retry trigger unmap",
            process->memory->unmap(RETRY_ADDRESS, K_PAGE_SIZE));
        if (open->writeCalls != writesBeforeRetry + 1) {
            testFail("final retirement short-write retry did not perform one full writeback");
        }
    }
    expectU32("final retirement short-write retry commits cache lease",
        cache->getMappingLeaseCountForTest(), 0);
    expectU32("final retirement short-write retry finalized accounting once",
        cache->getRetirementAccountingFinalizeCountForTest(), 1);
    if (!mapping->lease->isRetired()) {
        testFail("final retirement short-write retry did not retire the lease");
    }
    expectU32("final retirement short-write success clears pending error",
        (U32)mapping->getPendingRetirementErrorForTest(), 0);
    if (memcmp(open->bytes.data() + FILE_OFFSET, &EXPECTED, sizeof(EXPECTED))) {
        testFail("final retirement short-write retry did not persist complete bytes");
    }

done:
    open->failureMode = RetirementWriteTestOpenNode::FailureMode::None;
    KThread::setCurrentThread(context.thread);
    if (process) {
        if (descriptor) {
            process->close(descriptor->handle);
        }
        KSystem::eraseProcess(process->id);
        process = nullptr;
    }
    cleanupRoot(root);
}

void testFinalMappedRetirementSerializesNewRetain() {
#if !defined(BOXEDWINE_MULTI_THREADED)
    return;
#else
    TestContext& context = testContext();
    constexpr U32 FIRST_ADDRESS = 0x040e0000;
    constexpr U32 SECOND_ADDRESS = 0x040f0000;
    const BString root = B("tmp/test-final-retirement-retain-order-root");

    cleanupRoot(root);
    initTestFileSystem(root);
    Fs::makeLocalDirs(B("/proc"));
    setTestProcNode();

    KThread* thread = nullptr;
    KProcessPtr process = createProcessWithMemory(&thread);
    KThread* mapperThread = process->createThread();
    auto node = std::make_shared<FsMemNode>(1, 1, B("final-retirement-retain-order"));
    auto* open = new RetirementWriteTestOpenNode(node);
    auto file = std::make_shared<KFile>(open);
    KFileDescriptorPtr descriptor =
        process->allocFileDescriptor(file, K_O_RDWR, 0, -1, 0);
    std::shared_ptr<MappedFileCache> cache;
    MappedFilePtr firstMapping;
    std::thread unmapper;
    std::thread mapper;
    TestPausePoint beforeFinalize;
    TestEvent mapperStarted;
    TestEvent mapperDone;
    std::atomic<U32> unmapResult((U32)-K_EINTR);
    std::atomic<U32> mapResult(0);

    KThread::setCurrentThread(thread);
    if (process->memory->mmap(thread, FIRST_ADDRESS, K_PAGE_SIZE,
        K_PROT_READ | K_PROT_WRITE, K_MAP_SHARED | K_MAP_FIXED,
        descriptor->handle, 0) != FIRST_ADDRESS) {
        testFail("final retirement retain ordering first map failed");
        goto done;
    }
    process->memory->writed(FIRST_ADDRESS, 0x12345678);
    firstMapping = process->getMappedFileForRange(FIRST_ADDRESS, K_PAGE_SIZE);
    cache = KSystem::getFileCache(node->getFileIdentity());
    if (!firstMapping || !cache) {
        testFail("final retirement retain ordering did not create mapping state");
        goto done;
    }

    open->beforeWriteReturn = [&]() { beforeFinalize.pause(); };
    unmapper = std::thread([&]() {
        unmapResult.store(process->memory->unmap(FIRST_ADDRESS, K_PAGE_SIZE),
            std::memory_order_release);
    });
    if (!beforeFinalize.waitUntilReached(2000)) {
        testFail("final retirement retain ordering did not pause before finalize");
        beforeFinalize.release();
        goto done;
    }
    expectU32("final retirement retain ordering did not finalize while paused",
        cache->getRetirementAccountingFinalizeCountForTest(), 0);
    mapper = std::thread([&]() {
        mapperStarted.signal();
        mapResult.store(process->memory->mmap(mapperThread, SECOND_ADDRESS,
            K_PAGE_SIZE, K_PROT_READ | K_PROT_WRITE,
            K_MAP_SHARED | K_MAP_FIXED, descriptor->handle, 0),
            std::memory_order_release);
        mapperDone.signal();
    });
    if (!mapperStarted.wait(2000)) {
        testFail("final retirement retain ordering mapper did not start");
        beforeFinalize.release();
        goto done;
    }
    if (mapperDone.wait(100)) {
        testFail("new retain overtook final retirement I/O transaction");
    }
    beforeFinalize.release();
    unmapper.join();
    mapper.join();
    open->beforeWriteReturn = nullptr;

    expectZero("final retirement retain ordering first unmap",
        unmapResult.load(std::memory_order_acquire));
    expectU32("final retirement retain ordering second map",
        mapResult.load(std::memory_order_acquire), SECOND_ADDRESS);
    if (!firstMapping->lease->isRetired()) {
        testFail("final retirement retain ordering old lease did not retire");
    }
    expectU32("final retirement retain ordering new lease is live",
        cache->getMappingLeaseCountForTest(), 1);
    expectU32("final retirement retain ordering first lease finalized once",
        cache->getRetirementAccountingFinalizeCountForTest(), 1);
    expectZero("final retirement retain ordering second unmap",
        process->memory->unmap(SECOND_ADDRESS, K_PAGE_SIZE));
    expectU32("final retirement retain ordering final lease count",
        cache->getMappingLeaseCountForTest(), 0);
    expectU32("final retirement retain ordering each final lease finalized",
        cache->getRetirementAccountingFinalizeCountForTest(), 2);

done:
    beforeFinalize.release();
    if (unmapper.joinable()) {
        unmapper.join();
    }
    if (mapper.joinable()) {
        mapper.join();
    }
    open->beforeWriteReturn = nullptr;
    KThread::setCurrentThread(context.thread);
    if (process) {
        if (descriptor) {
            process->close(descriptor->handle);
        }
        process->deleteThread(mapperThread);
        KSystem::eraseProcess(process->id);
        process = nullptr;
    }
    cleanupRoot(root);
#endif
}

void testMmapRejectsFileOffsetOverflowBeforeReplacement() {
    TestContext& context = testContext();
    constexpr U32 MAP_ADDRESS = 0x04100000;
    constexpr U64 FILE_OFFSET = std::numeric_limits<U64>::max() - K_PAGE_MASK;
    constexpr U32 SENTINEL = 0x3a5c7e91;
    const BString path = B("/tmp/msync-file-offset-overflow");
    const BString root = B("tmp/test-msync-file-offset-overflow-root");

    cleanupRoot(root);
    initTestFileSystem(root);
    Fs::makeLocalDirs(B("/tmp"));
    Fs::makeLocalDirs(B("/proc"));
    setTestProcNode();

    KThread* thread = nullptr;
    KProcessPtr process = createProcessWithMemory(&thread);
    U32 fd = process->open(path, K_O_CREAT | K_O_TRUNC | K_O_RDWR, 0666);

    if ((S32)fd < 0) {
        testFail("mmap offset overflow open failed fd=%d", (S32)fd);
        goto done;
    }
    KThread::setCurrentThread(thread);
    if (process->memory->mmap(thread, MAP_ADDRESS, K_PAGE_SIZE, K_PROT_READ | K_PROT_WRITE,
        K_MAP_PRIVATE | K_MAP_ANONYMOUS | K_MAP_FIXED, -1, 0) != MAP_ADDRESS) {
        testFail("mmap offset overflow sentinel map failed");
        goto done;
    }
    process->memory->writed(MAP_ADDRESS, SENTINEL);
    expectU32("mmap file offset overflow result",
        process->memory->mmap(thread, MAP_ADDRESS, K_PAGE_SIZE * 2, K_PROT_READ | K_PROT_WRITE,
            K_MAP_SHARED | K_MAP_FIXED, fd, FILE_OFFSET), -K_EINVAL);
    expectU32("mmap file offset overflow preserved existing page",
        process->memory->readd(MAP_ADDRESS), SENTINEL);
    if (process->getMappedFileForRange(MAP_ADDRESS, K_PAGE_SIZE)) {
        testFail("mmap file offset overflow published a mapped-file record");
    }

done:
    KThread::setCurrentThread(context.thread);
    if (process) {
        if ((S32)fd >= 0) process->close(fd);
        KSystem::eraseProcess(process->id);
        process = nullptr;
    }
    cleanupRoot(root);
}

void testMsyncRejectsUnmappedFileRange() {
    TestContext& context = testContext();
    constexpr U32 MAP_ADDRESS = 0x04300000;
    const BString path = B("/tmp/msync-unmapped-range");
    const BString root = B("tmp/test-msync-unmapped-range-root");

    cleanupRoot(root);
    initTestFileSystem(root);
    Fs::makeLocalDirs(B("/tmp"));
    Fs::makeLocalDirs(B("/proc"));
    setTestProcNode();

    KThread* thread = nullptr;
    KProcessPtr process = createProcessWithMemory(&thread);
    U32 fd = process->open(path, K_O_CREAT | K_O_TRUNC | K_O_RDWR, 0666);

    if ((S32)fd < 0) {
        testFail("msync unmapped range open failed fd=%d", (S32)fd);
        goto done;
    }
    expectZero("msync unmapped range truncate", process->ftruncate64(fd, K_PAGE_SIZE));
    KThread::setCurrentThread(thread);
    if (process->memory->mmap(thread, MAP_ADDRESS, K_PAGE_SIZE, K_PROT_READ | K_PROT_WRITE,
        K_MAP_SHARED | K_MAP_FIXED, fd, 0) != MAP_ADDRESS) {
        testFail("msync unmapped range map failed");
        goto done;
    }
    expectZero("msync unmapped range unmap", process->memory->unmap(MAP_ADDRESS, K_PAGE_SIZE));
    expectU32("msync rejects unmapped former file range",
        process->msync(thread, MAP_ADDRESS, K_PAGE_SIZE, 0), -K_ENOMEM);

done:
    KThread::setCurrentThread(context.thread);
    if (process) {
        if ((S32)fd >= 0) process->close(fd);
        KSystem::eraseProcess(process->id);
        process = nullptr;
    }
    cleanupRoot(root);
}

void testMsyncFlushesRangeAcrossFixedFileOverlay() {
    TestContext& context = testContext();
    constexpr U32 MAP_ADDRESS = 0x04400000;
    constexpr U32 FIRST_VALUE = 0x12345678;
    constexpr U32 SECOND_VALUE = 0x89abcdef;
    const BString firstPath = B("/tmp/msync-overlay-first");
    const BString secondPath = B("/tmp/msync-overlay-second");
    const BString root = B("tmp/test-msync-overlay-root");

    cleanupRoot(root);
    initTestFileSystem(root);
    Fs::makeLocalDirs(B("/tmp"));
    Fs::makeLocalDirs(B("/proc"));
    setTestProcNode();

    KThread* thread = nullptr;
    KProcessPtr process = createProcessWithMemory(&thread);
    U32 firstFd = process->open(firstPath, K_O_CREAT | K_O_TRUNC | K_O_RDWR, 0666);
    U32 secondFd = process->open(secondPath, K_O_CREAT | K_O_TRUNC | K_O_RDWR, 0666);
    std::shared_ptr<KFile> firstFile;
    std::shared_ptr<KFile> secondFile;

    if ((S32)firstFd < 0 || (S32)secondFd < 0) {
        testFail("msync overlay open failed first=%d second=%d", (S32)firstFd, (S32)secondFd);
        goto done;
    }
    expectZero("msync overlay first truncate", process->ftruncate64(firstFd, K_PAGE_SIZE * 2));
    expectZero("msync overlay second truncate", process->ftruncate64(secondFd, K_PAGE_SIZE));
    KThread::setCurrentThread(thread);
    if (process->memory->mmap(thread, MAP_ADDRESS, K_PAGE_SIZE * 2, K_PROT_READ | K_PROT_WRITE,
        K_MAP_SHARED | K_MAP_FIXED, firstFd, 0) != MAP_ADDRESS) {
        testFail("msync overlay first map failed");
        goto done;
    }
    if (process->memory->mmap(thread, MAP_ADDRESS + K_PAGE_SIZE, K_PAGE_SIZE, K_PROT_READ | K_PROT_WRITE,
        K_MAP_SHARED | K_MAP_FIXED, secondFd, 0) != MAP_ADDRESS + K_PAGE_SIZE) {
        testFail("msync overlay second map failed");
        goto done;
    }
    process->memory->writed(MAP_ADDRESS, FIRST_VALUE);
    process->memory->writed(MAP_ADDRESS + K_PAGE_SIZE, SECOND_VALUE);
    expectZero("msync flushes range spanning fixed file overlay",
        process->msync(thread, MAP_ADDRESS, K_PAGE_SIZE * 2, 0));

    firstFile = std::dynamic_pointer_cast<KFile>(process->getFileDescriptor(firstFd)->kobject);
    secondFile = std::dynamic_pointer_cast<KFile>(process->getFileDescriptor(secondFd)->kobject);
    {
        U32 value = 0;
        expectU32("msync overlay first backing read count",
            firstFile->preadNativeUncached((U8*)&value, 0, sizeof(value)), sizeof(value));
        expectU32("msync overlay first backing value", value, FIRST_VALUE);
    }
    {
        U32 value = 0;
        expectU32("msync overlay second backing read count",
            secondFile->preadNativeUncached((U8*)&value, 0, sizeof(value)), sizeof(value));
        expectU32("msync overlay second backing value", value, SECOND_VALUE);
    }

done:
    KThread::setCurrentThread(context.thread);
    if (process) {
        if ((S32)secondFd >= 0) process->close(secondFd);
        if ((S32)firstFd >= 0) process->close(firstFd);
        KSystem::eraseProcess(process->id);
        process = nullptr;
    }
    cleanupRoot(root);
}

void testMsyncValidatesArgumentsAndAcceptsAnonymousRanges() {
    TestContext& context = testContext();
    constexpr U32 MAP_ADDRESS = 0x04500000;
    constexpr U32 TEST_MS_ASYNC = 1;
    constexpr U32 TEST_MS_INVALIDATE = 2;
    constexpr U32 TEST_MS_SYNC = 4;
    constexpr U32 UNKNOWN_FLAG = 8;
    const BString root = B("tmp/test-msync-argument-validation-root");

    cleanupRoot(root);
    initTestFileSystem(root);
    Fs::makeLocalDirs(B("/proc"));
    setTestProcNode();

    KThread* thread = nullptr;
    KProcessPtr process = createProcessWithMemory(&thread);

    KThread::setCurrentThread(thread);
    if (process->memory->mmap(thread, MAP_ADDRESS, K_PAGE_SIZE,
        K_PROT_READ | K_PROT_WRITE,
        K_MAP_PRIVATE | K_MAP_ANONYMOUS | K_MAP_FIXED, -1, 0) !=
            MAP_ADDRESS) {
        testFail("msync validation anonymous map failed");
        goto done;
    }

    expectU32("msync rejects unaligned address",
        process->msync(thread, MAP_ADDRESS + 1, 1, 0), -K_EINVAL);
    expectU32("msync rejects unknown flags",
        process->msync(thread, MAP_ADDRESS, 1, UNKNOWN_FLAG), -K_EINVAL);
    expectU32("msync rejects contradictory sync flags",
        process->msync(thread, MAP_ADDRESS, 1,
            TEST_MS_ASYNC | TEST_MS_SYNC), -K_EINVAL);
    expectZero("msync accepts Linux flags zero",
        process->msync(thread, MAP_ADDRESS, 1, 0));
    expectZero("msync accepts asynchronous anonymous range",
        process->msync(thread, MAP_ADDRESS, 1, TEST_MS_ASYNC));
    expectZero("msync accepts synchronous invalidate anonymous range",
        process->msync(thread, MAP_ADDRESS, 1,
            TEST_MS_SYNC | TEST_MS_INVALIDATE));
    expectZero("msync accepts zero length",
        process->msync(thread, MAP_ADDRESS, 0, 0));

done:
    KThread::setCurrentThread(context.thread);
    if (process) {
        KSystem::eraseProcess(process->id);
        process = nullptr;
    }
    cleanupRoot(root);
}

void testMsyncSnapshotSurvivesConcurrentPrefixTrim() {
#if !defined(BOXEDWINE_MULTI_THREADED)
    return;
#else
    TestContext& context = testContext();
    constexpr U32 MAP_ADDRESS = 0x04d00000;
    constexpr U32 EXPECTED = 0x6a7b8c9d;
    const BString path = B("/tmp/msync-snapshot-prefix-trim");
    const BString root = B("tmp/test-msync-snapshot-prefix-trim-root");

    cleanupRoot(root);
    initTestFileSystem(root);
    Fs::makeLocalDirs(B("/tmp"));
    Fs::makeLocalDirs(B("/proc"));
    setTestProcNode();

    KThread* thread = nullptr;
    KProcessPtr process = createProcessWithMemory(&thread);
    U32 fd = process->open(path, K_O_CREAT | K_O_TRUNC | K_O_RDWR, 0666);
    TestPausePoint snapshotPause;
    std::thread syncThread;
    std::atomic<U32> syncResult((U32)-K_EIO);
    std::shared_ptr<KFile> file;

    if ((S32)fd < 0) {
        testFail("msync snapshot open failed fd=%d", (S32)fd);
        goto done;
    }
    expectZero("msync snapshot truncate", process->ftruncate64(fd, K_PAGE_SIZE * 2));
    KThread::setCurrentThread(thread);
    if (process->memory->mmap(thread, MAP_ADDRESS, K_PAGE_SIZE * 2, K_PROT_READ | K_PROT_WRITE,
        K_MAP_SHARED | K_MAP_FIXED, fd, 0) != MAP_ADDRESS) {
        testFail("msync snapshot map failed");
        goto done;
    }
    process->memory->writed(MAP_ADDRESS + 0x120, EXPECTED);
    file = std::dynamic_pointer_cast<KFile>(process->getFileDescriptor(fd)->kobject);
    process->setTestAfterMappedFileRangeSnapshotHook([&]() { snapshotPause.pause(); });

    syncThread = std::thread([&]() {
        syncResult.store(process->msync(thread, MAP_ADDRESS, K_PAGE_SIZE, 0), std::memory_order_release);
    });
    if (!snapshotPause.waitUntilReached(2000)) {
        testFail("msync snapshot did not pause after mapped-range selection");
        snapshotPause.release();
        goto done;
    }
    expectZero("msync snapshot concurrent prefix trim",
        process->memory->unmap(MAP_ADDRESS, K_PAGE_SIZE));
    snapshotPause.release();
    syncThread.join();

    expectZero("msync snapshot result after prefix trim", syncResult.load(std::memory_order_acquire));
    {
        U32 value = 0;
        expectU32("msync snapshot backing read count",
            file->preadNativeUncached((U8*)&value, 0x120, sizeof(value)), sizeof(value));
        expectU32("msync snapshot persisted original range", value, EXPECTED);
    }

done:
    snapshotPause.release();
    if (syncThread.joinable()) syncThread.join();
    if (process) process->setTestAfterMappedFileRangeSnapshotHook(nullptr);
    KThread::setCurrentThread(context.thread);
    if (process) {
        if ((S32)fd >= 0) process->close(fd);
        KSystem::eraseProcess(process->id);
        process = nullptr;
    }
    cleanupRoot(root);
#endif
}

void testMremapRoundsNonAlignedShrink() {
    TestContext& context = testContext();
    constexpr U32 MAP_ADDRESS = 0x04e00000;
    constexpr U32 OLD_SIZE = K_PAGE_SIZE * 3 - 17;
    constexpr U32 NEW_SIZE = K_PAGE_SIZE + 7;
    const BString path = B("/tmp/mremap-rounded-shrink");
    const BString root = B("tmp/test-mremap-rounded-shrink-root");

    cleanupRoot(root);
    initTestFileSystem(root);
    Fs::makeLocalDirs(B("/tmp"));
    Fs::makeLocalDirs(B("/proc"));
    setTestProcNode();

    KThread* thread = nullptr;
    KProcessPtr process = createProcessWithMemory(&thread);
    U32 fd = process->open(path, K_O_CREAT | K_O_TRUNC | K_O_RDWR, 0666);

    if ((S32)fd < 0) {
        testFail("mremap rounded shrink open failed fd=%d", (S32)fd);
        goto done;
    }
    expectZero("mremap rounded shrink truncate", process->ftruncate64(fd, K_PAGE_SIZE * 3));
    KThread::setCurrentThread(thread);
    if (process->memory->mmap(thread, MAP_ADDRESS, K_PAGE_SIZE * 3, K_PROT_READ | K_PROT_WRITE,
        K_MAP_SHARED | K_MAP_FIXED, fd, 0) != MAP_ADDRESS) {
        testFail("mremap rounded shrink map failed");
        goto done;
    }

    expectU32("mremap rounded shrink result",
        process->memory->mremap(thread, MAP_ADDRESS, OLD_SIZE, NEW_SIZE, 0), MAP_ADDRESS);
    if (!(process->memory->getPageFlags(MAP_ADDRESS >> K_PAGE_SHIFT) & PAGE_MAPPED) ||
        !(process->memory->getPageFlags((MAP_ADDRESS >> K_PAGE_SHIFT) + 1) & PAGE_MAPPED)) {
        testFail("mremap rounded shrink removed a retained rounded page");
    }
    expectU32("mremap rounded shrink removed trailing page flags",
        process->memory->getPageFlags((MAP_ADDRESS >> K_PAGE_SHIFT) + 2), 0);
    if (!process->getMappedFileForRange(MAP_ADDRESS, K_PAGE_SIZE * 2)) {
        testFail("mremap rounded shrink lost retained file record");
    }
    if (process->getMappedFileForRange(MAP_ADDRESS + K_PAGE_SIZE * 2, K_PAGE_SIZE)) {
        testFail("mremap rounded shrink kept removed file record range");
    }

done:
    KThread::setCurrentThread(context.thread);
    if (process) {
        if ((S32)fd >= 0) process->close(fd);
        KSystem::eraseProcess(process->id);
        process = nullptr;
    }
    cleanupRoot(root);
}

void testFileBackedMremapGrowthIsRejected() {
    TestContext& context = testContext();
    constexpr U32 IN_PLACE_ADDRESS = 0x04f00000;
    constexpr U32 MOVE_ADDRESS = 0x05000000;
    constexpr U32 MREMAP_MAYMOVE = 1;
    const BString path = B("/tmp/mremap-file-growth");
    const BString root = B("tmp/test-mremap-file-growth-root");

    cleanupRoot(root);
    initTestFileSystem(root);
    Fs::makeLocalDirs(B("/tmp"));
    Fs::makeLocalDirs(B("/proc"));
    setTestProcNode();

    KThread* thread = nullptr;
    KProcessPtr process = createProcessWithMemory(&thread);
    U32 fd = process->open(path, K_O_CREAT | K_O_TRUNC | K_O_RDWR, 0666);

    if ((S32)fd < 0) {
        testFail("file mremap growth open failed fd=%d", (S32)fd);
        goto done;
    }
    expectZero("file mremap growth truncate",
        process->ftruncate64(fd, K_PAGE_SIZE * 2));
    KThread::setCurrentThread(thread);

    if (process->memory->mmap(thread, IN_PLACE_ADDRESS, K_PAGE_SIZE,
        K_PROT_READ | K_PROT_WRITE, K_MAP_SHARED | K_MAP_FIXED, fd, 0) !=
            IN_PLACE_ADDRESS) {
        testFail("file mremap in-place source map failed");
        goto done;
    }
    expectU32("file mremap rejects in-place growth",
        process->memory->mremap(thread, IN_PLACE_ADDRESS, K_PAGE_SIZE,
            K_PAGE_SIZE * 2, 0), -K_ENOMEM);
    if (!process->getMappedFileForRange(IN_PLACE_ADDRESS, K_PAGE_SIZE)) {
        testFail("file mremap in-place rejection lost source record");
    }
    expectU32("file mremap in-place rejection leaves growth page unmapped",
        process->memory->isPageMapped(
            (IN_PLACE_ADDRESS + K_PAGE_SIZE) >> K_PAGE_SHIFT), 0);

    if (process->memory->mmap(thread, MOVE_ADDRESS, K_PAGE_SIZE,
        K_PROT_READ | K_PROT_WRITE, K_MAP_SHARED | K_MAP_FIXED, fd, 0) !=
            MOVE_ADDRESS) {
        testFail("file mremap movable source map failed");
        goto done;
    }
    if (process->memory->mmap(thread, MOVE_ADDRESS + K_PAGE_SIZE, K_PAGE_SIZE,
        K_PROT_READ | K_PROT_WRITE,
        K_MAP_PRIVATE | K_MAP_ANONYMOUS | K_MAP_FIXED, -1, 0) !=
            MOVE_ADDRESS + K_PAGE_SIZE) {
        testFail("file mremap movable blocker map failed");
        goto done;
    }
    expectU32("file mremap rejects anonymous move fallback",
        process->memory->mremap(thread, MOVE_ADDRESS, K_PAGE_SIZE,
            K_PAGE_SIZE * 2, MREMAP_MAYMOVE), -K_ENOMEM);
    if (!process->getMappedFileForRange(MOVE_ADDRESS, K_PAGE_SIZE)) {
        testFail("file mremap move rejection lost source record");
    }
    expectU32("file mremap move rejection preserves blocker",
        process->memory->isPageMapped(
            (MOVE_ADDRESS + K_PAGE_SIZE) >> K_PAGE_SHIFT), 1);

done:
    KThread::setCurrentThread(context.thread);
    if (process) {
        if ((S32)fd >= 0) process->close(fd);
        KSystem::eraseProcess(process->id);
        process = nullptr;
    }
    cleanupRoot(root);
}

void testMprotectRejectsWriteAcrossReadOnlySharedMapping() {
    TestContext& context = testContext();
    constexpr U32 ANONYMOUS_ADDRESS = 0x05100000;
    constexpr U32 SHARED_ADDRESS = ANONYMOUS_ADDRESS + K_PAGE_SIZE;
    constexpr U32 PRIVATE_ADDRESS = 0x05200000;
    const BString path = B("/tmp/mprotect-read-only-shared");
    const BString root = B("tmp/test-mprotect-read-only-shared-root");

    cleanupRoot(root);
    initTestFileSystem(root);
    Fs::makeLocalDirs(B("/tmp"));
    Fs::makeLocalDirs(B("/proc"));
    setTestProcNode();

    KThread* thread = nullptr;
    KProcessPtr process = createProcessWithMemory(&thread);
    U32 writableFd =
        process->open(path, K_O_CREAT | K_O_TRUNC | K_O_RDWR, 0666);
    U32 readOnlyFd = (U32)-1;

    if ((S32)writableFd < 0) {
        testFail("mprotect read-only shared create failed fd=%d",
            (S32)writableFd);
        goto done;
    }
    expectZero("mprotect read-only shared truncate",
        process->ftruncate64(writableFd, K_PAGE_SIZE));
    expectZero("mprotect read-only shared close writable descriptor",
        process->close(writableFd));
    writableFd = (U32)-1;
    readOnlyFd = process->open(path, K_O_RDONLY, 0);
    if ((S32)readOnlyFd < 0) {
        testFail("mprotect read-only shared open failed fd=%d",
            (S32)readOnlyFd);
        goto done;
    }

    KThread::setCurrentThread(thread);
    if (process->memory->mmap(thread, ANONYMOUS_ADDRESS, K_PAGE_SIZE,
        K_PROT_READ, K_MAP_PRIVATE | K_MAP_ANONYMOUS | K_MAP_FIXED,
        -1, 0) != ANONYMOUS_ADDRESS) {
        testFail("mprotect read-only shared anonymous map failed");
        goto done;
    }
    if (process->memory->mmap(thread, SHARED_ADDRESS, K_PAGE_SIZE,
        K_PROT_READ, K_MAP_SHARED | K_MAP_FIXED, readOnlyFd, 0) !=
            SHARED_ADDRESS) {
        testFail("mprotect read-only shared file map failed");
        goto done;
    }

    expectU32("mprotect rejects range containing read-only shared file",
        process->memory->mprotect(thread, ANONYMOUS_ADDRESS,
            K_PAGE_SIZE * 2, K_PROT_READ | K_PROT_WRITE), -K_EACCES);
    expectU32("mprotect rejection leaves anonymous page read-only",
        process->memory->getPageFlags(
            ANONYMOUS_ADDRESS >> K_PAGE_SHIFT) & PAGE_WRITE, 0);
    expectU32("mprotect rejection leaves shared file page read-only",
        process->memory->getPageFlags(
            SHARED_ADDRESS >> K_PAGE_SHIFT) & PAGE_WRITE, 0);

    if (process->memory->mmap(thread, PRIVATE_ADDRESS, K_PAGE_SIZE,
        K_PROT_READ, K_MAP_PRIVATE | K_MAP_FIXED, readOnlyFd, 0) !=
            PRIVATE_ADDRESS) {
        testFail("mprotect read-only private file map failed");
        goto done;
    }
    expectZero("mprotect permits private write on read-only file",
        process->memory->mprotect(thread, PRIVATE_ADDRESS, K_PAGE_SIZE,
            K_PROT_READ | K_PROT_WRITE));
    expectU32("mprotect private mapping becomes writable",
        process->memory->getPageFlags(
            PRIVATE_ADDRESS >> K_PAGE_SHIFT) & PAGE_WRITE, PAGE_WRITE);

done:
    KThread::setCurrentThread(context.thread);
    if (process) {
        if ((S32)readOnlyFd >= 0) process->close(readOnlyFd);
        if ((S32)writableFd >= 0) process->close(writableFd);
        KSystem::eraseProcess(process->id);
        process = nullptr;
    }
    cleanupRoot(root);
}

void testUnmapNativeMemoryRemovesOffsetReservation() {
    TestContext& context = testContext();
    constexpr U32 HOST_OFFSET = 137;
    constexpr U32 SIZE = K_PAGE_SIZE + 211;
    const BString root = B("tmp/test-native-offset-unmap-root");

    cleanupRoot(root);
    initTestFileSystem(root);
    Fs::makeLocalDirs(B("/proc"));
    setTestProcNode();

    KThread* thread = nullptr;
    KProcessPtr process = createProcessWithMemory(&thread);
    std::vector<U8> host(K_PAGE_SIZE * 4);
    uintptr_t raw = reinterpret_cast<uintptr_t>(host.data());
    uintptr_t aligned = (raw + K_PAGE_MASK) & ~(uintptr_t)K_PAGE_MASK;
    U8* hostAddress = reinterpret_cast<U8*>(aligned + HOST_OFFSET);
    U32 guestAddress = process->memory->mapNativeMemory(hostAddress, SIZE);

    if (!guestAddress) {
        testFail("native offset map failed");
        goto done;
    }
    expectU32("native offset preserved in guest address", guestAddress & K_PAGE_MASK, HOST_OFFSET);
    {
        U32 dataPage = guestAddress >> K_PAGE_SHIFT;
        U32 guardPage = dataPage - 1;
        U32 dataPageCount = (HOST_OFFSET + SIZE + K_PAGE_MASK) >> K_PAGE_SHIFT;
        expectU32("native offset leading guard reserved",
            process->memory->getPageFlags(guardPage), PAGE_MAPPED);
        for (U32 i = 0; i < dataPageCount; ++i) {
            if (!process->memory->isPageNative(dataPage + i)) {
                testFail("native offset data page %u was not native", i);
            }
        }
        expectU32("native offset trailing guard reserved",
            process->memory->getPageFlags(dataPage + dataPageCount), PAGE_MAPPED);

        process->memory->unmapNativeMemory(guestAddress, SIZE);
        for (U32 i = 0; i < dataPageCount + 2; ++i) {
            U32 page = guardPage + i;
            expectU32("native offset unmap cleared reservation",
                process->memory->getPageFlags(page), 0);
            if (process->memory->isPageNative(page)) {
                testFail("native offset unmap retained native page %u", page);
            }
        }
    }

done:
    KThread::setCurrentThread(context.thread);
    if (process) {
        KSystem::eraseProcess(process->id);
        process = nullptr;
    }
    cleanupRoot(root);
}

void testMappedFileOffsetPageIndexBoundary() {
    TestContext& context = testContext();
    constexpr U32 MAP_ADDRESS = 0x04f00000;
    constexpr U64 LAST_REPRESENTABLE_OFFSET =
        ((U64)std::numeric_limits<U32>::max() - 1) << K_PAGE_SHIFT;
    constexpr U64 FIRST_UNREPRESENTABLE_OFFSET =
        (U64)std::numeric_limits<U32>::max() << K_PAGE_SHIFT;
    constexpr U32 LOW_VALUE = 0x13572468;
    const BString path = B("/tmp/mapped-file-offset-boundary");
    const BString root = B("tmp/test-mapped-file-offset-boundary-root");

    cleanupRoot(root);
    initTestFileSystem(root);
    Fs::makeLocalDirs(B("/tmp"));
    Fs::makeLocalDirs(B("/proc"));
    setTestProcNode();

    KThread* thread = nullptr;
    KProcessPtr process = createProcessWithMemory(&thread);
    U32 fd = process->open(path, K_O_CREAT | K_O_TRUNC | K_O_RDWR, 0666);
    std::shared_ptr<KFile> file;

    if ((S32)fd < 0) {
        testFail("mapped offset boundary open failed fd=%d", (S32)fd);
        goto done;
    }
    file = std::dynamic_pointer_cast<KFile>(process->getFileDescriptor(fd)->kobject);
    {
        U32 value = LOW_VALUE;
        expectU32("mapped offset boundary low write count",
            file->pwriteNative((U8*)&value, 0, sizeof(value)), sizeof(value));
    }
    {
        U32 value = 0xa5a5a5a5;
        expectU32("mapped offset boundary high descriptor read count",
            file->preadNativeUncached((U8*)&value, FIRST_UNREPRESENTABLE_OFFSET, sizeof(value)), 0);
        expectU32("mapped offset boundary high descriptor read did not alias low bytes",
            value, 0xa5a5a5a5);
    }

    KThread::setCurrentThread(thread);
    if (process->memory->mmap(thread, MAP_ADDRESS, K_PAGE_SIZE, K_PROT_READ,
        K_MAP_PRIVATE | K_MAP_FIXED, fd, LAST_REPRESENTABLE_OFFSET) != MAP_ADDRESS) {
        testFail("mapped offset boundary last representable page was rejected");
        goto done;
    }
    {
        MappedFilePtr mapping = process->getMappedFileForRange(MAP_ADDRESS, K_PAGE_SIZE);
        if (!mapping) {
            testFail("mapped offset boundary did not publish the valid record");
        } else {
            expectU64("mapped offset boundary preserved exact file offset",
                mapping->offset, LAST_REPRESENTABLE_OFFSET);
        }
    }
    expectZero("mapped offset boundary valid cleanup",
        process->memory->unmap(MAP_ADDRESS, K_PAGE_SIZE));
    expectU32("mapped offset boundary rejected first unrepresentable page",
        process->memory->mmap(thread, MAP_ADDRESS, K_PAGE_SIZE, K_PROT_READ,
            K_MAP_PRIVATE | K_MAP_FIXED, fd, FIRST_UNREPRESENTABLE_OFFSET), -K_EINVAL);
    expectU32("mapped offset boundary rejected mapping left page unmapped",
        process->memory->getPageFlags(MAP_ADDRESS >> K_PAGE_SHIFT), 0);

done:
    KThread::setCurrentThread(context.thread);
    if (process) {
        if ((S32)fd >= 0) process->close(fd);
        KSystem::eraseProcess(process->id);
        process = nullptr;
    }
    cleanupRoot(root);
}

void testMiddleFixedOverlayRekeysRightFileMapping() {
    TestContext& context = testContext();
    constexpr U32 MAP_ADDRESS = 0x05000000;
    constexpr U32 VALUE_OFFSET = 0x88;
    constexpr U32 RIGHT_VALUE = 0x24681357;
    constexpr U32 UPDATED_RIGHT_VALUE = 0xfedcba98;
    constexpr U32 MIDDLE_VALUE = 0x10293847;
    const BString firstPath = B("/tmp/middle-overlay-first");
    const BString secondPath = B("/tmp/middle-overlay-second");
    const BString root = B("tmp/test-middle-overlay-rekey-root");

    cleanupRoot(root);
    initTestFileSystem(root);
    Fs::makeLocalDirs(B("/tmp"));
    Fs::makeLocalDirs(B("/proc"));
    setTestProcNode();

    KThread* thread = nullptr;
    KProcessPtr process = createProcessWithMemory(&thread);
    U32 firstFd = process->open(firstPath, K_O_CREAT | K_O_TRUNC | K_O_RDWR, 0666);
    U32 secondFd = process->open(secondPath, K_O_CREAT | K_O_TRUNC | K_O_RDWR, 0666);
    std::shared_ptr<KFile> firstFile;
    std::shared_ptr<KFile> secondFile;

    if ((S32)firstFd < 0 || (S32)secondFd < 0) {
        testFail("middle overlay open failed first=%d second=%d", (S32)firstFd, (S32)secondFd);
        goto done;
    }
    expectZero("middle overlay first truncate", process->ftruncate64(firstFd, K_PAGE_SIZE * 3));
    expectZero("middle overlay second truncate", process->ftruncate64(secondFd, K_PAGE_SIZE));
    firstFile = std::dynamic_pointer_cast<KFile>(process->getFileDescriptor(firstFd)->kobject);
    secondFile = std::dynamic_pointer_cast<KFile>(process->getFileDescriptor(secondFd)->kobject);
    {
        U32 value = RIGHT_VALUE;
        expectU32("middle overlay right backing write count",
            firstFile->pwriteNative((U8*)&value, K_PAGE_SIZE * 2 + VALUE_OFFSET, sizeof(value)), sizeof(value));
        value = MIDDLE_VALUE;
        expectU32("middle overlay middle backing write count",
            secondFile->pwriteNative((U8*)&value, VALUE_OFFSET, sizeof(value)), sizeof(value));
    }

    KThread::setCurrentThread(thread);
    if (process->memory->mmap(thread, MAP_ADDRESS, K_PAGE_SIZE * 3, K_PROT_READ | K_PROT_WRITE,
        K_MAP_SHARED | K_MAP_FIXED, firstFd, 0) != MAP_ADDRESS) {
        testFail("middle overlay first map failed");
        goto done;
    }
    if (process->memory->mmap(thread, MAP_ADDRESS + K_PAGE_SIZE, K_PAGE_SIZE,
        K_PROT_READ | K_PROT_WRITE, K_MAP_SHARED | K_MAP_FIXED, secondFd, 0) != MAP_ADDRESS + K_PAGE_SIZE) {
        testFail("middle overlay replacement map failed");
        goto done;
    }

    {
        MappedFilePtr left = process->getMappedFileForRange(MAP_ADDRESS, K_PAGE_SIZE);
        MappedFilePtr middle = process->getMappedFileForRange(MAP_ADDRESS + K_PAGE_SIZE, K_PAGE_SIZE);
        MappedFilePtr right = process->getMappedFileForRange(MAP_ADDRESS + K_PAGE_SIZE * 2, K_PAGE_SIZE);
        if (!left || !middle || !right) {
            testFail("middle overlay did not retain three records");
        } else {
            if (left->key == right->key) testFail("middle overlay right record was not rekeyed");
            if (left->file != firstFile || right->file != firstFile || middle->file != secondFile) {
                testFail("middle overlay selected the wrong backing file");
            }
            expectU64("middle overlay right adjusted offset", right->offset, K_PAGE_SIZE * 2);
        }
    }
    expectU32("middle overlay faulted right file value",
        process->memory->readd(MAP_ADDRESS + K_PAGE_SIZE * 2 + VALUE_OFFSET), RIGHT_VALUE);
    expectU32("middle overlay faulted replacement value",
        process->memory->readd(MAP_ADDRESS + K_PAGE_SIZE + VALUE_OFFSET), MIDDLE_VALUE);
    process->memory->writed(MAP_ADDRESS + K_PAGE_SIZE * 2 + VALUE_OFFSET, UPDATED_RIGHT_VALUE);
    expectZero("middle overlay spanning msync",
        process->msync(thread, MAP_ADDRESS, K_PAGE_SIZE * 3, 0));
    {
        U32 value = 0;
        expectU32("middle overlay right backing read count",
            firstFile->preadNativeUncached((U8*)&value, K_PAGE_SIZE * 2 + VALUE_OFFSET, sizeof(value)), sizeof(value));
        expectU32("middle overlay right persisted adjusted offset", value, UPDATED_RIGHT_VALUE);
    }
    {
        U32 value = 0;
        expectU32("middle overlay replacement backing read count",
            secondFile->preadNativeUncached((U8*)&value, VALUE_OFFSET,
                sizeof(value)), sizeof(value));
        expectU32("middle overlay replacement persisted value",
            value, MIDDLE_VALUE);
    }

done:
    KThread::setCurrentThread(context.thread);
    if (process) {
        if ((S32)secondFd >= 0) process->close(secondFd);
        if ((S32)firstFd >= 0) process->close(firstFd);
        KSystem::eraseProcess(process->id);
        process = nullptr;
    }
    cleanupRoot(root);
}

void testClonePreservesMappedLeasePieceGrouping() {
    TestContext& context = testContext();
    constexpr U32 MAP_ADDRESS = 0x05100000;
    const BString path = B("/tmp/clone-mapped-lease-grouping");
    const BString root = B("tmp/test-clone-mapped-lease-grouping-root");

    cleanupRoot(root);
    initTestFileSystem(root);
    Fs::makeLocalDirs(B("/tmp"));
    Fs::makeLocalDirs(B("/proc"));
    setTestProcNode();

    KThread* sourceThread = nullptr;
    KThread* cloneThread = nullptr;
    KProcessPtr source = createProcessWithMemory(&sourceThread);
    KProcessPtr clone = createProcessWithMemory(&cloneThread);
    U32 fd = source->open(path, K_O_CREAT | K_O_TRUNC | K_O_RDWR, 0666);
    std::shared_ptr<MappedFileCache> cache;
    bool sourceCleaned = false;
    bool cloneCleaned = false;

    if ((S32)fd < 0) {
        testFail("clone lease grouping open failed fd=%d", (S32)fd);
        goto done;
    }
    expectZero("clone lease grouping truncate", source->ftruncate64(fd, K_PAGE_SIZE * 3));
    KThread::setCurrentThread(sourceThread);
    if (source->memory->mmap(sourceThread, MAP_ADDRESS, K_PAGE_SIZE * 3, K_PROT_READ | K_PROT_WRITE,
        K_MAP_SHARED | K_MAP_FIXED, fd, 0) != MAP_ADDRESS) {
        testFail("clone lease grouping map failed");
        goto done;
    }
    expectZero("clone lease grouping split",
        source->memory->unmap(MAP_ADDRESS + K_PAGE_SIZE, K_PAGE_SIZE));
    {
        MappedFilePtr sourceLeft = source->getMappedFileForRange(MAP_ADDRESS, K_PAGE_SIZE);
        MappedFilePtr sourceRight = source->getMappedFileForRange(MAP_ADDRESS + K_PAGE_SIZE * 2, K_PAGE_SIZE);
        if (!sourceLeft || !sourceRight) {
            testFail("clone lease grouping source fragments missing");
            goto done;
        }
        cache = sourceLeft->systemCacheEntry;
        if (sourceLeft->lease != sourceRight->lease) {
            testFail("clone lease grouping source fragments did not share a lease");
        }
        if (sourceLeft->retirementDiagnostic != sourceRight->retirementDiagnostic) {
            testFail("clone lease grouping source fragments did not share retirement diagnostics");
        }
        expectU32("clone lease grouping source piece count",
            sourceLeft->lease->getPieceCountForTest(), 2);

        clone->clone(source);
        MappedFilePtr cloneLeft = clone->getMappedFileForRange(MAP_ADDRESS, K_PAGE_SIZE);
        MappedFilePtr cloneRight = clone->getMappedFileForRange(MAP_ADDRESS + K_PAGE_SIZE * 2, K_PAGE_SIZE);
        if (!cloneLeft || !cloneRight) {
            testFail("clone lease grouping clone fragments missing");
            goto done;
        }
        if (cloneLeft->lease != cloneRight->lease) {
            testFail("clone lease grouping clone fragments did not share a lease");
        }
        if (cloneLeft->retirementDiagnostic != cloneRight->retirementDiagnostic) {
            testFail("clone lease grouping clone fragments did not share retirement diagnostics");
        }
        if (cloneLeft->lease == sourceLeft->lease) {
            testFail("clone lease grouping reused the source lease");
        }
        if (cloneLeft->retirementDiagnostic == sourceLeft->retirementDiagnostic) {
            testFail("clone lease grouping reused the source retirement diagnostic");
        }
        expectU32("clone lease grouping clone piece count",
            cloneLeft->lease->getPieceCountForTest(), 2);
        expectU32("clone lease grouping live cache lease count",
            cache->getMappingLeaseCountForTest(), 2);
    }

    clone->cleanupProcess();
    cloneCleaned = true;
    expectU32("clone lease grouping count after clone retirement",
        cache->getMappingLeaseCountForTest(), 1);
    source->cleanupProcess();
    sourceCleaned = true;
    expectU32("clone lease grouping count after source retirement",
        cache->getMappingLeaseCountForTest(), 0);

done:
    KThread::setCurrentThread(context.thread);
    if (clone) {
        if (!cloneCleaned) clone->cleanupProcess();
        KSystem::eraseProcess(clone->id);
        clone = nullptr;
    }
    if (source) {
        if (!sourceCleaned) source->cleanupProcess();
        KSystem::eraseProcess(source->id);
        source = nullptr;
    }
    cleanupRoot(root);
}

void testCloneRetirementDiagnosticsAreIndependent() {
    TestContext& context = testContext();
    constexpr U32 SOURCE_A_ADDRESS = 0x04700000;
    constexpr U32 SOURCE_B_ADDRESS = 0x04710000;
    constexpr U32 EXPECTED_A = 0x31415926;
    constexpr U32 EXPECTED_B = 0x27182818;
    const BString root = B("tmp/test-clone-retirement-diagnostic-root");

    cleanupRoot(root);
    initTestFileSystem(root);
    Fs::makeLocalDirs(B("/proc"));
    setTestProcNode();

    KThread* sourceThread = nullptr;
    KThread* cloneThread = nullptr;
    KProcessPtr source = createProcessWithMemory(&sourceThread);
    KProcessPtr clone = createProcessWithMemory(&cloneThread);
    auto nodeA = std::make_shared<FsMemNode>(
        1, 1, B("clone-retirement-diagnostic-source-a"));
    auto nodeB = std::make_shared<FsMemNode>(
        2, 1, B("clone-retirement-diagnostic-clone-b"));
    auto* openA = new RetirementWriteTestOpenNode(nodeA);
    auto* openB = new RetirementWriteTestOpenNode(nodeB);
    auto fileA = std::make_shared<KFile>(openA);
    auto fileB = std::make_shared<KFile>(openB);
    KFileDescriptorPtr descriptorA =
        source->allocFileDescriptor(fileA, K_O_RDWR, 0, -1, 0);
    KFileDescriptorPtr descriptorB =
        source->allocFileDescriptor(fileB, K_O_RDWR, 0, -1, 0);
    std::shared_ptr<MappedFileCache> cacheA;
    std::shared_ptr<MappedFileCache> cacheB;
    MappedFilePtr sourceA;
    MappedFilePtr sourceB;
    MappedFilePtr cloneA;
    MappedFilePtr cloneB;

    KThread::setCurrentThread(sourceThread);
    if (source->memory->mmap(sourceThread, SOURCE_A_ADDRESS, K_PAGE_SIZE,
            K_PROT_READ | K_PROT_WRITE, K_MAP_SHARED | K_MAP_FIXED,
            descriptorA->handle, 0) != SOURCE_A_ADDRESS ||
        source->memory->mmap(sourceThread, SOURCE_B_ADDRESS, K_PAGE_SIZE,
            K_PROT_READ | K_PROT_WRITE, K_MAP_SHARED | K_MAP_FIXED,
            descriptorB->handle, 0) != SOURCE_B_ADDRESS) {
        testFail("clone retirement diagnostic source mapping setup failed");
        goto done;
    }
    source->memory->writed(SOURCE_A_ADDRESS, EXPECTED_A);
    source->memory->writed(SOURCE_B_ADDRESS, EXPECTED_B);
    clone->cloneMemoryAndProcessForTest(source, false);
    sourceA = source->getMappedFileForRange(SOURCE_A_ADDRESS, K_PAGE_SIZE);
    sourceB = source->getMappedFileForRange(SOURCE_B_ADDRESS, K_PAGE_SIZE);
    cloneA = clone->getMappedFileForRange(SOURCE_A_ADDRESS, K_PAGE_SIZE);
    cloneB = clone->getMappedFileForRange(SOURCE_B_ADDRESS, K_PAGE_SIZE);
    cacheA = KSystem::getFileCache(nodeA->getFileIdentity());
    cacheB = KSystem::getFileCache(nodeB->getFileIdentity());
    if (!sourceA || !sourceB || !cloneA || !cloneB || !cacheA || !cacheB) {
        testFail("clone retirement diagnostic clone setup failed");
        goto done;
    }
    if (sourceA->retirementDiagnostic == cloneA->retirementDiagnostic ||
        sourceB->retirementDiagnostic == cloneB->retirementDiagnostic) {
        testFail("clone retirement diagnostic reused a source diagnostic");
        goto done;
    }

    KThread::setCurrentThread(cloneThread);
    expectZero("clone retirement diagnostic clone A nonfinal unmap",
        clone->memory->unmap(SOURCE_A_ADDRESS, K_PAGE_SIZE));
    KThread::setCurrentThread(sourceThread);
    expectZero("clone retirement diagnostic source B nonfinal unmap",
        source->memory->unmap(SOURCE_B_ADDRESS, K_PAGE_SIZE));
    expectU32("clone retirement diagnostic source A remaining lease",
        cacheA->getMappingLeaseCountForTest(), 1);
    expectU32("clone retirement diagnostic clone B remaining lease",
        cacheB->getMappingLeaseCountForTest(), 1);

    openA->failureMode = RetirementWriteTestOpenNode::FailureMode::Error;
    expectZero("clone retirement diagnostic source A failing unmap",
        source->memory->unmap(SOURCE_A_ADDRESS, K_PAGE_SIZE));
    cacheB->setTestFailWritebackPreparationAllocation(true);
    KThread::setCurrentThread(cloneThread);
    expectZero("clone retirement diagnostic clone B failing unmap",
        clone->memory->unmap(SOURCE_B_ADDRESS, K_PAGE_SIZE));

    expectU32("clone retirement diagnostic source A error",
        (U32)sourceA->getPendingRetirementErrorForTest(), -K_EIO);
    expectU32("clone retirement diagnostic clone B error",
        (U32)cloneB->getPendingRetirementErrorForTest(), -K_ENOMEM);
    expectU32("clone retirement diagnostic source A report count",
        sourceA->getPendingRetirementErrorReportCountForTest(), 1);
    expectU32("clone retirement diagnostic clone B report count",
        cloneB->getPendingRetirementErrorReportCountForTest(), 1);
    if (sourceA->file->openFile->node->path ==
        cloneB->file->openFile->node->path) {
        testFail("clone retirement diagnostic pending identities were not distinct");
    }

    openA->failureMode = RetirementWriteTestOpenNode::FailureMode::None;
    KProcess::retryPendingMappedFileRetirementsForTest();
    expectU32("clone retirement diagnostic source A success cleared only A",
        (U32)sourceA->getPendingRetirementErrorForTest(), 0);
    expectU32("clone retirement diagnostic clone B remained pending",
        (U32)cloneB->getPendingRetirementErrorForTest(), -K_ENOMEM);
    expectU32("clone retirement diagnostic clone B remained deduplicated",
        cloneB->getPendingRetirementErrorReportCountForTest(), 1);

    cacheB->setTestFailWritebackPreparationAllocation(false);
    KProcess::retryPendingMappedFileRetirementsForTest();
    expectU32("clone retirement diagnostic clone B success cleared B",
        (U32)cloneB->getPendingRetirementErrorForTest(), 0);
    expectU32("clone retirement diagnostic source A report stayed independent",
        sourceA->getPendingRetirementErrorReportCountForTest(), 1);
    expectU32("clone retirement diagnostic clone B report stayed independent",
        cloneB->getPendingRetirementErrorReportCountForTest(), 1);

done:
    openA->failureMode = RetirementWriteTestOpenNode::FailureMode::None;
    if (cacheB) {
        cacheB->setTestFailWritebackPreparationAllocation(false);
    }
    KProcess::retryPendingMappedFileRetirementsForTest();
    KThread::setCurrentThread(context.thread);
    if (clone) {
        KSystem::eraseProcess(clone->id);
        clone = nullptr;
    }
    if (source) {
        KSystem::eraseProcess(source->id);
        source = nullptr;
    }
    cleanupRoot(root);
}

static void runCloneMappedLeaseFailureCase(bool reusedLeaseInsertion) {
    TestContext& context = testContext();
    constexpr U32 MAP_ADDRESS = 0x05300000;
    constexpr U32 VALUE_OFFSET = 0x160;
    constexpr U32 DIRTY_VALUE = 0x7a6b5c4d;
    const BString path = reusedLeaseInsertion
        ? B("/tmp/clone-reused-lease-insert-failure")
        : B("/tmp/clone-new-lease-rehash-failure");
    const BString root = reusedLeaseInsertion
        ? B("tmp/test-clone-reused-lease-insert-failure-root")
        : B("tmp/test-clone-new-lease-rehash-failure-root");

    cleanupRoot(root);
    initTestFileSystem(root);
    Fs::makeLocalDirs(B("/tmp"));
    Fs::makeLocalDirs(B("/proc"));
    setTestProcNode();

    KThread* sourceThread = nullptr;
    KThread* cloneThread = nullptr;
    KProcessPtr source = createProcessWithMemory(&sourceThread);
    KProcessPtr clone = createProcessWithMemory(&cloneThread);
    U32 fd = source->open(path, K_O_CREAT | K_O_TRUNC | K_O_RDWR, 0666);
    std::shared_ptr<KFile> file;
    std::shared_ptr<MappedFileCache> cache;
    bool sourceCleaned = false;
    bool cloneCleaned = false;

    if ((S32)fd < 0) {
        testFail("clone allocation failure open failed fd=%d", (S32)fd);
        goto done;
    }
    expectZero("clone allocation failure truncate",
        source->ftruncate64(fd, K_PAGE_SIZE * 3));
    KThread::setCurrentThread(sourceThread);
    if (source->memory->mmap(sourceThread, MAP_ADDRESS, K_PAGE_SIZE * 3,
        K_PROT_READ | K_PROT_WRITE, K_MAP_SHARED | K_MAP_FIXED, fd, 0) != MAP_ADDRESS) {
        testFail("clone allocation failure map failed");
        goto done;
    }
    expectZero("clone allocation failure split",
        source->memory->unmap(MAP_ADDRESS + K_PAGE_SIZE, K_PAGE_SIZE));
    source->memory->writed(MAP_ADDRESS + VALUE_OFFSET, DIRTY_VALUE);
    {
        MappedFilePtr left = source->getMappedFileForRange(MAP_ADDRESS, K_PAGE_SIZE);
        MappedFilePtr right = source->getMappedFileForRange(
            MAP_ADDRESS + K_PAGE_SIZE * 2, K_PAGE_SIZE);
        if (!left || !right || left->lease != right->lease) {
            testFail("clone allocation failure source lease setup failed");
            goto done;
        }
        cache = left->systemCacheEntry;
        file = left->file;
    }
    expectU32("clone allocation failure initial cache count",
        cache->getMappingLeaseCountForTest(), 1);

    if (reusedLeaseInsertion) {
        BHashTable<U32, MappedFilePtr>::testFailNewInsertAfter(1);
    } else {
        BHashTable<U32, MappedFilePtr>::testFailNextRehash();
    }
    {
        bool allocationFailed = false;
        try {
            clone->clone(source);
        } catch (const std::bad_alloc&) {
            allocationFailed = true;
        } catch (...) {
            BHashTable<U32, MappedFilePtr>::testClearFailureInjection();
            testFail("clone allocation failure threw unexpected exception");
        }
        BHashTable<U32, MappedFilePtr>::testClearFailureInjection();
        if (!allocationFailed) {
            testFail("clone allocation failure injection did not throw");
        }
    }
    expectU32("clone allocation failure preserved cache count",
        cache->getMappingLeaseCountForTest(), 1);
    if (clone->getMappedFileForRange(MAP_ADDRESS, K_PAGE_SIZE) ||
        clone->getMappedFileForRange(MAP_ADDRESS + K_PAGE_SIZE * 2, K_PAGE_SIZE)) {
        testFail("clone allocation failure published a partial mapped-file table");
    }

    clone->cleanupProcess();
    cloneCleaned = true;
    expectU32("clone allocation failure cleanup preserved cache count",
        cache->getMappingLeaseCountForTest(), 1);
    expectZero("clone allocation failure unmap left",
        source->memory->unmap(MAP_ADDRESS, K_PAGE_SIZE));
    expectZero("clone allocation failure unmap right",
        source->memory->unmap(MAP_ADDRESS + K_PAGE_SIZE * 2, K_PAGE_SIZE));
    expectU32("clone allocation failure source cleanup reached zero",
        cache->getMappingLeaseCountForTest(), 0);
    {
        U32 hostValue = 0;
        expectU32("clone allocation failure final host read",
            file->preadNativeUncached((U8*)&hostValue, VALUE_OFFSET, sizeof(hostValue)),
            sizeof(hostValue));
        expectU32("clone allocation failure retained final writeback",
            hostValue, DIRTY_VALUE);
    }

done:
    BHashTable<U32, MappedFilePtr>::testClearFailureInjection();
    KThread::setCurrentThread(context.thread);
    if (clone) {
        if (!cloneCleaned) clone->cleanupProcess();
        KSystem::eraseProcess(clone->id);
        clone = nullptr;
    }
    if (source) {
        if (!sourceCleaned) source->cleanupProcess();
        KSystem::eraseProcess(source->id);
        source = nullptr;
    }
    cleanupRoot(root);
}

void testCloneNewLeaseRehashFailureIsTransactional() {
    runCloneMappedLeaseFailureCase(false);
}

void testCloneReusedLeaseInsertionFailureIsTransactional() {
    runCloneMappedLeaseFailureCase(true);
}

void testClonePostLeaseFailureDoesNotPublishProcess() {
    TestContext& context = testContext();
    constexpr U32 MAP_ADDRESS = 0x05380000;
    const BString path = B("/tmp/clone-post-lease-failure");
    const BString root = B("tmp/test-clone-post-lease-failure-root");

    cleanupRoot(root);
    initTestFileSystem(root);
    Fs::makeLocalDirs(B("/tmp"));
    Fs::makeLocalDirs(B("/proc"));
    setTestProcNode();

    KThread* sourceThread = nullptr;
    KProcessPtr source = createProcessWithMemory(&sourceThread);
    U32 fd = source->open(path, K_O_CREAT | K_O_TRUNC | K_O_RDWR, 0666);
    std::shared_ptr<KFile> file;
    std::shared_ptr<MappedFileCache> cache;

    if ((S32)fd < 0) {
        testFail("clone post-lease failure open failed fd=%d", (S32)fd);
        goto done;
    }
    expectZero("clone post-lease failure truncate",
        source->ftruncate64(fd, K_PAGE_SIZE));
    KThread::setCurrentThread(sourceThread);
    if (source->memory->mmap(sourceThread, MAP_ADDRESS, K_PAGE_SIZE,
        K_PROT_READ | K_PROT_WRITE, K_MAP_SHARED | K_MAP_FIXED, fd, 0) != MAP_ADDRESS) {
        testFail("clone post-lease failure map failed");
        goto done;
    }
    file = std::dynamic_pointer_cast<KFile>(source->getFileDescriptor(fd)->kobject);
    cache = KSystem::getFileCache(file->openFile->node->getFileIdentity());
    if (!cache) {
        testFail("clone post-lease failure cache was not created");
        goto done;
    }
    {
        U32 processCount = KSystem::getProcessCount();
        U32 cloneResult = 0;
        bool threw = false;
        KProcess::setTestFailCloneAfterMappedLeaseRetention(true);
        try {
            cloneResult = source->clone(sourceThread, 0, 0, 0, 0, 0);
        } catch (const std::bad_alloc&) {
            threw = true;
        } catch (...) {
            threw = true;
            testFail("clone post-lease failure threw unexpected exception");
        }
        KProcess::setTestFailCloneAfterMappedLeaseRetention(false);
        if (threw) {
            testFail("clone post-lease failure escaped the clone syscall");
        } else {
            expectU32("clone post-lease failure errno", cloneResult, -K_ENOMEM);
        }
        expectU32("clone post-lease failure process count",
            KSystem::getProcessCount(), processCount);
        expectU32("clone post-lease failure cache lease count",
            cache->getMappingLeaseCountForTest(), 1);
    }

done:
    KProcess::setTestFailCloneAfterMappedLeaseRetention(false);
    KThread::setCurrentThread(context.thread);
    if (source) {
        if ((S32)fd >= 0) {
            source->close(fd);
        }
        KSystem::eraseProcess(source->id);
        source = nullptr;
    }
    cleanupRoot(root);
}

void testCloneMemoryAndMappedSnapshotIsAtomic() {
#if !defined(BOXEDWINE_MULTI_THREADED)
    return;
#else
    TestContext& context = testContext();
    constexpr U32 MAP_ADDRESS = 0x05390000;
    const BString path = B("/tmp/clone-memory-mapped-atomic");
    const BString root = B("tmp/test-clone-memory-mapped-atomic-root");

    cleanupRoot(root);
    initTestFileSystem(root);
    Fs::makeLocalDirs(B("/tmp"));
    Fs::makeLocalDirs(B("/proc"));
    setTestProcNode();

    KThread* sourceThread = nullptr;
    KThread* cloneThread = nullptr;
    KProcessPtr source = createProcessWithMemory(&sourceThread);
    KProcessPtr clone = createProcessWithMemory(&cloneThread);
    U32 fd = source->open(path, K_O_CREAT | K_O_TRUNC | K_O_RDWR, 0666);
    TestPausePoint snapshotPause;
    TestEvent unmapStarted;
    TestEvent unmapDone;
    std::atomic<U32> unmapResult{(U32)-K_EINTR};
    bool sourceCleaned = false;
    bool cloneCleaned = false;

    if ((S32)fd < 0) {
        testFail("clone atomic snapshot open failed fd=%d", (S32)fd);
        goto done;
    }
    expectZero("clone atomic snapshot truncate",
        source->ftruncate64(fd, K_PAGE_SIZE));
    KThread::setCurrentThread(sourceThread);
    if (source->memory->mmap(sourceThread, MAP_ADDRESS, K_PAGE_SIZE,
        K_PROT_READ | K_PROT_WRITE, K_MAP_SHARED | K_MAP_FIXED, fd, 0) != MAP_ADDRESS) {
        testFail("clone atomic snapshot map failed");
        goto done;
    }

    source->setTestAfterCloneMemorySnapshotHook([&]() { snapshotPause.pause(); });
    {
        std::thread cloneWorker([&]() {
            clone->cloneMemoryAndProcessForTest(source, false);
        });
        if (!snapshotPause.waitUntilReached(2000)) {
            testFail("clone atomic snapshot did not pause after memory copy");
            snapshotPause.release();
            cloneWorker.join();
            goto done;
        }
        std::thread unmapWorker([&]() {
            unmapStarted.signal();
            unmapResult.store(source->memory->unmap(MAP_ADDRESS, K_PAGE_SIZE),
                std::memory_order_release);
            unmapDone.signal();
        });
        if (!unmapStarted.wait(2000)) {
            testFail("clone atomic snapshot unmap worker did not start");
        }
        if (unmapDone.wait(100)) {
            testFail("clone atomic snapshot allowed unmap between memory and mapped-file copies");
        }
        snapshotPause.release();
        cloneWorker.join();
        unmapWorker.join();
    }

    expectZero("clone atomic snapshot unmap result",
        unmapResult.load(std::memory_order_acquire));
    if (!clone->getMappedFileForRange(MAP_ADDRESS, K_PAGE_SIZE)) {
        testFail("clone atomic snapshot copied mapped memory without its file record");
    }
    if (!(clone->memory->getPageFlags(MAP_ADDRESS >> K_PAGE_SHIFT) & PAGE_MAPPED)) {
        testFail("clone atomic snapshot did not copy the mapped memory page");
    }

done:
    source->setTestAfterCloneMemorySnapshotHook(nullptr);
    KThread::setCurrentThread(context.thread);
    if (clone) {
        if (!cloneCleaned) {
            clone->cleanupProcess();
            cloneCleaned = true;
        }
        KSystem::eraseProcess(clone->id);
        clone = nullptr;
    }
    if (source) {
        if (!sourceCleaned) {
            source->cleanupProcess();
            sourceCleaned = true;
        }
        if ((S32)fd >= 0) {
            source->close(fd);
        }
        KSystem::eraseProcess(source->id);
        source = nullptr;
    }
    cleanupRoot(root);
#endif
}

void testCloneReservationDefersFinalRetirement() {
#if !defined(BOXEDWINE_MULTI_THREADED)
    return;
#else
    TestContext& context = testContext();
    constexpr U32 MAP_ADDRESS = 0x053a0000;
    constexpr U32 VALUE_OFFSET = 0x2a0;
    constexpr U32 DIRTY_VALUE = 0x6e4c2a18;
    const BString path = B("/tmp/clone-reservation-retirement");
    const BString root = B("tmp/test-clone-reservation-retirement-root");

    cleanupRoot(root);
    initTestFileSystem(root);
    Fs::makeLocalDirs(B("/tmp"));
    Fs::makeLocalDirs(B("/proc"));
    setTestProcNode();

    KThread* sourceThread = nullptr;
    KThread* cloneThread = nullptr;
    KProcessPtr source = createProcessWithMemory(&sourceThread);
    KProcessPtr clone = createProcessWithMemory(&cloneThread);
    U32 fd = source->open(path, K_O_CREAT | K_O_TRUNC | K_O_RDWR, 0666);
    std::shared_ptr<KFile> file;
    std::shared_ptr<MappedFileCache> cache;
    TestPausePoint afterSnapshotPause;
    TestEvent unmapDone;
    std::atomic<U32> unmapResult{(U32)-K_EINTR};
    bool sourceCleaned = false;
    bool cloneCleaned = false;

    if ((S32)fd < 0) {
        testFail("clone reservation retirement open failed fd=%d", (S32)fd);
        goto done;
    }
    expectZero("clone reservation retirement truncate",
        source->ftruncate64(fd, K_PAGE_SIZE));
    KThread::setCurrentThread(sourceThread);
    if (source->memory->mmap(sourceThread, MAP_ADDRESS, K_PAGE_SIZE,
        K_PROT_READ | K_PROT_WRITE, K_MAP_SHARED | K_MAP_FIXED, fd, 0) != MAP_ADDRESS) {
        testFail("clone reservation retirement map failed");
        goto done;
    }
    source->memory->writed(MAP_ADDRESS + VALUE_OFFSET, DIRTY_VALUE);
    file = std::dynamic_pointer_cast<KFile>(source->getFileDescriptor(fd)->kobject);
    cache = KSystem::getFileCache(file->openFile->node->getFileIdentity());
    if (!cache) {
        testFail("clone reservation retirement cache was not created");
        goto done;
    }

    source->setTestAfterCloneSnapshotLocksReleasedHook(
        [&]() { afterSnapshotPause.pause(); });
    {
        std::thread cloneWorker([&]() {
            clone->cloneMemoryAndProcessForTest(source, false);
        });
        if (!afterSnapshotPause.waitUntilReached(2000)) {
            testFail("clone reservation retirement did not pause after snapshot locks");
            afterSnapshotPause.release();
            cloneWorker.join();
            goto done;
        }
        std::thread unmapWorker([&]() {
            unmapResult.store(source->memory->unmap(MAP_ADDRESS, K_PAGE_SIZE),
                std::memory_order_release);
            unmapDone.signal();
        });
        if (!unmapDone.wait(2000)) {
            testFail("clone reservation retirement kept source locks during cache acquisition gap");
        }
        expectZero("clone reservation retirement source unmap result",
            unmapResult.load(std::memory_order_acquire));
        expectU32("clone reservation retirement deferred cache lease",
            cache->getMappingLeaseCountForTest(), 1);
        {
            U32 hostValue = 0;
            expectU32("clone reservation retirement deferred host read count",
                file->preadNativeUncached((U8*)&hostValue, VALUE_OFFSET, sizeof(hostValue)),
                sizeof(hostValue));
            expectU32("clone reservation retirement deferred final writeback",
                hostValue, 0);
        }
        afterSnapshotPause.release();
        cloneWorker.join();
        unmapWorker.join();
    }

    if (!clone->getMappedFileForRange(MAP_ADDRESS, K_PAGE_SIZE)) {
        testFail("clone reservation retirement lost child mapped-file record");
    }
    expectU32("clone reservation retirement child cache lease",
        cache->getMappingLeaseCountForTest(), 1);
    clone->cleanupProcess();
    cloneCleaned = true;
    expectU32("clone reservation retirement final cache lease",
        cache->getMappingLeaseCountForTest(), 0);
    {
        U32 hostValue = 0;
        expectU32("clone reservation retirement final host read count",
            file->preadNativeUncached((U8*)&hostValue, VALUE_OFFSET, sizeof(hostValue)),
            sizeof(hostValue));
        expectU32("clone reservation retirement final persisted bytes",
            hostValue, DIRTY_VALUE);
    }

done:
    source->setTestAfterCloneSnapshotLocksReleasedHook(nullptr);
    KThread::setCurrentThread(context.thread);
    if (clone) {
        if (!cloneCleaned) {
            clone->cleanupProcess();
            cloneCleaned = true;
        }
        KSystem::eraseProcess(clone->id);
        clone = nullptr;
    }
    if (source) {
        if (!sourceCleaned) {
            source->cleanupProcess();
            sourceCleaned = true;
        }
        if ((S32)fd >= 0) {
            source->close(fd);
        }
        KSystem::eraseProcess(source->id);
        source = nullptr;
    }
    cleanupRoot(root);
#endif
}

void testConcurrentMappedLeaseDetachRetiresExactlyOnce() {
#if !defined(BOXEDWINE_MULTI_THREADED)
    return;
#else
    TestContext& context = testContext();
    constexpr U32 MAP_ADDRESS = 0x053c0000;
    constexpr U32 VALUE_OFFSET = 0x2c0;
    constexpr U32 DIRTY_VALUE = 0x7193b5d7;
    const BString path = B("/tmp/concurrent-mapped-lease-detach");
    const BString root = B("tmp/test-concurrent-mapped-lease-detach-root");

    cleanupRoot(root);
    initTestFileSystem(root);
    Fs::makeLocalDirs(B("/tmp"));
    Fs::makeLocalDirs(B("/proc"));
    setTestProcNode();

    KThread* sourceThread = nullptr;
    KThread* cloneThread = nullptr;
    KProcessPtr source = createProcessWithMemory(&sourceThread);
    KProcessPtr clone = createProcessWithMemory(&cloneThread);
    U32 fd = source->open(path, K_O_CREAT | K_O_TRUNC | K_O_RDWR, 0666);
    std::shared_ptr<KFile> file;
    std::shared_ptr<MappedFileCache> cache;
    std::shared_ptr<MappedFileLease> sourceLease;
    std::shared_ptr<MappedFileLease> cloneLease;
    std::shared_ptr<std::function<void()>> detachHook;
    std::atomic<bool> barrierTimedOut{false};
    std::atomic<U32> finalRetirementCount{0};
    TestBarrier detachBarrier;

    if ((S32)fd < 0) {
        testFail("concurrent mapped detach open failed fd=%d", (S32)fd);
        goto done;
    }
    expectZero("concurrent mapped detach truncate",
        source->ftruncate64(fd, K_PAGE_SIZE));
    KThread::setCurrentThread(sourceThread);
    if (source->memory->mmap(sourceThread, MAP_ADDRESS, K_PAGE_SIZE,
        K_PROT_READ | K_PROT_WRITE, K_MAP_SHARED | K_MAP_FIXED, fd, 0) != MAP_ADDRESS) {
        testFail("concurrent mapped detach map failed");
        goto done;
    }
    source->memory->writed(MAP_ADDRESS + VALUE_OFFSET, DIRTY_VALUE);
    file = std::dynamic_pointer_cast<KFile>(source->getFileDescriptor(fd)->kobject);
    cache = KSystem::getFileCache(file->openFile->node->getFileIdentity());
    if (!cache) {
        testFail("concurrent mapped detach cache was not created");
        goto done;
    }
    clone->cloneMemoryAndProcessForTest(source, false);
    {
        MappedFilePtr sourceMapping =
            source->getMappedFileForRange(MAP_ADDRESS, K_PAGE_SIZE);
        MappedFilePtr cloneMapping =
            clone->getMappedFileForRange(MAP_ADDRESS, K_PAGE_SIZE);
        if (!sourceMapping || !cloneMapping ||
            sourceMapping->lease == cloneMapping->lease) {
            testFail("concurrent mapped detach clone did not create an independent lease");
            goto done;
        }
        sourceLease = sourceMapping->lease;
        cloneLease = cloneMapping->lease;
    }
    expectU32("concurrent mapped detach source initial piece count",
        sourceLease->getPieceCountForTest(), 1);
    expectU32("concurrent mapped detach clone initial piece count",
        cloneLease->getPieceCountForTest(), 1);
    expectU32("concurrent mapped detach initial cache leases",
        cache->getMappingLeaseCountForTest(), 2);
    cache->setTestAfterFinalRetirementPreparationHook([&]() {
        finalRetirementCount.fetch_add(1, std::memory_order_relaxed);
    });
    detachHook = std::make_shared<std::function<void()>>([&]() {
        if (!detachBarrier.arriveAndWait(2, 2000)) {
            barrierTimedOut.store(true, std::memory_order_release);
        }
    });
    sourceLease->setTestBeforePieceRemovalHook(detachHook);
    cloneLease->setTestBeforePieceRemovalHook(detachHook);
    {
        std::atomic<U32> sourceResult{(U32)-K_EINTR};
        std::atomic<U32> cloneResult{(U32)-K_EINTR};
        std::thread sourceDetach([&]() {
            sourceResult.store(source->memory->unmap(MAP_ADDRESS, K_PAGE_SIZE),
                std::memory_order_release);
        });
        std::thread cloneDetach([&]() {
            cloneResult.store(clone->memory->unmap(MAP_ADDRESS, K_PAGE_SIZE),
                std::memory_order_release);
        });
        sourceDetach.join();
        cloneDetach.join();
        sourceLease->setTestBeforePieceRemovalHook(nullptr);
        cloneLease->setTestBeforePieceRemovalHook(nullptr);
        if (barrierTimedOut.load(std::memory_order_acquire)) {
            testFail("concurrent mapped detach did not reach both decrements");
        }
        expectZero("concurrent mapped detach source result",
            sourceResult.load(std::memory_order_acquire));
        expectZero("concurrent mapped detach clone result",
            cloneResult.load(std::memory_order_acquire));
    }
    expectU32("concurrent mapped detach source final piece count",
        sourceLease->getPieceCountForTest(), 0);
    expectU32("concurrent mapped detach clone final piece count",
        cloneLease->getPieceCountForTest(), 0);
    expectU32("concurrent mapped detach final cache lease",
        cache->getMappingLeaseCountForTest(), 0);
    expectU32("concurrent mapped detach final retirement count",
        finalRetirementCount.load(std::memory_order_relaxed), 1);
    {
        U32 hostValue = 0;
        expectU32("concurrent mapped detach host read count",
            file->preadNativeUncached((U8*)&hostValue, VALUE_OFFSET, sizeof(hostValue)),
            sizeof(hostValue));
        expectU32("concurrent mapped detach persisted bytes", hostValue, DIRTY_VALUE);
    }

done:
    if (cache) cache->setTestAfterFinalRetirementPreparationHook(nullptr);
    if (sourceLease) sourceLease->setTestBeforePieceRemovalHook(nullptr);
    if (cloneLease) cloneLease->setTestBeforePieceRemovalHook(nullptr);
    KThread::setCurrentThread(context.thread);
    if (clone) {
        KSystem::eraseProcess(clone->id);
        clone = nullptr;
    }
    if (source) {
        if ((S32)fd >= 0) {
            source->close(fd);
        }
        KSystem::eraseProcess(source->id);
        source = nullptr;
    }
    cleanupRoot(root);
#endif
}

void testConcurrentProcessCleanupRunsOnce() {
#if !defined(BOXEDWINE_MULTI_THREADED)
    return;
#else
    TestContext& context = testContext();
    const BString root = B("tmp/test-concurrent-process-cleanup-root");

    cleanupRoot(root);
    initTestFileSystem(root);
    Fs::makeLocalDirs(B("/proc"));
    setTestProcNode();

    KThread* thread = nullptr;
    KProcessPtr process = createProcessWithMemory(&thread);
    std::atomic<U32> hookCalls{0};
    TestPausePoint firstCleanupPaused;
    TestEvent secondCleanupEntered;

    process->setTestBeforeCleanupProcessHook([&]() {
        if (hookCalls.fetch_add(1, std::memory_order_acq_rel) == 0) {
            firstCleanupPaused.pause();
        } else {
            secondCleanupEntered.signal();
        }
    });

    std::thread firstCleanup([&]() {
        KThread::setCurrentThread(thread);
        process->cleanupProcess();
    });
    if (!firstCleanupPaused.waitUntilReached(2000)) {
        testFail("first process cleanup did not reach the entry hook");
        firstCleanupPaused.release();
        firstCleanup.join();
        process->setTestBeforeCleanupProcessHook(nullptr);
        KThread::setCurrentThread(context.thread);
        KSystem::eraseProcess(process->id);
        cleanupRoot(root);
        return;
    }

    std::thread secondCleanup([&]() {
        KThread::setCurrentThread(thread);
        process->cleanupProcess();
    });
    bool cleanupOverlapped = secondCleanupEntered.wait(250);
    firstCleanupPaused.release();
    firstCleanup.join();
    secondCleanup.join();
    process->setTestBeforeCleanupProcessHook(nullptr);

    if (cleanupOverlapped ||
        hookCalls.load(std::memory_order_acquire) != 1) {
        testFail("concurrent process cleanup entered more than once");
    }

    KThread::setCurrentThread(context.thread);
    KSystem::eraseProcess(process->id);
    process = nullptr;
    cleanupRoot(root);
#endif
}

void testCloneVmFailureDoesNotDestroySourceMemory() {
    TestContext& context = testContext();
    constexpr U32 MAP_ADDRESS = 0x053b0000;
    constexpr U32 SENTINEL = 0x5d7f91b3;
    const BString root = B("tmp/test-clone-vm-failure-root");

    cleanupRoot(root);
    initTestFileSystem(root);
    Fs::makeLocalDirs(B("/proc"));
    setTestProcNode();

    KThread* sourceThread = nullptr;
    KProcessPtr source = createProcessWithMemory(&sourceThread);
    KThread::setCurrentThread(sourceThread);
    if (source->memory->mmap(sourceThread, MAP_ADDRESS, K_PAGE_SIZE,
        K_PROT_READ | K_PROT_WRITE, K_MAP_PRIVATE | K_MAP_ANONYMOUS | K_MAP_FIXED,
        -1, 0) != MAP_ADDRESS) {
        testFail("clone VM failure source map failed");
        goto done;
    }
    source->memory->writed(MAP_ADDRESS, SENTINEL);
    {
        U32 processCount = KSystem::getProcessCount();
        KProcess::setTestFailCloneAfterMappedLeaseRetention(true);
        expectU32("clone VM failure errno",
            source->clone(sourceThread, TEST_CLONE_VM, 0, 0, 0, 0), -K_ENOMEM);
        KProcess::setTestFailCloneAfterMappedLeaseRetention(false);
        expectU32("clone VM failure process count",
            KSystem::getProcessCount(), processCount);
        expectU32("clone VM failure preserved source memory",
            source->memory->readd(MAP_ADDRESS), SENTINEL);
    }

done:
    KProcess::setTestFailCloneAfterMappedLeaseRetention(false);
    KThread::setCurrentThread(context.thread);
    if (source) {
        KSystem::eraseProcess(source->id);
        source = nullptr;
    }
    cleanupRoot(root);
}

void testProcessPublicationIsFinal() {
#if !defined(BOXEDWINE_MULTI_THREADED)
    return;
#else
    TestContext& context = testContext();
    const BString root = B("tmp/test-process-publication-final-root");

    cleanupRoot(root);
    initTestFileSystem(root);
    Fs::makeLocalDirs(B("/proc"));
    setTestProcNode();

    bool hookCalled = false;
    bool observedEarlyProcess = false;
    bool observedEarlyProcPath = false;
    bool observedEarlyProcList = false;
    KProcess::setTestDuringProcessPublicationHook([&](U32 id) {
        hookCalled = true;
        std::thread observer([&]() {
            observedEarlyProcess = KSystem::getProcess(id) != nullptr;
            observedEarlyProcPath = Fs::getNodeFromLocalPath(
                B(""), B("/proc/") + BString::valueOf(id), false) != nullptr;
            std::vector<std::shared_ptr<FsNode>> children;
            KSystem::procNode->getAllChildren(children);
            for (const std::shared_ptr<FsNode>& child : children) {
                if (child->name == BString::valueOf(id)) {
                    observedEarlyProcList = true;
                    break;
                }
            }
        });
        observer.join();
    });
    KProcessPtr process;
    try {
        process = KProcess::create();
    } catch (...) {
        testFail("process final publication unexpectedly threw");
    }
    KProcess::setTestDuringProcessPublicationHook(nullptr);

    if (!hookCalled) {
        testFail("process final publication hook was not called");
    }
    if (observedEarlyProcess) {
        testFail("process lookup observed child before final publication");
    }
    if (observedEarlyProcPath) {
        testFail("proc path lookup observed child before final publication");
    }
    if (observedEarlyProcList) {
        testFail("proc directory listing observed child before final publication");
    }
    if (!process || KSystem::getProcess(process->id) != process) {
        testFail("process final publication did not publish the completed process");
    }
    if (!process || !Fs::getNodeFromLocalPath(
        B(""), B("/proc/") + BString::valueOf(process->id), false)) {
        testFail("process final publication did not publish the completed proc tree");
    }

    if (process) {
        KSystem::eraseProcess(process->id);
        process = nullptr;
    }

    U32 failedId = 0;
    bool failureHookCalled = false;
    KProcess::setTestDuringProcessPublicationHook([&](U32 id) {
        failedId = id;
        failureHookCalled = true;
        throw std::bad_alloc();
    });
    try {
        KProcess::create();
        testFail("process publication failure injection did not throw");
    } catch (const std::bad_alloc&) {
    } catch (...) {
        testFail("process publication failure injection threw wrong exception");
    }
    KProcess::setTestDuringProcessPublicationHook(nullptr);
    if (!failureHookCalled) {
        testFail("process publication failure hook was not called");
    }
    if (failedId && KSystem::getProcess(failedId)) {
        testFail("failed process publication left a process-table entry");
    }
    if (failedId && Fs::getNodeFromLocalPath(
        B(""), B("/proc/") + BString::valueOf(failedId), false)) {
        testFail("failed process publication left a proc path");
    }
    if (failedId) {
        std::vector<std::shared_ptr<FsNode>> children;
        KSystem::procNode->getAllChildren(children);
        for (const std::shared_ptr<FsNode>& child : children) {
            if (child->name == BString::valueOf(failedId)) {
                testFail("failed process publication left a proc directory entry");
                break;
            }
        }
    }

    KThread::setCurrentThread(context.thread);
    if (process) {
        KSystem::eraseProcess(process->id);
        process = nullptr;
    }
    cleanupRoot(root);
#endif
}

void testChildrenVisibilityMutexBindsOnce() {
#if !defined(BOXEDWINE_MULTI_THREADED)
    return;
#else
    const BString root = B("tmp/test-children-visibility-binding-root");
    cleanupRoot(root);
    initTestFileSystem(root);
    std::shared_ptr<FsNode> fsRoot =
        Fs::getNodeFromLocalPath(B(""), B("/"), false);
    std::shared_ptr<FsNode> node = Fs::createFileNode(
        B("/visibility"), B(""), B(""), true, fsRoot);
    std::shared_ptr<FsNode> child = Fs::createFileNode(
        B("/visibility/child"), B(""), B(""), false, node);
    BOXEDWINE_MUTEX firstMutex;
    BOXEDWINE_MUTEX differentMutex;
    std::atomic<bool> start{false};
    std::atomic<bool> bindFailed{false};

    std::thread binder([&]() {
        while (!start.load(std::memory_order_acquire)) {
            std::this_thread::yield();
        }
        if (!node->trySetChildrenVisibilityMutexForTest(&firstMutex)) {
            bindFailed.store(true, std::memory_order_release);
        }
    });
    std::thread lookup([&]() {
        while (!start.load(std::memory_order_acquire)) {
            std::this_thread::yield();
        }
        for (U32 i = 0; i < 1000; ++i) {
            node->getChildByName(B("child"));
            node->getChildByNameIgnoreCase(B("CHILD"));
        }
    });
    std::thread listAndCount([&]() {
        while (!start.load(std::memory_order_acquire)) {
            std::this_thread::yield();
        }
        for (U32 i = 0; i < 1000; ++i) {
            std::vector<std::shared_ptr<FsNode>> children;
            node->getAllChildren(children);
            node->getChildCount();
        }
    });
    std::thread writer([&]() {
        while (!start.load(std::memory_order_acquire)) {
            std::this_thread::yield();
        }
        for (U32 i = 0; i < 1000; ++i) {
            node->addChild(child);
            node->removeChildByName(B("child"));
        }
    });
    start.store(true, std::memory_order_release);
    binder.join();
    lookup.join();
    listAndCount.join();
    writer.join();

    if (bindFailed.load(std::memory_order_acquire) ||
        node->getChildrenVisibilityMutexForTest() != &firstMutex) {
        testFail("children visibility mutex first binding was not published");
    }
    if (!node->trySetChildrenVisibilityMutexForTest(&firstMutex)) {
        testFail("children visibility mutex rejected identical rebinding");
    }
    if (node->trySetChildrenVisibilityMutexForTest(&differentMutex) ||
        node->getChildrenVisibilityMutexForTest() != &firstMutex) {
        testFail("children visibility mutex allowed runtime switching");
    }
    cleanupRoot(root);
#endif
}

void testWindowsNoThrowCodeWriteFlushesInstructionCache() {
#if !defined(BOXEDWINE_MSVC)
    return;
#else
    U8* code = Platform::alloc64kBlock(1, true);
    NoThrowCodeWriteTestContext context{code, 0x12345678};
    Platform::resetTestInstructionCacheClearCount();
    Platform::writeCodeToMemory(code, sizeof(context.value),
        writeNoThrowCodeForTest, &context);
    expectU32("Windows no-throw code write value",
        *reinterpret_cast<U32*>(code), context.value);
    expectU32("Windows no-throw code write instruction-cache clear count",
        Platform::getTestInstructionCacheClearCount(), 1);
    Platform::releaseNativeMemory(code, 64 * 1024);
#endif
}

void testMappedRecordAllocationFailureContainment() {
    TestContext& context = testContext();
    constexpr U32 MAP_ADDRESS = 0x05200000;
    constexpr U32 VALUE_OFFSET = 0x180;
    constexpr U32 MAPPED_VALUE = 0x4a5b6c7d;
    const BString path = B("/tmp/mapped-record-allocation-failure");
    const BString replacementPath = B("/tmp/mapped-record-allocation-replacement");
    const BString root = B("tmp/test-mapped-record-allocation-failure-root");

    cleanupRoot(root);
    initTestFileSystem(root);
    Fs::makeLocalDirs(B("/tmp"));
    Fs::makeLocalDirs(B("/proc"));
    setTestProcNode();

    KThread* thread = nullptr;
    KProcessPtr process = createProcessWithMemory(&thread);
    U32 fd = process->open(path, K_O_CREAT | K_O_TRUNC | K_O_RDWR, 0666);
    U32 replacementFd = process->open(replacementPath, K_O_CREAT | K_O_TRUNC | K_O_RDWR, 0666);
    MappedFilePtr before;
    std::shared_ptr<MappedFileCache> oldCache;
    U32 beforeFlags = 0;

    if ((S32)fd < 0 || (S32)replacementFd < 0) {
        testFail("mapped record allocation open failed fd=%d replacement=%d",
            (S32)fd, (S32)replacementFd);
        goto done;
    }
    expectZero("mapped record allocation truncate", process->ftruncate64(fd, K_PAGE_SIZE * 3));
    expectZero("mapped record allocation replacement truncate",
        process->ftruncate64(replacementFd, K_PAGE_SIZE));
    KThread::setCurrentThread(thread);

    process->memory->setTestFailMappedFileRecordAllocation(true);
    expectU32("mapped record mmap allocation failure result",
        process->memory->mmap(thread, MAP_ADDRESS, K_PAGE_SIZE, K_PROT_READ | K_PROT_WRITE,
            K_MAP_SHARED | K_MAP_FIXED, fd, 0), -K_ENOMEM);
    process->memory->setTestFailMappedFileRecordAllocation(false);
    expectU32("mapped record mmap allocation failure left page",
        process->memory->getPageFlags(MAP_ADDRESS >> K_PAGE_SHIFT), 0);
    if (process->getMappedFileForRange(MAP_ADDRESS, K_PAGE_SIZE)) {
        testFail("mapped record mmap allocation failure left a record");
        process->memory->unmap(MAP_ADDRESS, K_PAGE_SIZE);
    }

    if (process->memory->mmap(thread, MAP_ADDRESS, K_PAGE_SIZE, K_PROT_READ | K_PROT_WRITE,
        K_MAP_SHARED | K_MAP_FIXED, fd, 0) != MAP_ADDRESS) {
        testFail("mapped record occupied replacement initial map failed");
        goto done;
    }
    process->memory->writed(MAP_ADDRESS + VALUE_OFFSET, MAPPED_VALUE);
    before = process->getMappedFileForRange(MAP_ADDRESS, K_PAGE_SIZE);
    if (!before) {
        testFail("mapped record occupied replacement initial record missing");
        goto done;
    }
    oldCache = before->systemCacheEntry;
    beforeFlags = process->memory->getPageFlags(MAP_ADDRESS >> K_PAGE_SHIFT);
    expectU32("mapped record occupied replacement initial piece count",
        before->lease->getPieceCountForTest(), 1);
    expectU32("mapped record occupied replacement initial cache count",
        oldCache->getMappingLeaseCountForTest(), 1);

    process->memory->setTestFailMappedFileRecordAllocation(true);
    expectU32("mapped record occupied replacement allocation failure result",
        process->memory->mmap(thread, MAP_ADDRESS, K_PAGE_SIZE,
            K_PROT_READ | K_PROT_WRITE, K_MAP_SHARED | K_MAP_FIXED,
            replacementFd, 0), -K_ENOMEM);
    process->memory->setTestFailMappedFileRecordAllocation(false);
    expectU32("mapped record occupied replacement preserved flags",
        process->memory->getPageFlags(MAP_ADDRESS >> K_PAGE_SHIFT), beforeFlags);
    {
        MappedFilePtr after = process->getMappedFileForRange(MAP_ADDRESS, K_PAGE_SIZE);
        if (!after || after->key != before->key || after->lease != before->lease) {
            testFail("mapped record occupied replacement changed the original record");
        }
    }
    expectU32("mapped record occupied replacement preserved piece count",
        before->lease->getPieceCountForTest(), 1);
    expectU32("mapped record occupied replacement preserved cache count",
        oldCache->getMappingLeaseCountForTest(), 1);
    if (process->memory->getPageFlags(MAP_ADDRESS >> K_PAGE_SHIFT) & PAGE_MAPPED) {
        expectU32("mapped record occupied replacement preserved bytes",
            process->memory->readd(MAP_ADDRESS + VALUE_OFFSET), MAPPED_VALUE);
    } else {
        testFail("mapped record occupied replacement removed page bytes");
    }
    expectZero("mapped record occupied replacement cleanup",
        process->memory->unmap(MAP_ADDRESS, K_PAGE_SIZE));

    if (process->memory->mmap(thread, MAP_ADDRESS, K_PAGE_SIZE * 3, K_PROT_READ | K_PROT_WRITE,
        K_MAP_SHARED | K_MAP_FIXED, fd, 0) != MAP_ADDRESS) {
        testFail("mapped record split allocation map failed");
        goto done;
    }
    before = process->getMappedFileForRange(MAP_ADDRESS, K_PAGE_SIZE * 3);
    if (!before) {
        testFail("mapped record split allocation initial record missing");
        goto done;
    }
    process->memory->setTestFailMappedFileRecordAllocation(true);
    expectU32("mapped record split allocation failure result",
        process->memory->unmap(MAP_ADDRESS + K_PAGE_SIZE, K_PAGE_SIZE), -K_ENOMEM);
    process->memory->setTestFailMappedFileRecordAllocation(false);
    for (U32 i = 0; i < 3; ++i) {
        if (!(process->memory->getPageFlags((MAP_ADDRESS >> K_PAGE_SHIFT) + i) & PAGE_MAPPED)) {
            testFail("mapped record split allocation failure removed page %u", i);
        }
    }
    {
        MappedFilePtr after = process->getMappedFileForRange(MAP_ADDRESS, K_PAGE_SIZE * 3);
        if (!after || after->key != before->key || after->lease != before->lease) {
            testFail("mapped record split allocation failure changed the original record");
        }
        expectU32("mapped record split allocation failure piece count",
            before->lease->getPieceCountForTest(), 1);
    }

done:
    if (process) process->memory->setTestFailMappedFileRecordAllocation(false);
    KThread::setCurrentThread(context.thread);
    if (process) {
        if ((S32)replacementFd >= 0) process->close(replacementFd);
        if ((S32)fd >= 0) process->close(fd);
        KSystem::eraseProcess(process->id);
        process = nullptr;
    }
    cleanupRoot(root);
}

void testBHashTableRehashFailurePreservesEntries() {
    static_assert(!noexcept(BHashTable<U32, BHashConstructorThrowValue>(1)),
        "capacity construction allocates and must be allowed to throw");
    using NoThrowTable = BHashTable<U32, U32>;
    static_assert(noexcept(NoThrowTable()));
    static_assert(noexcept(NoThrowTable(std::declval<NoThrowTable&&>())));
    static_assert(noexcept(swap(
        std::declval<NoThrowTable&>(), std::declval<NoThrowTable&>())));
    static_assert(NoThrowTable::testSlotMoveIsNoexcept());
    static_assert(NoThrowTable::testSlotSwapIsNoexcept());
    using ThrowingSlotTable = BHashTable<U32, BHashThrowingMoveValue>;
    static_assert(!ThrowingSlotTable::testSlotMoveIsNoexcept());
    static_assert(!ThrowingSlotTable::testSlotSwapIsNoexcept());
    // Table moves and swaps transfer only the vector allocation and remain
    // non-throwing even when moving an individual slot can throw.
    static_assert(noexcept(
        ThrowingSlotTable(std::declval<ThrowingSlotTable&&>())));
    static_assert(noexcept(swap(
        std::declval<ThrowingSlotTable&>(), std::declval<ThrowingSlotTable&>())));
    if (!NoThrowTable::testSlotMoveAndSwap()) {
        testFail("bhash slot default/move/swap state transitions");
    }

    bool lengthFailed = false;
    try {
        NoThrowTable impossible(std::numeric_limits<size_t>::max());
    } catch (const std::length_error&) {
        lengthFailed = true;
    } catch (...) {
        testFail("bhash oversized capacity threw an unexpected exception");
    }
    if (!lengthFailed) {
        testFail("bhash oversized capacity was not rejected");
    }

    expectU64("bhash empty organic growth starts at eight slots",
        NoThrowTable::testCheckedGrowthSize(0), 8);
    expectU64("bhash organic growth doubles its current slots",
        NoThrowTable::testCheckedGrowthSize(8), 16);
    bool organicGrowthFailed = false;
    try {
        NoThrowTable::testCheckedGrowthSize(
            std::numeric_limits<size_t>::max() / 2 + 1);
    } catch (const std::length_error&) {
        organicGrowthFailed = true;
    } catch (...) {
        testFail("bhash organic growth overflow threw an unexpected exception");
    }
    if (!organicGrowthFailed) {
        testFail("bhash organic growth overflow was not rejected");
    }

    bool constructionFailed = false;
    BHashConstructorThrowValue::failConstruction = true;
    try {
        BHashTable<U32, BHashConstructorThrowValue> throwingTable(1);
    } catch (const std::bad_alloc&) {
        constructionFailed = true;
    } catch (...) {
        BHashConstructorThrowValue::failConstruction = false;
        testFail("bhash construction threw an unexpected exception");
        return;
    }
    BHashConstructorThrowValue::failConstruction = false;
    if (!constructionFailed) {
        testFail("bhash construction failure was not propagated");
    }

    BHashTable<U32, RehashThrowValue> table;
    constexpr U32 ENTRY_COUNT = 6;

    for (U32 i = 0; i < ENTRY_COUNT; ++i) {
        table.set(i, RehashThrowValue(0x100 + i));
    }
    expectU32("bhash rehash initial size", (U32)table.size(), ENTRY_COUNT);

    bool allocationFailed = false;
    RehashThrowValue::failTransfer = true;
    try {
        table.set(ENTRY_COUNT, RehashThrowValue(0x200));
    } catch (const std::bad_alloc&) {
        allocationFailed = true;
    } catch (...) {
        RehashThrowValue::failTransfer = false;
        testFail("bhash rehash threw an unexpected exception");
        return;
    }
    RehashThrowValue::failTransfer = false;

    if (!allocationFailed) {
        testFail("bhash rehash injection did not throw");
    }
    expectU32("bhash rehash preserved size", (U32)table.size(), ENTRY_COUNT);
    for (U32 i = 0; i < ENTRY_COUNT; ++i) {
        RehashThrowValue value(0);
        if (!table.get(i, value)) {
            testFail("bhash rehash lost key %u", i);
        } else {
            expectU32("bhash rehash preserved value", value.value, 0x100 + i);
        }
    }
}

void testMappedFilePageLoadRetriesAfterMutation() {
#if !defined(BOXEDWINE_MULTI_THREADED)
    return;
#else
    TestContext& context = testContext();
    constexpr U32 MAP_ADDRESS = 0x03600000;
    constexpr U32 FILE_OFFSET = 0x300;
    constexpr U32 ORIGINAL_VALUE = 0x10203040;
    constexpr U32 UPDATED_VALUE = 0x50607080;
    const BString path = B("/tmp/mapped-page-load-mutation");
    const BString root = B("tmp/test-mapped-page-load-mutation-root");

    cleanupRoot(root);
    initTestFileSystem(root);
    Fs::makeLocalDirs(B("/tmp"));
    Fs::makeLocalDirs(B("/proc"));
    setTestProcNode();

    KThread* thread = nullptr;
    KProcessPtr process = createProcessWithMemory(&thread);
    U32 fd = process->open(path, K_O_CREAT | K_O_TRUNC | K_O_RDWR, 0666);
    std::shared_ptr<KFile> file;
    std::shared_ptr<MappedFileCache> cache;
    RamPage loaded;
    std::thread loader;
    std::atomic<U32> hookCalls(0);
    TestPausePoint pageReadPause;

    if ((S32)fd < 0) {
        testFail("mapped page-load mutation open failed fd=%d", (S32)fd);
        goto done;
    }
    expectZero("mapped page-load mutation initial truncate", process->ftruncate64(fd, K_PAGE_SIZE));
    file = std::dynamic_pointer_cast<KFile>(process->getFileDescriptor(fd)->kobject);
    {
        U32 value = ORIGINAL_VALUE;
        expectU32("mapped page-load original write count",
            file->pwriteNative((U8*)&value, FILE_OFFSET, sizeof(value)), sizeof(value));
    }

    KThread::setCurrentThread(thread);
    if (process->memory->mmap(thread, MAP_ADDRESS, K_PAGE_SIZE, K_PROT_READ | K_PROT_WRITE,
        K_MAP_SHARED | K_MAP_FIXED, fd, 0) != MAP_ADDRESS) {
        testFail("mapped page-load mutation map failed");
        goto done;
    }
    cache = KSystem::getFileCache(file->openFile->node->getFileIdentity());
    if (!cache) {
        testFail("mapped page-load mutation cache was not created");
        goto done;
    }
    cache->setTestAfterPageReadHook([&]() {
        if (hookCalls.fetch_add(1, std::memory_order_acq_rel) == 0) {
            pageReadPause.pause();
        }
    });

    loader = std::thread([&]() { loaded = cache->getOrCreatePage(0, true); });
    if (!pageReadPause.waitUntilReached(2000)) {
        testFail("mapped page loader did not reach post-read pause");
        pageReadPause.release();
        goto done;
    }
    {
        U32 value = UPDATED_VALUE;
        expectU32("mapped page-load concurrent write count",
            file->pwriteNative((U8*)&value, FILE_OFFSET, sizeof(value)), sizeof(value));
    }
    pageReadPause.release();
    loader.join();
    cache->setTestAfterPageReadHook(nullptr);
    if (!loaded.value) {
        testFail("mapped page loader returned an empty page");
        goto done;
    }
    {
        U32 loadedValue = 0;
        memcpy(&loadedValue, ramPageGet(loaded) + FILE_OFFSET, sizeof(loadedValue));
        expectU32("mapped page loader retried stale backing read", loadedValue, UPDATED_VALUE);
    }
done:
    pageReadPause.release();
    if (loader.joinable()) loader.join();
    if (cache) cache->setTestAfterPageReadHook(nullptr);
    if (loaded.value) ramPageRelease(loaded);
    KThread::setCurrentThread(context.thread);
    if (process) {
        if ((S32)fd >= 0) process->close(fd);
        KSystem::eraseProcess(process->id);
        process = nullptr;
    }
    cleanupRoot(root);
#endif
}

void testMmapCacheAcquireDoesNotHoldMemoryLock() {
#if !defined(BOXEDWINE_MULTI_THREADED)
    return;
#else
    TestContext& context = testContext();
    constexpr U32 MAP_ADDRESS = 0x03700000;
    constexpr U32 BUFFER_ADDRESS = 0x03710000;
    const BString path = B("/tmp/mmap-cache-lock-order");
    const BString root = B("tmp/test-mmap-cache-lock-order-root");

    cleanupRoot(root);
    initTestFileSystem(root);
    Fs::makeLocalDirs(B("/tmp"));
    Fs::makeLocalDirs(B("/proc"));
    setTestProcNode();

    KThread* mapperThread = nullptr;
    KProcessPtr process = createProcessWithMemory(&mapperThread);
    KThread* writerThread = process->createThread();
    U32 fd = process->open(path, K_O_CREAT | K_O_TRUNC | K_O_RDWR, 0666);
    std::shared_ptr<FsFileIdentity> identity;
    std::thread writer;
    std::thread mapper;
    TestEvent writerHasFileLocks;
    TestEvent mapperReachedCacheAcquire;
    std::atomic<bool> writerTimedOut(false);
    std::atomic<bool> memoryLockWasAvailable(false);
    std::atomic<U32> writerResult((U32)-K_EIO);
    std::atomic<U32> mapperResult((U32)-K_EIO);

    if ((S32)fd < 0) {
        testFail("mmap cache lock-order open failed fd=%d", (S32)fd);
        goto done;
    }
    expectZero("mmap cache lock-order truncate", process->ftruncate64(fd, K_PAGE_SIZE));
    KThread::setCurrentThread(mapperThread);
    if (process->memory->mmap(mapperThread, BUFFER_ADDRESS, K_PAGE_SIZE,
        K_PROT_READ | K_PROT_WRITE,
        K_MAP_PRIVATE | K_MAP_ANONYMOUS | K_MAP_FIXED, -1, 0) != BUFFER_ADDRESS) {
        testFail("mmap cache lock-order buffer map failed");
        goto done;
    }
    process->memory->writed(BUFFER_ADDRESS, 0x11223344);

    identity = std::dynamic_pointer_cast<KFile>(
        process->getFileDescriptor(fd)->kobject)->openFile->node->getFileIdentity();
    identity->testBeforeGuestMemoryAccess = [&]() {
        writerHasFileLocks.signal();
        if (!mapperReachedCacheAcquire.wait(2000)) {
            writerTimedOut.store(true, std::memory_order_release);
        }
        bool acquired = BOXEDWINE_MUTEX_TRY_LOCK(process->memory->mutex);
        memoryLockWasAvailable.store(acquired, std::memory_order_release);
        if (acquired) {
            BOXEDWINE_MUTEX_UNLOCK(process->memory->mutex);
        }
        // Abort the synthetic write so the descriptor locks are released and
        // the mapper can always finish, even when this test exposes the old
        // inverted order.
        return false;
    };
    identity->testBeforeMappedCacheFileLock = [&]() {
        mapperReachedCacheAcquire.signal();
    };

    writer = std::thread([&]() {
        KThread::setCurrentThread(writerThread);
        writerResult.store(process->write(writerThread, fd, BUFFER_ADDRESS, sizeof(U32)),
            std::memory_order_release);
    });
    if (!writerHasFileLocks.wait(2000)) {
        testFail("mmap cache lock-order writer did not acquire file locks");
        mapperReachedCacheAcquire.signal();
    }
    mapper = std::thread([&]() {
        KThread::setCurrentThread(mapperThread);
        mapperResult.store(process->memory->mmap(mapperThread, MAP_ADDRESS, K_PAGE_SIZE,
            K_PROT_READ, K_MAP_PRIVATE | K_MAP_FIXED, fd, 0), std::memory_order_release);
    });
    writer.join();
    mapper.join();

    if (writerTimedOut.load(std::memory_order_acquire)) {
        testFail("mmap cache lock-order writer timed out waiting for mapper");
    }
    if (!memoryLockWasAvailable.load(std::memory_order_acquire)) {
        testFail("mmap cache acquisition held the guest memory lock while waiting for file locks");
    }
    expectU32("mmap cache lock-order synthetic writer result",
        writerResult.load(std::memory_order_acquire), -K_EINTR);
    expectU32("mmap cache lock-order mapping result",
        mapperResult.load(std::memory_order_acquire), MAP_ADDRESS);

done:
    if (identity) {
        identity->testBeforeGuestMemoryAccess = nullptr;
        identity->testBeforeMappedCacheFileLock = nullptr;
    }
    mapperReachedCacheAcquire.signal();
    if (writer.joinable()) writer.join();
    if (mapper.joinable()) mapper.join();
    KThread::setCurrentThread(context.thread);
    if (process) {
        if ((S32)fd >= 0) process->close(fd);
        KSystem::eraseProcess(process->id);
        process = nullptr;
    }
    cleanupRoot(root);
#endif
}

void testExecutableFixedReplacementPreparationFailurePreservesState() {
#if defined(BOXEDWINE_JIT)
    TestContext& context = testContext();
    constexpr U32 MAP_ADDRESS = 0x03800000;
    constexpr U32 MAPPING_LENGTH = K_PAGE_SIZE * 3;
    constexpr U32 CODE_FILE_OFFSET = K_PAGE_SIZE;
    constexpr U32 CODE_ADDRESS = MAP_ADDRESS + CODE_FILE_OFFSET;
    constexpr U32 REPLACEMENT_ADDRESS = CODE_ADDRESS;
    constexpr U32 CODE_VALUE = 0x44332211;
    constexpr U32 PEER_OFFSET = 0x20;
    constexpr U32 PEER_FILE_OFFSET = CODE_FILE_OFFSET + PEER_OFFSET;
    constexpr U32 PEER_ADDRESS = CODE_ADDRESS + PEER_OFFSET;
    constexpr U32 PEER_CODE_VALUE = 0x88776655;
    const BString firstPath = B("/tmp/executable-fixed-preparation-first");
    const BString secondPath = B("/tmp/executable-fixed-preparation-second");
    const BString root = B("tmp/test-executable-fixed-preparation-root");

    cleanupRoot(root);
    initTestFileSystem(root);
    Fs::makeLocalDirs(B("/tmp"));
    Fs::makeLocalDirs(B("/proc"));
    setTestProcNode();

    KThread* thread = nullptr;
    KProcessPtr process = createProcessWithMemory(&thread);
    CPU* cpu = thread->cpu;
    U32 firstFd = process->open(firstPath, K_O_CREAT | K_O_TRUNC | K_O_RDWR, 0666);
    U32 secondFd = process->open(secondPath, K_O_CREAT | K_O_TRUNC | K_O_RDWR, 0666);
    std::shared_ptr<KFile> firstFile;
    std::shared_ptr<KFile> secondFile;
    std::shared_ptr<MappedFileCache> sourceCache;
    std::shared_ptr<MappedFileCache> replacementCache;
    MappedFilePtr originalMapping;
    DecodedOp* originalOp = nullptr;
    DecodedOp* originalPeerOp = nullptr;
    DecodedOp* originalBlockStart = nullptr;
    DecodedOp* originalPeerBlockStart = nullptr;
    void* originalJitCode = nullptr;
    void* originalPeerJitCode = nullptr;
    U32 originalOpFlags = 0;
    U32 originalPeerOpFlags = 0;
    U32 originalLeftPageFlags = 0;
    U32 originalMiddlePageFlags = 0;
    U32 originalRightPageFlags = 0;
    U32 originalMappedRecordCount = 0;
    U32 originalSourceLeaseCount = 0;
    U32 originalReplacementLeaseCount = 0;
    U32 originalSourcePieceCount = 0;
    U32 originalMappingAddress = 0;
    U64 originalMappingLength = 0;
    U64 originalMappingOffset = 0;
    U32 originalMappingKey = 0;
#ifdef BOXEDWINE_WASM_JIT
#ifdef BOXEDWINE_MULTI_THREADED
    WasmJitMtBrokerModuleRef originalBrokerRef;
    WasmJitMtBrokerModuleRef originalPeerBrokerRef;
    WasmJitMtRetirementStateSnapshot originalRetirementState;
    bool originalSlotMetadata = false;
    bool originalPeerSlotMetadata = false;
    U32 originalGroupOwner = 0;
    U32 originalRuntimeGroupCount = 0;
    U32 originalRuntimeModuleCount = 0;
#else
    U32 originalPendingCount = 0;
    U32 originalSealedCount = 0;
    U32 originalGroupOwner = 0;
    U32 originalPeerGroupOwner = 0;
    U32 originalRuntimeGroupCount = 0;
    U32 originalRuntimeModuleCount = 0;
    U32 originalRuntimeGroupReleaseCount = 0;
    bool originalCompilationPaused = false;
#endif
#endif

    if ((S32)firstFd < 0 || (S32)secondFd < 0) {
        testFail("executable fixed preparation open failed first=%d second=%d",
            (S32)firstFd, (S32)secondFd);
        goto done;
    }
    expectZero("executable fixed preparation first truncate",
        process->ftruncate64(firstFd, MAPPING_LENGTH));
    expectZero("executable fixed preparation second truncate",
        process->ftruncate64(secondFd, K_PAGE_SIZE));
    firstFile = std::dynamic_pointer_cast<KFile>(
        process->getFileDescriptor(firstFd)->kobject);
    secondFile = std::dynamic_pointer_cast<KFile>(
        process->getFileDescriptor(secondFd)->kobject);
    replacementCache = secondFile->getOrCreateMappedFileCacheForTest(
        secondPath, false);
    if (!replacementCache) {
        testFail("executable fixed preparation replacement cache setup failed");
        goto done;
    }
    {
        const U8 firstCode[] = {
            0xb8, (U8)CODE_VALUE, (U8)(CODE_VALUE >> 8),
            (U8)(CODE_VALUE >> 16), (U8)(CODE_VALUE >> 24), 0xc3
        };
        const U8 firstPeerCode[] = {
            0xb9, (U8)PEER_CODE_VALUE, (U8)(PEER_CODE_VALUE >> 8),
            (U8)(PEER_CODE_VALUE >> 16), (U8)(PEER_CODE_VALUE >> 24), 0xc3
        };
        const U8 secondCode[] = {0x31, 0xc0, 0xc3};
        expectU32("executable fixed preparation first code write",
            firstFile->pwriteNative((U8*)firstCode, CODE_FILE_OFFSET,
                sizeof(firstCode)), sizeof(firstCode));
        expectU32("executable fixed preparation peer code write",
            firstFile->pwriteNative((U8*)firstPeerCode, PEER_FILE_OFFSET,
                sizeof(firstPeerCode)), sizeof(firstPeerCode));
        expectU32("executable fixed preparation second code write",
            secondFile->pwriteNative((U8*)secondCode, 0, sizeof(secondCode)), sizeof(secondCode));
    }

    KThread::setCurrentThread(thread);
    process->memory->clearOpCache();
    if (process->memory->mmap(thread, MAP_ADDRESS, MAPPING_LENGTH,
        K_PROT_READ | K_PROT_EXEC, K_MAP_PRIVATE | K_MAP_FIXED, firstFd, 0) != MAP_ADDRESS) {
        testFail("executable fixed preparation first map failed");
        goto done;
    }
    cpu->big = 1;
    cpu->seg[CS].address = 0;
    cpu->eip.u32 = CODE_ADDRESS;
    cpu->nextOp = nullptr;
#ifdef BOXEDWINE_WASM_JIT
    {
        WasmJitBatchLimits limits;
        limits.maxBlocks = 2;
        limits.maxBatchBytes = 1024 * 1024;
        limits.urgentPendingHits = 8;
        limits.maxProcessOpenBytes = 4 * 1024 * 1024;
#ifdef BOXEDWINE_MULTI_THREADED
        wasmJitTestResetMtRuntimeBatching();
        wasmJitTestSetMtBatchLimits(limits);
        wasmJitTestEnableMtRuntimeBatching(true);
#else
        wasmJitTestResetRuntimeBatching();
        wasmJitTestSetBatchLimits(limits);
        wasmJitTestEnableRuntimeBatching(true);
#endif
    }
#endif
    originalOp = cpu->getOp(CODE_ADDRESS, 0);
    startNewJIT(cpu, CODE_ADDRESS, originalOp);
#ifdef BOXEDWINE_WASM_JIT
    if (!(originalOp->flags2 & OP_FLAG2_WASM_JIT_PENDING) ||
            originalOp->pfnJitCode) {
        testFail("executable fixed preparation first grouped block did not remain pending");
        goto done;
    }
    originalPeerOp = cpu->getOp(PEER_ADDRESS, 0);
    startNewJIT(cpu, PEER_ADDRESS, originalPeerOp);
#else
    originalPeerOp = cpu->getOp(PEER_ADDRESS, 0);
    startNewJIT(cpu, PEER_ADDRESS, originalPeerOp);
#endif
    if (!originalOp || !originalPeerOp ||
        !originalOp->blockStart || !originalPeerOp->blockStart ||
        !originalOp->pfnJitCode || !originalPeerOp->pfnJitCode ||
        !(originalOp->flags & OP_FLAG_JIT)) {
        testFail("executable fixed preparation did not compile both original file blocks");
        goto done;
    }
    originalMapping =
        process->getMappedFileForRange(MAP_ADDRESS, MAPPING_LENGTH);
    if (!originalMapping) {
        testFail("executable fixed preparation original mapping record missing");
        goto done;
    }
    if (originalMapping->address != MAP_ADDRESS ||
            originalMapping->len != MAPPING_LENGTH ||
            originalMapping->offset != 0) {
        testFail("executable fixed preparation setup was not one three-page mapping");
    }
    originalBlockStart = originalOp->blockStart;
    originalPeerBlockStart = originalPeerOp->blockStart;
    originalJitCode = originalOp->pfnJitCode;
    originalPeerJitCode = originalPeerOp->pfnJitCode;
    originalOpFlags = originalOp->flags;
    originalPeerOpFlags = originalPeerOp->flags;
    originalLeftPageFlags =
        process->memory->getPageFlags(MAP_ADDRESS >> K_PAGE_SHIFT);
    originalMiddlePageFlags =
        process->memory->getPageFlags(REPLACEMENT_ADDRESS >> K_PAGE_SHIFT);
    originalRightPageFlags = process->memory->getPageFlags(
        (MAP_ADDRESS + K_PAGE_SIZE * 2) >> K_PAGE_SHIFT);
    originalMappedRecordCount = process->getMappedFileCountForTest();
    sourceCache = originalMapping->systemCacheEntry;
    originalSourceLeaseCount = sourceCache->getMappingLeaseCountForTest();
    originalReplacementLeaseCount =
        replacementCache->getMappingLeaseCountForTest();
    originalSourcePieceCount = originalMapping->lease->getPieceCountForTest();
    originalMappingAddress = originalMapping->address;
    originalMappingLength = originalMapping->len;
    originalMappingOffset = originalMapping->offset;
    originalMappingKey = originalMapping->key;
    expectU32("executable fixed preparation setup mapped record count",
        originalMappedRecordCount, 1);
    expectU32("executable fixed preparation setup source lease pieces",
        originalSourcePieceCount, 1);
#ifdef BOXEDWINE_WASM_JIT
#ifdef BOXEDWINE_MULTI_THREADED
    originalBrokerRef = wasmJitTestGetMtBrokerSlotRef(
        (int)(uintptr_t)originalJitCode);
    originalPeerBrokerRef = wasmJitTestGetMtBrokerSlotRef(
        (int)(uintptr_t)originalPeerJitCode);
    originalSlotMetadata = wasmJitTestHasMtSlotMetadata(
        (int)(uintptr_t)originalJitCode);
    originalPeerSlotMetadata = wasmJitTestHasMtSlotMetadata(
        (int)(uintptr_t)originalPeerJitCode);
    originalGroupOwner = wasmJitTestGetMtGroupIndexForSlot(
        (int)(uintptr_t)originalJitCode);
    originalRetirementState = wasmJitTestGetMtRetirementState(cpu);
    originalRuntimeGroupCount = wasmJitTestMtRuntimeGroupCount();
    originalRuntimeModuleCount = wasmJitTestMtRuntimeModuleCount();
    if (!originalGroupOwner ||
            originalGroupOwner != wasmJitTestGetMtGroupIndexForSlot(
                (int)(uintptr_t)originalPeerJitCode) ||
            !originalBrokerRef.moduleId ||
            originalBrokerRef.moduleId != originalPeerBrokerRef.moduleId ||
            originalBrokerRef.memoryId != originalPeerBrokerRef.memoryId ||
            originalBrokerRef.memoryIncarnation !=
                originalPeerBrokerRef.memoryIncarnation) {
        testFail("MT WASM fixed preparation setup did not publish one two-block owner");
        goto done;
    }
#else
    originalPendingCount = wasmJitTestPendingCount();
    originalSealedCount = wasmJitTestSealedCount();
    originalGroupOwner = wasmJitTestRuntimeGroupIdForSlot(
        (int)(uintptr_t)originalJitCode);
    originalPeerGroupOwner = wasmJitTestRuntimeGroupIdForSlot(
        (int)(uintptr_t)originalPeerJitCode);
    originalRuntimeGroupCount = wasmJitTestRuntimeGroupCount();
    originalRuntimeModuleCount = wasmJitTestRuntimeModuleCount();
    originalRuntimeGroupReleaseCount = wasmJitTestRuntimeGroupReleaseCount();
    originalCompilationPaused = wasmJitCompilationPaused();
    if (!originalGroupOwner || originalGroupOwner != originalPeerGroupOwner) {
        testFail("WASM fixed preparation setup did not publish one two-block group");
        goto done;
    }
#endif
#endif

#ifdef BOXEDWINE_WASM_JIT
    wasmJitTestSetFailInvalidationPreparation(true);
#else
    process->memory->setTestFailCodeInvalidationPreparation(true);
#endif
    {
        U32 result;
        try {
            result = process->memory->mmap(thread, REPLACEMENT_ADDRESS,
                K_PAGE_SIZE,
                K_PROT_READ | K_PROT_EXEC, K_MAP_PRIVATE | K_MAP_FIXED, secondFd, 0);
        } catch (const std::bad_alloc&) {
            result = -K_ENOMEM;
        }
        expectU32("executable fixed preparation failure result", result, -K_ENOMEM);
    }
#ifdef BOXEDWINE_WASM_JIT
    wasmJitTestSetFailInvalidationPreparation(false);
#else
    process->memory->setTestFailCodeInvalidationPreparation(false);
#endif

    {
        MappedFilePtr afterFailure =
            process->getMappedFileForRange(MAP_ADDRESS, MAPPING_LENGTH);
        if (!afterFailure ||
            afterFailure->address != originalMappingAddress ||
            afterFailure->len != originalMappingLength ||
            afterFailure->offset != originalMappingOffset ||
            afterFailure->key != originalMappingKey ||
            afterFailure->lease != originalMapping->lease ||
            afterFailure->systemCacheEntry != originalMapping->systemCacheEntry) {
            testFail("executable fixed preparation failure changed the mapping record");
        }
        MappedFilePtr left =
            process->getMappedFileForRange(MAP_ADDRESS, K_PAGE_SIZE);
        MappedFilePtr middle = process->getMappedFileForRange(
            REPLACEMENT_ADDRESS, K_PAGE_SIZE);
        MappedFilePtr right = process->getMappedFileForRange(
            MAP_ADDRESS + K_PAGE_SIZE * 2, K_PAGE_SIZE);
        auto isOriginalWholeMapping = [&](const MappedFilePtr& mapping) {
            return mapping &&
                mapping->address == originalMappingAddress &&
                mapping->len == originalMappingLength &&
                mapping->offset == originalMappingOffset &&
                mapping->key == originalMappingKey &&
                mapping->lease == originalMapping->lease &&
                mapping->systemCacheEntry ==
                    originalMapping->systemCacheEntry;
        };
        if (!isOriginalWholeMapping(left) ||
                !isOriginalWholeMapping(middle) ||
                !isOriginalWholeMapping(right)) {
            testFail("executable fixed preparation failure split the source mapping");
        }
    }
    expectU32("executable fixed preparation failure preserved left page flags",
        process->memory->getPageFlags(MAP_ADDRESS >> K_PAGE_SHIFT),
        originalLeftPageFlags);
    expectU32("executable fixed preparation failure preserved middle page flags",
        process->memory->getPageFlags(REPLACEMENT_ADDRESS >> K_PAGE_SHIFT),
        originalMiddlePageFlags);
    expectU32("executable fixed preparation failure preserved right page flags",
        process->memory->getPageFlags(
            (MAP_ADDRESS + K_PAGE_SIZE * 2) >> K_PAGE_SHIFT),
        originalRightPageFlags);
    expectU32("executable fixed preparation failure preserved mapped record count",
        process->getMappedFileCountForTest(), originalMappedRecordCount);
    expectU32("executable fixed preparation failure preserved source cache lease",
        sourceCache->getMappingLeaseCountForTest(), originalSourceLeaseCount);
    expectU32("executable fixed preparation failure retired replacement cache lease",
        replacementCache->getMappingLeaseCountForTest(),
        originalReplacementLeaseCount);
    expectU32("executable fixed preparation failure preserved source lease pieces",
        originalMapping->lease->getPieceCountForTest(),
        originalSourcePieceCount);
    if (process->memory->getDecodedOp(CODE_ADDRESS) != originalOp ||
        originalOp->blockStart != originalBlockStart ||
        originalOp->pfnJitCode != originalJitCode ||
        originalOp->flags != originalOpFlags ||
        process->memory->getDecodedOp(PEER_ADDRESS) != originalPeerOp ||
        originalPeerOp->blockStart != originalPeerBlockStart ||
        originalPeerOp->pfnJitCode != originalPeerJitCode ||
        originalPeerOp->flags != originalPeerOpFlags) {
        testFail("executable fixed preparation failure changed decoded/JIT state");
    }
#ifdef BOXEDWINE_WASM_JIT
#ifdef BOXEDWINE_MULTI_THREADED
    {
        WasmJitMtBrokerModuleRef afterBrokerRef =
            wasmJitTestGetMtBrokerSlotRef((int)(uintptr_t)originalJitCode);
        WasmJitMtBrokerModuleRef afterPeerBrokerRef =
            wasmJitTestGetMtBrokerSlotRef((int)(uintptr_t)originalPeerJitCode);
        WasmJitMtRetirementStateSnapshot afterRetirementState =
            wasmJitTestGetMtRetirementState(cpu);
        if (afterBrokerRef.moduleId != originalBrokerRef.moduleId ||
                afterBrokerRef.memoryId != originalBrokerRef.memoryId ||
                afterBrokerRef.memoryIncarnation !=
                    originalBrokerRef.memoryIncarnation ||
                afterPeerBrokerRef.moduleId != originalPeerBrokerRef.moduleId ||
                afterPeerBrokerRef.memoryId != originalPeerBrokerRef.memoryId ||
                afterPeerBrokerRef.memoryIncarnation !=
                    originalPeerBrokerRef.memoryIncarnation ||
                wasmJitTestHasMtSlotMetadata(
                    (int)(uintptr_t)originalJitCode) != originalSlotMetadata ||
                wasmJitTestHasMtSlotMetadata(
                    (int)(uintptr_t)originalPeerJitCode) !=
                    originalPeerSlotMetadata ||
                wasmJitTestGetMtGroupIndexForSlot(
                    (int)(uintptr_t)originalJitCode) != originalGroupOwner ||
                wasmJitTestGetMtGroupIndexForSlot(
                    (int)(uintptr_t)originalPeerJitCode) !=
                    originalGroupOwner ||
                afterRetirementState != originalRetirementState ||
                wasmJitTestMtRuntimeGroupCount() !=
                    originalRuntimeGroupCount ||
                wasmJitTestMtRuntimeModuleCount() !=
                    originalRuntimeModuleCount) {
            testFail("MT WASM fixed preparation failure changed broker or slot ownership");
        }
    }
#else
    if (wasmJitTestPendingCount() != originalPendingCount ||
            wasmJitTestSealedCount() != originalSealedCount ||
            wasmJitTestRuntimeGroupIdForSlot(
                (int)(uintptr_t)originalJitCode) != originalGroupOwner ||
            wasmJitTestRuntimeGroupIdForSlot(
                (int)(uintptr_t)originalPeerJitCode) != originalPeerGroupOwner ||
            wasmJitTestRuntimeGroupCount() != originalRuntimeGroupCount ||
            wasmJitTestRuntimeModuleCount() != originalRuntimeModuleCount ||
            wasmJitTestRuntimeGroupReleaseCount() !=
                originalRuntimeGroupReleaseCount ||
            wasmJitCompilationPaused() != originalCompilationPaused) {
        testFail("WASM fixed preparation failure changed pending or table ownership");
    }
#endif
#endif

done:
    if (process) {
#ifdef BOXEDWINE_WASM_JIT
        wasmJitTestSetFailInvalidationPreparation(false);
#else
        process->memory->setTestFailCodeInvalidationPreparation(false);
#endif
        KThread::setCurrentThread(thread);
        process->memory->mmap(thread, MAP_ADDRESS, MAPPING_LENGTH,
            K_PROT_READ | K_PROT_WRITE | K_PROT_EXEC,
            K_MAP_PRIVATE | K_MAP_ANONYMOUS | K_MAP_FIXED, -1, 0);
        process->memory->clearOpCache();
#ifdef BOXEDWINE_WASM_JIT
#ifdef BOXEDWINE_MULTI_THREADED
        wasmJitTestReapMtRetiredSlots(process->memory);
        wasmJitTestResetMtRuntimeBatching();
#else
        wasmJitTestResetRuntimeBatching();
#endif
#endif
        if ((S32)secondFd >= 0) process->close(secondFd);
        if ((S32)firstFd >= 0) process->close(firstFd);
        KSystem::eraseProcess(process->id);
        process = nullptr;
    }
    KThread::setCurrentThread(context.thread);
    cleanupRoot(root);
#endif
}

void testNativeHeapSmallFreeQueueFailureIsRetryable() {
    BNativeHeap heap;
    heap.delayedFree = 60000;
    constexpr U32 ALLOCATION_SIZE = 32760;
    void* first = heap.alloc(ALLOCATION_SIZE);
    void* second = heap.alloc(ALLOCATION_SIZE);
    U32 originalHeader = *(((U32*)first) - 1);

    heap.setTestFailNextSmallFreeQueue(true);
    bool caught = false;
    try {
        heap.free(first);
    } catch (const std::bad_alloc&) {
        caught = true;
    }
    if (!caught) {
        testFail("native heap small free queue injection did not throw");
    }
    expectU32("native heap failed free preserves size-class header",
        *(((U32*)first) - 1), originalHeader);

    heap.free(first);
    heap.delayedFree = 0;
    void* reused = heap.alloc(ALLOCATION_SIZE);
    if (reused != first) {
        testFail("native heap retry did not make the freed block reusable");
    }
    heap.free(reused);
    heap.free(second);
}

void testCachedPositionedReadUsesCheckedOffsetAndSeek() {
    TestContext& context = testContext();
    constexpr U32 GUEST_BUFFER = TEST_HEAP_ADDRESS + 0x1800;
    auto node = std::make_shared<FsMemNode>(
        1, 1, B("positioned-read-checked-offset"));
    auto* open = new PositionedReadTestOpenNode(node);
    auto file = std::make_shared<KFile>(open);
    std::shared_ptr<MappedFileCache> cache =
        file->getOrCreateMappedFileCacheForTest(
            B("positioned-read-checked-offset"), true);
    RamPage lowPage = cache->getOrCreatePage(0, true);
    const U8 lowDirty[4] = {0x11, 0x22, 0x33, 0x44};
    memcpy(ramPageGet(lowPage), lowDirty, sizeof(lowDirty));
    ramPageRelease(lowPage);

    U8 buffer[4] = {};
    expectU32("cached high positioned read byte count",
        file->preadNative(buffer, PositionedReadTestOpenNode::HIGH_OFFSET,
            sizeof(buffer)), sizeof(buffer));
    if (memcmp(buffer, open->highBytes, sizeof(buffer))) {
        testFail("cached high positioned read aliased a low dirty cache page");
    }

    constexpr S64 PRIOR_POSITION = 123;
    open->position = PRIOR_POSITION;
    open->failNextSeek = true;
    U32 readsBefore = open->readCalls;
    expectU32("cached positioned read seek failure",
        file->preadNative(buffer, PositionedReadTestOpenNode::HIGH_OFFSET,
            sizeof(buffer)), -K_EINVAL);
    expectU32("cached positioned read seek failure performs no read",
        open->readCalls, readsBefore);
    expectU64("cached positioned read seek failure preserves cursor",
        (U64)open->position, PRIOR_POSITION);

    open->position = PRIOR_POSITION;
    open->failNextSeek = true;
    readsBefore = open->readCalls;
    expectU32("uncached positioned read seek failure",
        file->preadNativeUncached(
            buffer, PositionedReadTestOpenNode::HIGH_OFFSET, sizeof(buffer)),
        -K_EINVAL);
    expectU32("uncached positioned read seek failure performs no read",
        open->readCalls, readsBefore);
    expectU64("uncached positioned read seek failure preserves cursor",
        (U64)open->position, PRIOR_POSITION);

    auto expectSavedPositionFailure = [&](const char* label,
        const std::function<U32()>& read) {
        open->position = PRIOR_POSITION;
        open->failNextGetFilePointer = true;
        U32 readsBeforeFailure = open->readCalls;
        U32 seeksBeforeFailure = open->seekCalls;
        expectU32(label, read(), -K_EIO);
        expectU32("saved-position failure performs no read",
            open->readCalls, readsBeforeFailure);
        expectU32("saved-position failure performs no seek",
            open->seekCalls, seeksBeforeFailure);
        expectU64("saved-position failure preserves cursor",
            (U64)open->position, PRIOR_POSITION);
    };
    expectSavedPositionFailure("guest positioned read saved-position failure",
        [&]() {
            return file->pread(context.thread, GUEST_BUFFER,
                PositionedReadTestOpenNode::HIGH_OFFSET, sizeof(buffer));
        });
    expectSavedPositionFailure("cached positioned read saved-position failure",
        [&]() {
            return file->preadNative(buffer,
                PositionedReadTestOpenNode::HIGH_OFFSET, sizeof(buffer));
        });
    expectSavedPositionFailure("uncached positioned read saved-position failure",
        [&]() {
            return file->preadNativeUncached(buffer,
                PositionedReadTestOpenNode::HIGH_OFFSET, sizeof(buffer));
        });

    open->position = PRIOR_POSITION;
    open->failSeekPosition = PRIOR_POSITION;
    U32 readsBeforeRestoreFailure = open->readCalls;
    expectU32("guest positioned read restore failure",
        file->pread(context.thread, GUEST_BUFFER, 0, sizeof(buffer)), -K_EIO);
    expectU32("guest positioned read restore failure performs read",
        open->readCalls, readsBeforeRestoreFailure + 1);
    expectU64("guest positioned read restore failure exposes changed cursor",
        (U64)open->position, sizeof(buffer));
    U8 guestBuffer[sizeof(buffer)] = {};
    context.memory->memcpy(guestBuffer, GUEST_BUFFER, sizeof(guestBuffer));
    if (memcmp(guestBuffer, lowDirty, sizeof(guestBuffer))) {
        testFail("guest positioned read restore failure did not overlay cached bytes");
    }

    open->position = PRIOR_POSITION;
    open->failSeekPosition = PRIOR_POSITION;
    readsBeforeRestoreFailure = open->readCalls;
    memset(buffer, 0, sizeof(buffer));
    expectU32("cached positioned read restore failure",
        file->preadNative(buffer, 0, sizeof(buffer)), -K_EIO);
    expectU32("cached positioned read restore failure performs read",
        open->readCalls, readsBeforeRestoreFailure + 1);
    expectU64("cached positioned read restore failure exposes changed cursor",
        (U64)open->position, sizeof(buffer));
    if (memcmp(buffer, lowDirty, sizeof(buffer))) {
        testFail("cached positioned read restore failure did not overlay cached bytes");
    }

    open->position = PRIOR_POSITION;
    open->failSeekPosition = PRIOR_POSITION;
    readsBeforeRestoreFailure = open->readCalls;
    memset(buffer, 0, sizeof(buffer));
    expectU32("uncached positioned read restore failure",
        file->preadNativeUncached(buffer,
            PositionedReadTestOpenNode::HIGH_OFFSET, sizeof(buffer)), -K_EIO);
    expectU32("uncached positioned read restore failure performs read",
        open->readCalls, readsBeforeRestoreFailure + 1);
    expectU64("uncached positioned read restore failure exposes changed cursor",
        (U64)open->position,
        PositionedReadTestOpenNode::HIGH_OFFSET + sizeof(buffer));
    if (memcmp(buffer, open->highBytes, sizeof(buffer))) {
        testFail("uncached positioned read restore failure lost read bytes");
    }
}

void testFtruncateBackendLengthBoundaries() {
#ifdef __EMSCRIPTEN__
    // MEMFS materializes file contents and cannot represent a sparse file
    // larger than its WebAssembly address space.
    constexpr U64 REGULAR_LENGTH = 0x2000;
#else
    constexpr U64 REGULAR_LENGTH = 0x100001000ULL;
#endif
    constexpr U64 MEM_INITIAL_LENGTH = 0x1000;
    constexpr U64 MEM_UNSUPPORTED_LENGTH = 0x100000000ULL;
    const BString path = B("/tmp/ftruncate-backend-boundaries");
    const BString root = B("tmp/test-ftruncate-backend-boundaries-root");

    cleanupRoot(root);
    initTestFileSystem(root);
    Fs::makeLocalDirs(B("/tmp"));

    std::shared_ptr<FsNode> regularNode = addRegularFile(path);
    FsOpenNode* regularOpen = regularNode ? regularNode->open(K_O_CREAT | K_O_RDWR) : nullptr;
    if (!regularOpen) {
        testFail("ftruncate boundary regular backend open failed");
    } else {
        constexpr S64 PRESERVED_OFFSET = 0x321;
        if (regularOpen->seek(PRESERVED_OFFSET) != PRESERVED_OFFSET) {
            testFail("regular backend could not establish file offset before truncate");
        }
        if (!regularOpen->setLength(REGULAR_LENGTH)) {
#ifdef __EMSCRIPTEN__
            testFail("regular backend rejected supported length");
#else
            testFail("regular backend rejected signed 64-bit sparse length");
#endif
        }
#ifdef __EMSCRIPTEN__
        expectU64("regular backend committed supported length",
            (U64)regularOpen->length(), REGULAR_LENGTH);
#else
        expectU64("regular backend committed signed 64-bit sparse length", (U64)regularOpen->length(), REGULAR_LENGTH);
#endif
        expectU64("regular backend ftruncate preserves file offset", (U64)regularOpen->getFilePointer(), PRESERVED_OFFSET);
        if (!regularOpen->setLength(0)) {
            testFail("regular backend boundary cleanup truncate failed");
        }
        delete regularOpen;
    }

    {
        std::shared_ptr<FsMemNode> memNode = std::make_shared<FsMemNode>(1, 1, B("ftruncate-boundary-memfd"));
        FsMemOpenNode memOpen(K_O_RDWR, memNode);
        memNode->openNode = &memOpen;
        if (!memOpen.setLength(MEM_INITIAL_LENGTH)) {
            testFail("memory backend rejected supported ftruncate length");
        }
        if (memOpen.setLength(MEM_UNSUPPORTED_LENGTH)) {
            testFail("memory backend accepted greater than U32 ftruncate length");
        }
        expectU64("memory backend rejection preserves committed length", (U64)memOpen.length(), MEM_INITIAL_LENGTH);
        memNode->openNode = nullptr;
    }
    cleanupRoot(root);
}

void testPrivateMappedWriteDoesNotChangePread() {
    TestContext& context = testContext();
    constexpr U32 MAP_ADDRESS = 0x02700000;
    constexpr U32 BUFFER_ADDRESS = 0x02800000;
    constexpr U32 FILE_OFFSET = 0x240;
    constexpr U32 PRIVATE_VALUE = 0x55667788;
    const BString path = B("/tmp/private-map-pread");
    const BString root = B("tmp/test-private-map-pread-root");

    cleanupRoot(root);
    initTestFileSystem(root);
    Fs::makeLocalDirs(B("/tmp"));
    Fs::makeLocalDirs(B("/proc"));
    setTestProcNode();

    KThread* thread = nullptr;
    KProcessPtr process = createProcessWithMemory(&thread);
    U32 fd = process->open(path, K_O_CREAT | K_O_TRUNC | K_O_RDWR, 0666);
    if ((S32)fd < 0) {
        testFail("private map pread open failed fd=%d", (S32)fd);
        goto done;
    }
    if (process->ftruncate64(fd, K_PAGE_SIZE)) {
        testFail("private map pread truncate failed");
        goto done;
    }

    KThread::setCurrentThread(thread);
    if (process->memory->mmap(thread, BUFFER_ADDRESS, K_PAGE_SIZE, K_PROT_READ | K_PROT_WRITE,
        K_MAP_PRIVATE | K_MAP_ANONYMOUS | K_MAP_FIXED, -1, 0) != BUFFER_ADDRESS) {
        testFail("private map pread buffer map failed");
        goto done;
    }
    if (process->memory->mmap(thread, MAP_ADDRESS, K_PAGE_SIZE, K_PROT_READ | K_PROT_WRITE,
        K_MAP_PRIVATE | K_MAP_FIXED, fd, 0) != MAP_ADDRESS) {
        testFail("private map pread file map failed");
        goto done;
    }

    process->memory->writed(MAP_ADDRESS + FILE_OFFSET, PRIVATE_VALUE);
    expectU32("private map pread count",
        process->pread64(thread, fd, BUFFER_ADDRESS, sizeof(U32), FILE_OFFSET), sizeof(U32));
    expectU32("private mapped write left pread unchanged", process->memory->readd(BUFFER_ADDRESS), 0);
    expectU32("private mapped write remained in private mapping", process->memory->readd(MAP_ADDRESS + FILE_OFFSET), PRIVATE_VALUE);

done:
    KThread::setCurrentThread(context.thread);
    if (process) {
        if ((S32)fd >= 0) process->close(fd);
        KSystem::eraseProcess(process->id);
        process = nullptr;
    }
    cleanupRoot(root);
}

void testMemfdPwriteUpdatesResidentSharedMapping() {
    TestContext& context = testContext();
    constexpr U32 MAP_ADDRESS = 0x02900000;
    constexpr U32 BUFFER_ADDRESS = 0x02a00000;
    constexpr U32 FILE_OFFSET = 0x240;
    constexpr U32 EXPECTED = 0x11223344;
    const BString root = B("tmp/test-mapped-memfd-root");

    cleanupRoot(root);
    initTestFileSystem(root);
    Fs::makeLocalDirs(B("/proc"));
    setTestProcNode();

    KThread* thread = nullptr;
    KProcessPtr process = createProcessWithMemory(&thread);
    U32 fd = process->memfd_create(B("mapped-memfd"), 0);
    if ((S32)fd < 0) {
        testFail("mapped memfd create failed fd=%d", (S32)fd);
        goto done;
    }
    if (process->ftruncate64(fd, K_PAGE_SIZE)) {
        testFail("mapped memfd truncate failed");
        goto done;
    }
    expectU32("mapped memfd rejects greater than U32 ftruncate length",
        process->ftruncate64(fd, 0x100000000ULL), -K_EFBIG);
    expectU32("mapped memfd rejects greater than signed 64-bit ftruncate length",
        process->ftruncate64(fd, (U64)std::numeric_limits<S64>::max() + 1), -K_EINVAL);
    expectU64("mapped memfd rejected truncates preserve length",
        (U64)std::dynamic_pointer_cast<KFile>(process->getFileDescriptor(fd)->kobject)->length(), K_PAGE_SIZE);

    KThread::setCurrentThread(thread);
    if (process->memory->mmap(thread, BUFFER_ADDRESS, K_PAGE_SIZE, K_PROT_READ | K_PROT_WRITE,
        K_MAP_PRIVATE | K_MAP_ANONYMOUS | K_MAP_FIXED, -1, 0) != BUFFER_ADDRESS) {
        testFail("mapped memfd buffer map failed");
        goto done;
    }
    if (process->memory->mmap(thread, MAP_ADDRESS, K_PAGE_SIZE, K_PROT_READ | K_PROT_WRITE,
        K_MAP_SHARED | K_MAP_FIXED, fd, 0) != MAP_ADDRESS) {
        testFail("mapped memfd file map failed");
        goto done;
    }

    process->memory->readd(MAP_ADDRESS + FILE_OFFSET);
    process->memory->writed(BUFFER_ADDRESS, EXPECTED);
    expectU32("mapped memfd pwrite count",
        process->pwrite64(thread, fd, BUFFER_ADDRESS, sizeof(U32), FILE_OFFSET), sizeof(U32));
    expectU32("mapped memfd pwrite updated resident mapping", process->memory->readd(MAP_ADDRESS + FILE_OFFSET), EXPECTED);

done:
    KThread::setCurrentThread(context.thread);
    if (process) {
        if ((S32)fd >= 0) process->close(fd);
        KSystem::eraseProcess(process->id);
        process = nullptr;
    }
    cleanupRoot(root);
}

void testMemfdPwriteRejectsUnrepresentableOffset() {
    TestContext& context = testContext();
    constexpr U32 BUFFER_ADDRESS = 0x02b00000;
    constexpr S64 PRESERVED_OFFSET = 17;
    const BString root = B("tmp/test-memfd-pwrite-boundary-root");

    cleanupRoot(root);
    initTestFileSystem(root);
    Fs::makeLocalDirs(B("/proc"));
    setTestProcNode();

    KThread* thread = nullptr;
    KProcessPtr process = createProcessWithMemory(&thread);
    U32 fd = process->memfd_create(B("memfd-pwrite-boundary"), 0);
    std::shared_ptr<KFile> file;

    if ((S32)fd < 0) {
        testFail("memfd pwrite boundary create failed fd=%d", (S32)fd);
        goto done;
    }
    file =
        std::dynamic_pointer_cast<KFile>(process->getFileDescriptor(fd)->kobject);
    if (!file) {
        testFail("memfd pwrite boundary file lookup failed");
        goto done;
    }

    KThread::setCurrentThread(thread);
    if (process->memory->mmap(thread, BUFFER_ADDRESS, K_PAGE_SIZE,
        K_PROT_READ | K_PROT_WRITE,
        K_MAP_PRIVATE | K_MAP_ANONYMOUS | K_MAP_FIXED, -1, 0) !=
            BUFFER_ADDRESS) {
        testFail("memfd pwrite boundary buffer map failed");
        goto done;
    }
    process->memory->writeb(BUFFER_ADDRESS, 0x5a);
    expectU64("memfd pwrite boundary set preserved offset",
        (U64)file->seek(PRESERVED_OFFSET), PRESERVED_OFFSET);
    {
        U32 result = (U32)-K_EIO;
        try {
            result = process->pwrite64(thread, fd, BUFFER_ADDRESS, 1,
                (U64)std::numeric_limits<S64>::max());
        } catch (const std::exception& error) {
            testFail("memfd pwrite boundary escaped host exception: %s",
                error.what());
        } catch (...) {
            testFail("memfd pwrite boundary escaped unknown host exception");
        }
        expectU32("memfd pwrite boundary returns EFBIG",
            result, -K_EFBIG);
    }
    expectU64("memfd pwrite boundary preserves length",
        (U64)file->length(), 0);
    expectU64("memfd pwrite boundary restores file offset",
        (U64)file->openFile->getFilePointer(), PRESERVED_OFFSET);

done:
    KThread::setCurrentThread(context.thread);
    if (process) {
        if ((S32)fd >= 0) process->close(fd);
        KSystem::eraseProcess(process->id);
        process = nullptr;
    }
    cleanupRoot(root);
}

void testProcessVmReadvUsesRemoteFileMappingContext() {
    TestContext& context = testContext();
    constexpr U32 TARGET_ADDRESS = 0x03000000;
    constexpr U32 CALLER_IOV_PAGE = 0x03100000;
    constexpr U32 LOCAL_IOV = CALLER_IOV_PAGE;
    constexpr U32 REMOTE_IOV = CALLER_IOV_PAGE + 0x20;
    constexpr U32 LOCAL_BUFFER = CALLER_IOV_PAGE + 0x40;
    constexpr U32 K_SYSCALL_PROCESS_VM_READV = 347;
    constexpr U32 EXPECTED = 0x12345678;
    const U8 fileBytes[] = { 0x78, 0x56, 0x34, 0x12 };
    const BString path = B("/tmp/process-vm-map");
    const BString root = B("tmp/test-process-vm-map-root");

    cleanupRoot(root);
    initTestFileSystem(root);
    Fs::makeLocalDirs(B("/tmp"));
    Fs::makeLocalDirs(B("/proc"));
    setTestProcNode();
    KSystem::eraseFileCache(path);

    std::shared_ptr<FsNode> node = addRegularFile(path);
    if (!node) {
        testFail("process-vm map source node was not created");
        cleanupRoot(root);
        return;
    }
    FsOpenNode* openNode = node->open(K_O_CREAT | K_O_RDWR);
    if (!openNode) {
        testFail("process-vm map source file could not be opened");
        cleanupRoot(root);
        return;
    }
    expectU32("write process-vm source", openNode->writeNative((U8*)fileBytes, sizeof(fileBytes)), sizeof(fileBytes));
    delete openNode;

    KThread* targetThread = nullptr;
    KThread* callerThread = nullptr;
    KProcessPtr target = createProcessWithMemory(&targetThread);
    KProcessPtr caller = createProcessWithMemory(&callerThread);
    U32 targetFd = target->open(path, K_O_RDWR, 0666);
    if ((S32)targetFd < 0) {
        testFail("process-vm map open failed target=%d", (S32)targetFd);
        goto done;
    }

    KThread::setCurrentThread(targetThread);
    if (target->memory->mmap(targetThread, TARGET_ADDRESS, K_PAGE_SIZE, K_PROT_READ | K_PROT_WRITE,
        K_MAP_SHARED | K_MAP_FIXED, targetFd, 0) != TARGET_ADDRESS) {
        testFail("process-vm target map failed");
        goto done;
    }

    KThread::setCurrentThread(callerThread);
    if (caller->memory->mmap(callerThread, CALLER_IOV_PAGE, K_PAGE_SIZE, K_PROT_READ | K_PROT_WRITE,
        K_MAP_PRIVATE | K_MAP_ANONYMOUS | K_MAP_FIXED, -1, 0) != CALLER_IOV_PAGE) {
        testFail("process-vm caller iov map failed");
        goto done;
    }

    caller->memory->writed(LOCAL_IOV, LOCAL_BUFFER);
    caller->memory->writed(LOCAL_IOV + 4, sizeof(fileBytes));
    caller->memory->writed(REMOTE_IOV, TARGET_ADDRESS);
    caller->memory->writed(REMOTE_IOV + 4, sizeof(fileBytes));
    callerThread->cpu->reg[0].u32 = K_SYSCALL_PROCESS_VM_READV;
    callerThread->cpu->reg[3].u32 = target->id;
    callerThread->cpu->reg[1].u32 = LOCAL_IOV;
    callerThread->cpu->reg[2].u32 = 1;
    callerThread->cpu->reg[6].u32 = REMOTE_IOV;
    callerThread->cpu->reg[7].u32 = 1;
    callerThread->cpu->reg[5].u32 = 0;
    callerThread->cpu->eip.u32 = 0;
    ksyscall(callerThread->cpu, 2);

    expectU32("process-vm readv copied lazy file mapping", callerThread->cpu->reg[0].u32, sizeof(fileBytes));
    expectU32("process-vm readv copied value", caller->memory->readd(LOCAL_BUFFER), EXPECTED);

done:
    KThread::setCurrentThread(context.thread);
    if (target) {
        if ((S32)targetFd >= 0) target->close(targetFd);
        KSystem::eraseProcess(target->id);
        target = nullptr;
    }
    if (caller) {
        KSystem::eraseProcess(caller->id);
        caller = nullptr;
    }
    cleanupRoot(root);
}

void testReadDirectoryReturnsIsDir() {
    TestContext& context = testContext();

    BString root = B("tmp/test-read-directory-root");
    cleanupRoot(root);
    initTestFileSystem(root);
    Fs::makeLocalDirs(B("/tmp"));

    std::shared_ptr<FsNode> node = Fs::getNodeFromLocalPath(B(""), B("/tmp"), false);
    if (!node) {
        testFail("directory node was not found");
        cleanupRoot(root);
        return;
    }
    FsOpenNode* openNode = node->open(K_O_RDONLY);
    if (!openNode) {
        testFail("directory could not be opened");
        cleanupRoot(root);
        return;
    }
    expectU32("read directory", openNode->read(context.thread, BUFFER, 8), (U32)-K_EISDIR);
    delete openNode;

    cleanupRoot(root);
}

void testUtimensatPreservesAccessTimeInStat() {
    TestContext& context = testContext();
    KProcessPtr process = context.process;
    KMemory* memory = context.memory;

    BString root = B("tmp/test-file-times-root");
    cleanupRoot(root);
    initTestFileSystem(root);
    Fs::makeLocalDirs(B("/tmp"));

    std::shared_ptr<FsNode> node = addRegularFile(B("/tmp/time-file"));
    if (!node) {
        testFail("time file node was not created");
        cleanupRoot(root);
        return;
    }
    FsOpenNode* openNode = node->open(K_O_CREAT | K_O_RDWR);
    if (!openNode) {
        testFail("time file could not be opened");
        cleanupRoot(root);
        return;
    }
    delete openNode;

    memory->writed(TIMES, 2791529009U);
    memory->writed(TIMES + 4, 225700700U);
    memory->writed(TIMES + 8, 0);
    memory->writed(TIMES + 12, UTIME_OMIT);

    expectZero("set access time", process->utimesat(-100, B("/tmp/time-file"), TIMES, 0, false));
    expectZero("stat time file", process->stat64(B("/tmp/time-file"), STAT_A));
    expectU32("stat access seconds", stat64AccessSeconds(memory, STAT_A), 2791529009U);
    expectU32("stat access nanos", stat64AccessNanos(memory, STAT_A), 225700700U);

    cleanupRoot(root);
}

void testFutimensPreservesAccessTimeInFstat() {
    TestContext& context = testContext();
    KMemory* memory = context.memory;

    BString root = B("tmp/test-file-futimens-root");
    cleanupRoot(root);
    initTestFileSystem(root);
    Fs::makeLocalDirs(B("/tmp"));
    Fs::makeLocalDirs(B("/proc"));
    setTestProcNode();

    KProcessPtr process = KProcess::create();
    process->memory = memory;
    U32 fd = process->open(B("/tmp/time-file"), K_O_CREAT | K_O_RDWR, 0666);
    if ((S32)fd < 0) {
        testFail("time file could not be opened, got %d (0x%X)", (S32)fd, fd);
        cleanupRoot(root);
        return;
    }

    memory->writeq(TIMES, 2791529009ULL);
    memory->writed(TIMES + 8, 225700700U);
    memory->writed(TIMES + 12, 0);
    memory->writeq(TIMES + 16, 0);
    memory->writed(TIMES + 24, UTIME_OMIT);
    memory->writed(TIMES + 28, 0);

    expectZero("set fd access time", process->utimesat(fd, B(""), TIMES, 0, true));
    expectZero("fstat time file", process->fstat64(fd, STAT_A));
    expectU32("fstat access seconds", stat64AccessSeconds(memory, STAT_A), 2791529009U);
    expectU32("fstat access nanos", stat64AccessNanos(memory, STAT_A), 225700700U);

    process->close(fd);
    KSystem::eraseProcess(process->id);
    process->memory = nullptr;
    process = nullptr;
    cleanupRoot(root);
}

void testFutimensTime64SignExtendedSecondsPreservesAccessTimeInFstat() {
    TestContext& context = testContext();
    KMemory* memory = context.memory;

    BString root = B("tmp/test-file-futimens-sign-extended-root");
    cleanupRoot(root);
    initTestFileSystem(root);
    Fs::makeLocalDirs(B("/tmp"));
    Fs::makeLocalDirs(B("/proc"));
    setTestProcNode();

    KProcessPtr process = KProcess::create();
    process->memory = memory;
    U32 fd = process->open(B("/tmp/time-file"), K_O_CREAT | K_O_RDWR, 0666);
    if ((S32)fd < 0) {
        testFail("time file could not be opened, got %d (0x%X)", (S32)fd, fd);
        cleanupRoot(root);
        return;
    }

    memory->writeq(TIMES, 0xffffffffa6635a31ULL);
    memory->writed(TIMES + 8, 225700700U);
    memory->writed(TIMES + 12, 0);
    memory->writeq(TIMES + 16, 0);
    memory->writed(TIMES + 24, UTIME_OMIT);
    memory->writed(TIMES + 28, 0);

    expectZero("set sign-extended fd access time", process->utimesat(fd, B(""), TIMES, 0, true));
    expectZero("fstat sign-extended time file", process->fstat64(fd, STAT_A));
    expectU32("fstat sign-extended access seconds", stat64AccessSeconds(memory, STAT_A), 2791529009U);
    expectU32("fstat sign-extended access nanos", stat64AccessNanos(memory, STAT_A), 225700700U);

    process->close(fd);
    KSystem::eraseProcess(process->id);
    process->memory = nullptr;
    process = nullptr;
    cleanupRoot(root);
}

void testDirectoryReparseSidecarReplacedAfterRemoveAndRecreate() {
    TestContext& context = testContext();
    KProcessPtr process = context.process;

    BString root = B("tmp/test-directory-reparse-sidecar-root");
    cleanupRoot(root);
    initTestFileSystem(root);
    Fs::makeLocalDirs(B("/tmp/link"));

    std::shared_ptr<FsNode> node = Fs::getNodeFromLocalPath(B(""), B("/tmp/link"), false);
    if (!node) {
        testFail("link directory node was not created");
        cleanupRoot(root);
        return;
    }

    const U8 staleReparseData[] = { 0x11, 0x22, 0x33, 0x44 };
    expectZero("set stale directory reparse xattr", Fs::setXAttr(node, B("user.WINEREPARSE"), staleReparseData, sizeof(staleReparseData)));
    expectZero("rename stale directory reparse marker", process->rename(B("/tmp/link"), B("/tmp/link?")));
    expectZero("remove stale directory reparse marker", process->rmdir(B("/tmp/link?")));

    Fs::makeLocalDirs(B("/tmp/link"));
    node = Fs::getNodeFromLocalPath(B(""), B("/tmp/link"), false);
    if (!node) {
        testFail("recreated link directory node was not found");
        cleanupRoot(root);
        return;
    }

    const U8 freshReparseData[] = { 0xaa, 0xbb, 0xcc };
    expectZero("set fresh directory reparse xattr", Fs::setXAttr(node, B("user.WINEREPARSE"), freshReparseData, sizeof(freshReparseData)));
    expectZero("rename fresh directory reparse marker", process->rename(B("/tmp/link"), B("/tmp/link?")));

    node = Fs::getNodeFromLocalPath(B(""), B("/tmp/link?"), false);
    if (!node) {
        testFail("fresh reparse marker node was not found");
        cleanupRoot(root);
        return;
    }

    std::vector<U8> value;
    expectZero("get fresh directory reparse xattr", Fs::getXAttr(node, B("user.WINEREPARSE"), value));
    expectBytes("fresh directory reparse xattr", value, freshReparseData, sizeof(freshReparseData));

    cleanupRoot(root);
}

void testDotDotAfterDotResolvesToParentDirectory() {
    BString root = B("tmp/test-dot-dot-after-dot-root");
    cleanupRoot(root);
    initTestFileSystem(root);
    Fs::makeLocalDirs(B("/tmp/parent/child"));

    std::shared_ptr<FsNode> parent = Fs::getNodeFromLocalPath(B(""), B("/tmp/parent"), false);
    std::shared_ptr<FsNode> child = Fs::getNodeFromLocalPath(B(""), B("/tmp/parent/child"), false);
    std::shared_ptr<FsNode> resolved = Fs::getNodeFromLocalPath(B("/tmp/parent/child"), B("./.."), false);

    if (!parent || !child || !resolved) {
        testFail("./.. test nodes were not created or resolved");
        cleanupRoot(root);
        return;
    }

    expectU32("./.. parent id", resolved->getId(), parent->getId());
    if (resolved->getId() == child->getId()) {
        testFail("./.. resolved to the child directory");
    }

    cleanupRoot(root);
}

void testUtf8NamesSurviveNativeFilesystemReload() {
    BString root = B("tmp/test-utf8-native-file-name-root");
    cleanupRoot(root);
    initTestFileSystem(root);
    Fs::makeLocalDirs(B("/tmp"));

    BString utf8Name = BString::copy("\xC3\xA9" "a.tmp");
    BString path = B("/tmp/") + utf8Name;
    std::shared_ptr<FsNode> node = addRegularFile(path);
    if (!node) {
        testFail("UTF-8 file node was not created");
        cleanupRoot(root);
        return;
    }
    FsOpenNode* openNode = node->open(K_O_CREAT | K_O_RDWR);
    if (!openNode) {
        testFail("UTF-8 file could not be opened");
        cleanupRoot(root);
        return;
    }
    delete openNode;

    Fs::shutDown();
    initTestFileSystem(root);
    if (!Fs::getNodeFromLocalPath(B(""), path, false)) {
        testFail("UTF-8 file was not found after native filesystem reload");
    }

    cleanupRoot(root);
}

void testTrailingDotNamesCanBeUnlinked() {
    BString root = B("tmp/test-trailing-dot-file-name-root");
    cleanupRoot(root);
    initTestFileSystem(root);
    Fs::makeLocalDirs(B("/tmp"));

    std::shared_ptr<FsNode> node = addRegularFile(B("/tmp/a.."));
    if (!node) {
        testFail("trailing-dot file node was not created");
        cleanupRoot(root);
        return;
    }
    FsOpenNode* openNode = node->open(K_O_CREAT | K_O_RDWR);
    if (!openNode) {
        testFail("trailing-dot file could not be opened");
        cleanupRoot(root);
        return;
    }
    delete openNode;

    if (!node->remove()) {
        testFail("trailing-dot file could not be removed");
    }
    Fs::shutDown();
    initTestFileSystem(root);
    if (Fs::getNodeFromLocalPath(B(""), B("/tmp/a.."), false)) {
        testFail("trailing-dot file was found after removal and reload");
    }

    cleanupRoot(root);
}

void testDirectorySeekCanStoreOpaquePosition() {
    BString root = B("tmp/test-directory-opaque-position-root");
    cleanupRoot(root);
    initTestFileSystem(root);
    Fs::makeLocalDirs(B("/tmp/dir"));

    std::shared_ptr<FsNode> node = Fs::getNodeFromLocalPath(B(""), B("/tmp/dir"), false);
    if (!node) {
        testFail("directory node was not found");
        cleanupRoot(root);
        return;
    }

    FsOpenNode* openNode = node->open(K_O_RDONLY);
    if (!openNode) {
        testFail("directory could not be opened");
        cleanupRoot(root);
        return;
    }

    expectU32("seek directory opaque position", (U32)openNode->seek(0xbeef), 0xbeef);
    expectU32("get directory opaque position", (U32)openNode->getFilePointer(), 0xbeef);
    delete openNode;

    cleanupRoot(root);
}

void testInotifyReportsChildDirectoryCreate() {
    TestContext& context = testContext();
    KMemory* memory = context.memory;

    BString root = B("tmp/test-inotify-directory-create-root");
    cleanupRoot(root);
    initTestFileSystem(root);
    Fs::makeLocalDirs(B("/tmp/watch"));
    Fs::makeLocalDirs(B("/proc"));
    setTestProcNode();

    KProcessPtr process = KProcess::create();
    process->memory = memory;
    KThread* thread = process->createThread();
    ChangeThread current(thread);

    U32 fd = KInotifyObject::create(thread, 0);
    if ((S32)fd < 0) {
        testFail("inotify fd was not created, got %d (0x%X)", (S32)fd, fd);
        cleanupRoot(root);
        return;
    }

    U32 wd = KInotifyObject::addWatch(thread, fd, B("/tmp/watch"), K_IN_CREATE);
    if ((S32)wd < 0) {
        testFail("inotify watch was not added, got %d (0x%X)", (S32)wd, wd);
        cleanupRoot(root);
        return;
    }

    expectZero("mkdir watched child", process->mkdir(B("/tmp/watch/child")));
    if (!process->getFileDescriptor(fd)->kobject->isReadReady()) {
        testFail("inotify fd was not readable after watched mkdir");
    }

    U32 read = process->read(thread, fd, BUFFER, 64);
    expectU32("read inotify event length", read, 24);
    expectU32("event watch descriptor", memory->readd(BUFFER), wd);
    expectU32("event mask", memory->readd(BUFFER + 4), K_IN_CREATE | K_IN_ISDIR);
    expectU32("event cookie", memory->readd(BUFFER + 8), 0);
    expectU32("event name length", memory->readd(BUFFER + 12), 8);
    if (strcmp(memory->readString(BUFFER + 16).c_str(), "child")) {
        testFail("event name expected child, got %s", memory->readString(BUFFER + 16).c_str());
    }

    process->close(fd);
    process->removeThread(thread);
    delete thread;
    KSystem::eraseProcess(process->id);
    process->memory = nullptr;
    cleanupRoot(root);
}

void testInotifyFollowsWatchedSymlinkTarget() {
    TestContext& context = testContext();
    KMemory* memory = context.memory;

    BString root = B("tmp/test-inotify-symlink-watch-root");
    cleanupRoot(root);
    initTestFileSystem(root);
    Fs::makeLocalDirs(B("/tmp/watch-target"));
    Fs::makeLocalDirs(B("/proc"));
    setTestProcNode();

    KProcessPtr process = KProcess::create();
    process->memory = memory;
    KThread* thread = process->createThread();
    ChangeThread current(thread);

    expectZero("symlink watched directory", process->symlink(B("/tmp/watch-target"), B("/tmp/watch-link")));

    U32 fd = KInotifyObject::create(thread, 0);
    if ((S32)fd < 0) {
        testFail("inotify fd was not created, got %d (0x%X)", (S32)fd, fd);
        cleanupRoot(root);
        return;
    }

    U32 wd = KInotifyObject::addWatch(thread, fd, B("/tmp/watch-link"), K_IN_CREATE);
    if ((S32)wd < 0) {
        testFail("inotify watch was not added, got %d (0x%X)", (S32)wd, wd);
        cleanupRoot(root);
        return;
    }

    expectZero("mkdir watched symlink target child", process->mkdir(B("/tmp/watch-target/child")));
    if (!process->getFileDescriptor(fd)->kobject->isReadReady()) {
        testFail("inotify fd was not readable after watched symlink target mkdir");
        process->close(fd);
        process->removeThread(thread);
        delete thread;
        KSystem::eraseProcess(process->id);
        process->memory = nullptr;
        cleanupRoot(root);
        return;
    }

    U32 read = process->read(thread, fd, BUFFER, 64);
    expectU32("read symlink inotify event length", read, 24);
    expectU32("symlink event watch descriptor", memory->readd(BUFFER), wd);
    expectU32("symlink event mask", memory->readd(BUFFER + 4), K_IN_CREATE | K_IN_ISDIR);
    expectU32("symlink event cookie", memory->readd(BUFFER + 8), 0);
    expectU32("symlink event name length", memory->readd(BUFFER + 12), 8);
    if (strcmp(memory->readString(BUFFER + 16).c_str(), "child")) {
        testFail("symlink event name expected child, got %s", memory->readString(BUFFER + 16).c_str());
    }

    process->close(fd);
    process->removeThread(thread);
    delete thread;
    KSystem::eraseProcess(process->id);
    process->memory = nullptr;
    cleanupRoot(root);
}

void testInotifyPollReportsChildDirectoryDelete() {
    TestContext& context = testContext();
    KMemory* memory = context.memory;

    BString root = B("tmp/test-inotify-directory-delete-root");
    cleanupRoot(root);
    initTestFileSystem(root);
    Fs::makeLocalDirs(B("/tmp/watch"));
    Fs::makeLocalDirs(B("/proc"));
    setTestProcNode();

    KProcessPtr process = KProcess::create();
    process->memory = memory;
    KThread* thread = process->createThread();
    ChangeThread current(thread);

    U32 fd = KInotifyObject::create(thread, 0);
    if ((S32)fd < 0) {
        testFail("inotify fd was not created, got %d (0x%X)", (S32)fd, fd);
        cleanupRoot(root);
        return;
    }

    U32 wd = KInotifyObject::addWatch(thread, fd, B("/tmp/watch"), K_IN_DELETE);
    if ((S32)wd < 0) {
        testFail("inotify watch was not added, got %d (0x%X)", (S32)wd, wd);
        cleanupRoot(root);
        return;
    }

    KPollData pollData = {};
    pollData.fd = fd;
    pollData.events = K_POLLIN;
    expectU32("poll before watched rmdir", internal_poll(thread, &pollData, 1, 0), 0);
    expectU32("poll before watched rmdir revents", pollData.revents, 0);

    expectZero("mkdir watched child before delete", process->mkdir(B("/tmp/watch/child")));
    expectZero("rmdir watched child", process->rmdir(B("/tmp/watch/child")));

    pollData.revents = 0;
    expectU32("poll after watched rmdir", internal_poll(thread, &pollData, 1, 0), 1);
    expectU32("poll after watched rmdir revents", pollData.revents, K_POLLIN);

    U32 read = process->read(thread, fd, BUFFER, 64);
    expectU32("read inotify delete event length", read, 24);
    expectU32("delete event watch descriptor", memory->readd(BUFFER), wd);
    expectU32("delete event mask", memory->readd(BUFFER + 4), K_IN_DELETE | K_IN_ISDIR);
    expectU32("delete event cookie", memory->readd(BUFFER + 8), 0);
    expectU32("delete event name length", memory->readd(BUFFER + 12), 8);
    if (strcmp(memory->readString(BUFFER + 16).c_str(), "child")) {
        testFail("delete event name expected child, got %s", memory->readString(BUFFER + 16).c_str());
    }

    process->close(fd);
    process->removeThread(thread);
    delete thread;
    KSystem::eraseProcess(process->id);
    process->memory = nullptr;
    cleanupRoot(root);
}

void testInotifyAsyncSignalsSigioOnDelete() {
    TestContext& context = testContext();
    KMemory* memory = context.memory;

    BString root = B("tmp/test-inotify-async-sigio-root");
    cleanupRoot(root);
    initTestFileSystem(root);
    Fs::makeLocalDirs(B("/tmp/watch"));
    Fs::makeLocalDirs(B("/proc"));
    setTestProcNode();

    KProcessPtr process = KProcess::create();
    process->memory = memory;
    KThread* thread = process->createThread();
    ChangeThread current(thread);
    U64 oldSigMask = thread->sigMask;
    const U64 sigioBit = 1ULL << (K_SIGIO - 1);

    U32 fd = KInotifyObject::create(thread, 0);
    if ((S32)fd < 0) {
        testFail("inotify fd was not created, got %d (0x%X)", (S32)fd, fd);
        cleanupRoot(root);
        return;
    }

    U32 wd = KInotifyObject::addWatch(thread, fd, B("/tmp/watch"), K_IN_DELETE);
    if ((S32)wd < 0) {
        testFail("inotify watch was not added, got %d (0x%X)", (S32)wd, wd);
        cleanupRoot(root);
        return;
    }

    if (thread->cpu) {
        thread->cpu->reg[3].u32 = fd;
    }
    thread->sigMask |= sigioBit;
    expectZero("inotify fcntl F_SETSIG SIGIO", process->fcntrl(thread, fd, K_F_SETSIG, K_SIGIO));
    expectZero("inotify fcntl F_SETFL O_ASYNC", process->fcntrl(thread, fd, K_F_SETFL, K_O_ASYNC));

    expectZero("mkdir watched async child before delete", process->mkdir(B("/tmp/watch/child")));
    expectZero("rmdir watched async child", process->rmdir(B("/tmp/watch/child")));

    if (!(process->pendingSignals & sigioBit)) {
        testFail("inotify async delete did not signal SIGIO");
    }
    expectU32("inotify async SIGIO code", process->sigActions[K_SIGIO].sigInfo[2], K_POLL_IN);
    expectU32("inotify async SIGIO band", process->sigActions[K_SIGIO].sigInfo[3], 0);
    expectU32("inotify async SIGIO fd", process->sigActions[K_SIGIO].sigInfo[4], fd);

    thread->sigMask = oldSigMask;
    process->close(fd);
    process->removeThread(thread);
    delete thread;
    KSystem::eraseProcess(process->id);
    process->memory = nullptr;
    cleanupRoot(root);
}

#endif
