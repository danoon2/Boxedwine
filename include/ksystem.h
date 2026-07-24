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

#ifndef __KSYSTEM_H__
#define __KSYSTEM_H__

#include "platformBoxedwine.h"
#include "pixelformat.h"

#define UID 1
#define GID 1000

#define MAX_STACK_SIZE (4*1024*1024)
#define INITIAL_STACK_PAGES 16
#define MAX_ADDRESS_SPACE 0xFFFF0000
#define MAX_NUMBER_OF_FILES 0x4000
#define MAX_DATA_SIZE 1024*1024*1024
// MappedFileCache uses a U32 page key and grows its backing vectors to
// pageIndex + 1. Keep the all-ones value unavailable so that size calculation
// remains representable before any mapping can reach a page fault.
constexpr U64 K_MAX_MAPPED_FILE_CACHE_PAGE = 0xfffffffeULL;

#define CALL_BACK_ADDRESS 0xFFFF0000
#define SIG_RETURN_ADDRESS CALL_BACK_ADDRESS

#define PF_COLOR_TYPE_NOTSET 0
#define PF_COLOR_TYPE_RGBA 1
#define PF_COLOR_TYPE_PALETTE 2

class KTimerCallback;
class CPU;
class KProcess;
class KThread;
class FsFileIdentity;
class MappedFileLease;
struct KWritebackResult;
struct KWritebackRange;

class MappedFileCache {
public:
    MappedFileCache(BString name, const std::shared_ptr<KFile>& file, U64 length);
    ~MappedFileCache();

    RamPage getOrCreatePage(U32 pageIndex, bool shared);
    void overlayRead(U64 offset, U8* buffer, U32 len);
    void updateWrite(U64 offset, const U8* buffer, U32 len);
    void setLength(U64 length);
    void setWriteFile(const std::shared_ptr<KFile>& file);
    U32 flush(U64 offset, U64 len);
#ifdef __TEST
    void setTestAfterPageReadHook(const std::function<void()>& hook);
    void setTestAfterFinalRetirementPreparationHook(const std::function<void()>& hook);
    void setTestFailWritebackPreparationAllocation(bool fail);
    KWritebackResult testWritebackPreparedBytes(U64 offset, const U8* buffer, U32 len, const std::function<void()>& afterPreparation);
    U32 getMappingLeaseCountForTest();
    U32 getRetirementAccountingFinalizeCountForTest();
#endif

    const BString name;
private:
    friend class KFile;
    friend class MappedFileLease;

    void retainMapping();
    U32 retireMapping(bool& accountingCommitted);
    S32 snapshotWritebackRangesLocked(U64 offset, U64 len, std::vector<KWritebackRange>& ranges, U64& preparedBytes);

    // Writeback implementations must copy writeFile and release mutex before
    // entering KFile::writeback. Its callback may take cache locks to snapshot
    // owned byte ranges; raw I/O begins only after the callback returns.
    std::shared_ptr<KFile> file;
    std::shared_ptr<KFile> writeFile;
    std::vector<RamPage> data;
    std::vector<bool> possiblyDirty;
    U64 length;
    U64 mutationGeneration = 0;
    U32 mappingLeaseCount = 0;
#ifdef __TEST
    std::function<void()> testAfterPageReadHook;
    std::shared_ptr<std::function<void()>> testAfterFinalRetirementPreparationHook;
    bool testFailWritebackPreparationAllocation = false;
    U32 testRetirementAccountingFinalizeCount = 0;
#endif
    // Serializes cache-data mutations; acquire before mutex when both are needed.
    BOXEDWINE_MUTEX mutationMutex;
    BOXEDWINE_MUTEX mutex;
};

class SHM {
public:
    SHM(U32 id, U32 key) : id(id), key(key) {}
    virtual ~SHM();

    void incAttach() {this->nattch++;}
    void decAttach() {this->nattch--;}

    std::vector<RamPage> pages;
    const U32 id;
    U32 len = 0;
    const U32 key;
    U32 cpid = 0;
    U32 lpid = 0;
    U64 ctime = 0;
    U64 dtime = 0;
    U64 atime = 0;
    U32 nattch = 0;
    U32 markedForDelete = 0;
    U32 cuid = 0;
    U32 cgid = 0;
};

enum VideoOption {
    VIDEO_NORMAL,
    VIDEO_NO_WINDOW,
    VIDEO_HIDE_WINDOW
};

class KSystem {
public:    
    static VideoOption videoOption;
    static BString openglLib;
    static bool soundEnabled;
    static bool enableSoundAfterMouseClick;
    static U32 pentiumLevel;
	static bool shutingDown;
    static U32 killTime;
    static U32 killTime2;
    static BString title;
    static U32 wineMajorVersion;
#ifdef BOXEDWINE_MULTI_THREADED
    static U32 cpuAffinityCountForApp;
#endif
    static U32 pollRate;
    static U32 skipFrameFPS;
    static BWriteFile logFile;
    static std::function<void(BString line)> watchTTY;
    static bool ttyPrepend;
    static BString exePath;
    static bool disableHideCursor;
    static bool forceRelativeMouse;
    static bool cacheReads;
    static bool disableWasmJitForWrittenCode;
    static bool useF64;
    static U32 pageSize;
    static bool canJitUse4KPage;

    static void init();
	static void destroy();
    static U32 getNextThreadId();

    // helpers
    static void writeStat(KProcess* process, BString path, U32 buf, bool is64, U64 st_dev, U64 st_ino, U32 st_mode, U64 st_rdev, U64 st_size, U32 st_blksize, U64 st_blocks, U64 mtime, U32 linkCount);
    static void writeStat(KProcess* process, BString path, U32 buf, bool is64, U64 st_dev, U64 st_ino, U32 st_mode, U64 st_rdev, U64 st_size, U32 st_blksize, U64 st_blocks, U64 atime, U32 atimeNano, U64 mtime, U32 mtimeNano, U64 ctime, U32 ctimeNano, U32 linkCount);
    static KProcessPtr getProcess(U32 id);
    static void eraseFileCache(BString name);
    static std::shared_ptr<MappedFileCache> getFileCache(const std::shared_ptr<FsFileIdentity>& identity);
    static void eraseProcess(U32 id);
    static std::shared_ptr<FsNode> addProcess(U32 id, const KProcessPtr& process);
    static KThread* getThreadById(U32 threadId);
    static U32 getRunningProcessCount();
    static U32 getProcessCount();
    static void printStacks();
    static void wakeThreadsWaitingOnProcessStateChanged();    

    // syscalls
    static U32 clock_getres(KThread* thread, U32 clk_id, U32 timespecAddress);
    static U32 clock_getres64(KThread* thread, U32 clk_id, U32 timespecAddress);
    static U32 clock_gettime(KThread* thread, U32 clock_id, U32 tp);
    static U32 clock_gettime64(KThread* thread, U32 clock_id, U32 tp);
    static U32 getpgid(U32 pid);
    static U32 gettimeofday(KThread* thread, U32 tv, U32 tz);
    static U32 kill(S32 pid, U32 signal);
    static U32 prlimit64(KThread* thread, U32 pid, U32 resource, U32 newlimit, U32 oldlimit);
    static U32 setpgid(U32 pid, U32 gpid);
    static U32 shmget(KThread* thread, U32 key, U32 size, U32 flags);
    static U32 shmat(KThread* thread, U32 shmid, U32 shmaddr, U32 shmflg, U32 rtnAddr, U32* nativeRtnAddr);
    static U32 shmdt(KThread* thread, U32 shmaddr);
    static U32 shmctl(KThread* thread, U32 shmid, U32 cmd, U32 buf);
    static U32 sysinfo(KThread* thread, U32 address);
    static U32 times(KThread* thread, U32 buf);
    static U32 tgkill(U32 threadGroupId, U32 threadId, U32 signal);
    static U32 ugetrlimit(KThread* thread, U32 resource, U32 rlim);
    static U32 uname(KThread* thread, U32 address);
    static U32 waitpid(KThread* thread, S32 pid, U32 statusAddress, U32 options);

    static BOXEDWINE_CONDITION processesCond;
    
    static U32 getMilliesSinceStart();
    static U64 getSystemTimeAsMicroSeconds();
    static U64 getMicroCounter();
    static void startMicroCounter();
    static U32 emulatedMilliesToHost(U32 millies);
    static U32 describePixelFormat(KThread* thread, U32 hdc, U32 fmt, U32 size, U32 descr);
    static PixelFormat* getPixelFormat(U32 index);
    static U32 getPixelFormatCount();
    static U32 findPixelFormat(U32 flags, U32 colorType, U32 cRedBits, U32 cGreenBits, U32 cBlueBits, U32 cAlphaBits, U32 cAccumBits, U32 cDepthBits, U32 cStencilBits);

    static BString getPlatform();
    static BString getArchitecture();
    static bool isWindows();
    static bool isMac();
    static bool isLinux();

    static void setProcNode(const std::shared_ptr<FsNode>& node);
    static std::shared_ptr<FsNode> procNode;
    static BString showWindowTimestamp;
private:
    friend class KFile;
    friend class KProcess;

    // Low-level mmap reconciliation. KFile is the only caller so the required
    // filePos -> identity ordering cannot be bypassed by a new mapping path.
    static std::shared_ptr<MappedFileCache> getOrCreateFileCache(const std::shared_ptr<FsFileIdentity>& identity, BString name, const std::shared_ptr<KFile>& file, U64 length, bool writable);
    static void reserveProcessPublication();
    static std::shared_ptr<FsNode> prepareProcessNode(U32 id);
    static void publishPreparedProcess(U32 id, const KProcessPtr& process, const std::shared_ptr<FsNode>& processNode);
    static void initDisplayModes();
    static void internalEraseProcess(U32 id);
    static KThread* getThreadByIdNoProcessLock(U32 threadId);
    static KThread* findSelectedPtraceStop(KThread* waiter, S32 pid, U32 parentGroupId, bool* hasPtraceTracee);
    static KProcessPtr findSelectedPtraceTermination(KThread* waiter, S32 pid, U32 parentGroupId, bool* hasPtraceTracee);

    static U32 nextThreadId;
    static bool adjustClock;
    static U32 adjustClockFactor; // 100 is normal
    static U32 startTimeTicks;
    static U64 startTimeMicroCounter;
    static U64 startTimeSystemTime;    
    static bool modesInitialized;

    static BHashTable<U32, KProcessPtr > processes;
    static BOXEDWINE_MUTEX processPublicationMutex;
    static BOXEDWINE_MUTEX fileCacheMutex;
};

void runThreadSlice(KThread* thread);
void ksyscall(CPU* cpu, U32 eipCount);

#endif
