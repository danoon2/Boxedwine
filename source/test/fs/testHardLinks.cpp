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
#include "../../io/fsfilenode.h"

#ifdef __TEST

#include "../cpu/testCPU.h"
#include <filesystem>

namespace {

constexpr U32 STAT_A = TEST_HEAP_ADDRESS + 0x100;
constexpr U32 STAT_B = TEST_HEAP_ADDRESS + 0x200;
constexpr U32 BUFFER = TEST_HEAP_ADDRESS + 0x300;
constexpr U32 TIMES = TEST_HEAP_ADDRESS + 0x400;
constexpr U32 UTIME_OMIT = 0x3ffffffe;

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
    KSystem::procNode = nullptr;
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

} // namespace

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
    KSystem::procNode = Fs::getNodeFromLocalPath(B(""), B("/proc"), false);
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
    KSystem::procNode = Fs::getNodeFromLocalPath(B(""), B("/proc"), false);
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
    KSystem::procNode = Fs::getNodeFromLocalPath(B(""), B("/proc"), false);

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
    KSystem::procNode = Fs::getNodeFromLocalPath(B(""), B("/proc"), false);

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
    KSystem::procNode = Fs::getNodeFromLocalPath(B(""), B("/proc"), false);

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
    KSystem::procNode = Fs::getNodeFromLocalPath(B(""), B("/proc"), false);

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
    KSystem::procNode = Fs::getNodeFromLocalPath(B(""), B("/proc"), false);

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
    KSystem::procNode = Fs::getNodeFromLocalPath(B(""), B("/proc"), false);

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
