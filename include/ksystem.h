/*
 *  Copyright (C) 2016  The BoxedWine Team
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

#include "platform.h"
#include "pixelformat.h"

#define UID 1
#define GID 1000

#define MAX_STACK_SIZE (4*1024*1024)
#define INITIAL_STACK_PAGES 16
#define MAX_ADDRESS_SPACE 0xFFFF0000
#define MAX_NUMBER_OF_FILES 0xFFF
#define MAX_DATA_SIZE 1024*1024*1024

#define CALL_BACK_ADDRESS 0xFFFF0000
#define SIG_RETURN_ADDRESS CALL_BACK_ADDRESS

#define OPENGL_TYPE_NOT_SET 0
#define OPENGL_TYPE_UNAVAILABLE 1
#define OPENGL_TYPE_SDL 2
#define OPENGL_TYPE_OSMESA 3

class KTimerCallback;
class CPU;
class KProcess;
class KThread;

class MappedFileCache {
public:
    MappedFileCache(BString name) : name(name) {}
    virtual ~MappedFileCache();
    const BString name;
    std::shared_ptr<KFile> file;
    KRamPtr* data = nullptr;
    U32 dataSize = 0;
};

class SHM {
public:
    SHM(U32 id, U32 key) : id(id), key(key) {}
    virtual ~SHM();

    void incAttach() {this->nattch++;}
    void decAttach() {this->nattch--;}

    std::vector<KRamPtr> pages;
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

class KSystem {
public:    
    static bool videoEnabled;
    static U32 openglType;
    static bool soundEnabled;
    static U32 pentiumLevel;
	static bool shutingDown;
    static U32 killTime;
    static BString title;
    static U32 wineMajorVersion;
#ifdef BOXEDWINE_MULTI_THREADED
    static U32 cpuAffinityCountForApp;
#endif
    static U32 pollRate;
    static bool showWindowImmediately;
    static U32 skipFrameFPS;
    static BWriteFile logFile;
    static std::function<void(BString line)> watchTTY;
    static bool ttyPrepend;
    static BString exePath;
    
    static void init();
	static void destroy();
    static U32 getNextThreadId();

    // helpers
    static void writeStat(KProcess* process, BString path, U32 buf, bool is64, U64 st_dev, U64 st_ino, U32 st_mode, U64 st_rdev, U64 st_size, U32 st_blksize, U64 st_blocks, U64 mtime, U32 linkCount);
    static std::shared_ptr<KProcess> getProcess(U32 id);
    static void eraseFileCache(BString name);
    static std::shared_ptr<MappedFileCache> getFileCache(BString name);
    static void setFileCache(BString name, const std::shared_ptr<MappedFileCache>& fileCache);
    static void eraseProcess(U32 id);
    static std::shared_ptr<FsNode> addProcess(U32 id, const std::shared_ptr<KProcess>& process);
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

    static std::shared_ptr<FsNode> procNode;
private:
    static void initDisplayModes();
    static void internalEraseProcess(U32 id);

    static U32 nextThreadId;
    static bool adjustClock;
    static U32 adjustClockFactor; // 100 is normal
    static U32 startTimeTicks;
    static U64 startTimeMicroCounter;
    static U64 startTimeSystemTime;    
    static bool modesInitialized;
    
    static BHashTable<U32, std::shared_ptr<KProcess> > processes;
    static BHashTable<BString, std::shared_ptr<MappedFileCache> > fileCache;
    static BOXEDWINE_MUTEX fileCacheMutex;
};

void runThreadSlice(KThread* thread);
void ksyscall(CPU* cpu, U32 eipCount);

#endif
