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

#include "boxedwine.h"

#include "bufferaccess.h"
#include "kstat.h"
#include "kscheduler.h"
#include "../emulation/softmmu/soft_ram.h"
#include "../emulation/cpu/normal/normalCPU.h"
#include "knativesystem.h"
#include "pixelformat.h"
#include "../io/fsfilenode.h"

#include <time.h>

bool KSystem::modesInitialized = false;
U32 KSystem::skipFrameFPS = 0;
bool KSystem::videoEnabled = true;
#ifdef BOXEDWINE_OPENGL_SDL
U32 KSystem::openglType = OPENGL_TYPE_SDL;
#elif defined (BOXEDWINE_OPENGL_OSMESA) 
U32 KSystem::openglType = OPENGL_TYPE_OSMESA;
#else
U32 KSystem::openglType = OPENGL_TYPE_UNAVAILABLE;
#endif
bool KSystem::soundEnabled = true;
unsigned int KSystem::nextThreadId=10;
BHashTable<U32, std::shared_ptr<KProcess> > KSystem::processes;
BHashTable<BString, std::shared_ptr<MappedFileCache> > KSystem::fileCache;
BOXEDWINE_MUTEX KSystem::fileCacheMutex;
U32 KSystem::pentiumLevel = 4;
bool KSystem::shutingDown;
U32 KSystem::killTime;
bool KSystem::adjustClock;
U32 KSystem::adjustClockFactor=100;
U32 KSystem::startTimeTicks;
U64 KSystem::startTimeMicroCounter;
U64 KSystem::startTimeSystemTime;
BString KSystem::title;
// some simple opengl apps seem to have a hard time starting if this is false
// Not sure if this is a Boxedwine issue or if its normal for Windows to behave different for OpenGL if the window is hidden
bool KSystem::showWindowImmediately = false;
#ifdef BOXEDWINE_MULTI_THREADED
U32 KSystem::cpuAffinityCountForApp = 0;
#endif
U32 KSystem::pollRate = DEFAULT_POLL_RATE;
BWriteFile KSystem::logFile;
std::function<void(BString line)> KSystem::watchTTY;
bool KSystem::ttyPrepend;
BString KSystem::exePath;
std::shared_ptr<FsNode> KSystem::procNode;
U32 KSystem::wineMajorVersion;

BOXEDWINE_CONDITION KSystem::processesCond(std::make_shared<BoxedWineCondition>(B("KSystem::processesCond")));

void KSystem::init() {
    KSystem::adjustClock = false;
    KSystem::nextThreadId=10;
    KSystem::processes.clear();
    KSystem::fileCache.clear();
    KSystem::pentiumLevel = 4;
	KSystem::shutingDown = false;
    KSystem::startTimeTicks = KNativeSystem::getTicks();
    KSystem::startTimeMicroCounter = Platform::getMicroCounter();
    KSystem::startTimeSystemTime = Platform::getSystemTimeAsMicroSeconds();
    KSystem::killTime = 0;
}

void KSystem::destroy() {
	KThread::setCurrentThread(nullptr);
	KSystem::shutingDown = true;
    while (true) {
        std::shared_ptr<KProcess> p;
        {
            BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(processesCond);
            if (KSystem::processes.size()) {
                for (auto& process : KSystem::processes) {
                    if (process.value && process.value->getThreadCount()) {
                        p = process.value;
                        break;
                    }
                }
            }
        }
        if (!p) {
            break;
        }
        p->killAllThreads();
    }
    KSystem::procNode = nullptr;
	KSystem::processes.clear();
    KSystem::fileCache.clear();
	KSystem::shutingDown = false;
	Fs::shutDown();
    DecodedOp::clearCache();
    NormalCPU::clearCache();
    KMemory::shutdown();
    shutdownRam();
}

U32 KSystem::getProcessCount() {
    BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(processesCond);
    return (U32)KSystem::processes.size();
}

U32 KSystem::uname(KThread* thread, U32 address) {
    char name[64];
    strcpy(name, "Boxedwine");
    std::shared_ptr<FsNode> hostName = Fs::getNodeFromLocalPath(B(""), B("/etc/hostname"), true);
    if (hostName) {
        FsOpenNode* openHostName = hostName->open(K_O_RDONLY);
        if (openHostName) {
            openHostName->readNative((U8*)name, 64);
            openHostName->close();
        }
    }
    KMemory* memory = thread->memory;
    memory->strcpy(address, "Linux"); // sysname
    memory->strcpy(address + 65, name); // nodename
    memory->strcpy(address + 130, "5.15.10-20-generic"); // release
#ifdef BOXEDWINE_MULTI_THREADED
    if (Platform::getCpuCount() > 1 && KSystem::cpuAffinityCountForApp != 1) {
        memory->strcpy(address + 105, "SMP Boxedwine"); // version
    } else {
        memory->strcpy(address + 105, "Boxedwine"); // version
    }
#else
    memory->strcpy(address + 105, "Boxedwine"); // version
#endif
    memory->strcpy(address + 260, "i686"); // machine
    return 0;
}

U32 KSystem::ugetrlimit(KThread * thread, U32 resource, U32 rlim) {
    KMemory* memory = thread->memory;

    switch (resource) {
        case 2: // RLIMIT_DATA
            memory->writed(rlim, MAX_DATA_SIZE);
            memory->writed(rlim + 4, MAX_DATA_SIZE);
            break;
        case 3: // RLIMIT_STACK
            memory->writed(rlim, MAX_STACK_SIZE);
            memory->writed(rlim + 4, MAX_STACK_SIZE);
            break;
        case 4: // RLIMIT_CORE
            memory->writed(rlim, 1024 * 1024 * 4);
            memory->writed(rlim + 4, 1024 * 1024 * 4);
            break;
        case 5: // RLIMIT_DATA
            memory->writed(rlim, MAX_DATA_SIZE);
            memory->writed(rlim + 4, MAX_DATA_SIZE);
            break;
        case 6: // RLIMIT_MEMLOCK
            memory->writed(rlim, 64 * 1024 * 1024);
            memory->writed(rlim + 4, 64 * 1024 * 1024);
            break;
        case 7: // RLIMIT_NOFILE
            memory->writed(rlim, MAX_NUMBER_OF_FILES);
            memory->writed(rlim + 4, MAX_NUMBER_OF_FILES);
            break;
        case 9: // RLIMIT_AS
            memory->writed(rlim, MAX_ADDRESS_SPACE);
            memory->writed(rlim + 4, MAX_ADDRESS_SPACE);
            break;
        default:
            kpanic("ugetrlimit resource %d not implemented", resource);
    }
    return 0;
}

U32 KSystem::clock_gettime(KThread* thread, U32 clock_id, U32 tp) {
    KMemory* memory = thread->memory;

    if (clock_id==0 || clock_id==5) { // CLOCK_REALTIME / CLOCK_REALTIME_COARSE
        U64 m = KSystem::getSystemTimeAsMicroSeconds();
        memory->writed(tp, (U32)(m / 1000000l));
        memory->writed(tp + 4, (U32)(m % 1000000l) * 1000);
    } else if (clock_id==1 || clock_id==2 || clock_id==4 || clock_id==6) { // CLOCK_MONOTONIC_RAW, CLOCK_PROCESS_CPUTIME_ID , CLOCK_MONOTONIC_COARSE
        U64 diff = KSystem::getMicroCounter();
        memory->writed(tp, (U32)(diff / 1000000l));
        memory->writed(tp + 4, (U32)(diff % 1000000l) * 1000);
    } else {
        kpanic("Unknown clock id for clock_gettime: %d",clock_id);
    }
    return 0;
}

U32 KSystem::clock_gettime64(KThread* thread, U32 clock_id, U32 tp) {
    KMemory* memory = thread->memory;

    if (clock_id == 0 || clock_id == 5) { // CLOCK_REALTIME / CLOCK_REALTIME_COARSE
        U64 m = KSystem::getSystemTimeAsMicroSeconds();
        memory->writeq(tp, m / 1000000l);
        memory->writed(tp + 8, (U32)(m % 1000000l) * 1000);
    }
    else if (clock_id == 1 || clock_id == 2 || clock_id == 4 || clock_id == 6) { // CLOCK_MONOTONIC_RAW, CLOCK_PROCESS_CPUTIME_ID , CLOCK_MONOTONIC_COARSE
        U64 diff = KSystem::getMicroCounter();
        memory->writeq(tp, diff / 1000000l);
        memory->writed(tp + 8, (U32)(diff % 1000000l) * 1000);
    }
    else {
        kpanic("Unknown clock id for clock_gettime64: %d", clock_id);
    }
    return 0;
}

U32 KSystem::clock_getres(KThread* thread, U32 clk_id, U32 timespecAddress) {
    KMemory* memory = thread->memory;

    memory->writed(timespecAddress, 0);
    memory->writed(timespecAddress+4, 1000000);
    return 0;
}

U32 KSystem::clock_getres64(KThread* thread, U32 clk_id, U32 timespecAddress) {
    KMemory* memory = thread->memory;

    memory->writeq(timespecAddress, 0);
    memory->writed(timespecAddress + 8, 1000000);
    return 0;
}

KThread* KSystem::getThreadById(U32 threadId) {
    KThread* result = nullptr;
    BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(processesCond);

    for (auto& p : processes) {
        const std::shared_ptr<KProcess>& process = p.value;
        if (process) {
            result = process->getThreadById(threadId);
        }
        if (result) {
            break;
        }
    }
    return result;
}

U32 KSystem::tgkill(U32 threadGroupId, U32 threadId, U32 signal) {
    KThread* result = KSystem::getThreadById(threadId);

    if (!result) {
        return -K_ESRCH;;
    }

    return result->signal(signal, false);
}

/*
 struct sysinfo {
    long uptime;             // Seconds since boot
    unsigned long loads[3];  // 1, 5, and 15 minute load averages
    unsigned long totalram;  // Total usable main memory size
    unsigned long freeram;   // Available memory size
    unsigned long sharedram; // Amount of shared memory
    unsigned long bufferram; // Memory used by buffers
    unsigned long totalswap; // Total swap space size
    unsigned long freeswap;  // Swap space still available
    unsigned short procs;    // Number of current processes
    unsigned long totalhigh; // Total high memory size
    unsigned long freehigh;  // Available high memory size
    unsigned int mem_unit;   // Memory unit size in bytes
    char _f[20-2*sizeof(long)-sizeof(int)]; // Padding to 64 bytes
};
*/

U32 KSystem::sysinfo(KThread* thread, U32 address) {
    KMemory* memory = thread->memory;

    memory->writed(address, KSystem::getMilliesSinceStart()/1000); address+=4;
    memory->writed(address, 0); address+=4;
    memory->writed(address, 0); address+=4;
    memory->writed(address, 0); address+=4;
    memory->writed(address, 262144); address+=4; // 1 GB
    memory->writed(address, 196608); address+=4;
    memory->writed(address, 0); address+=4;
    memory->writed(address, 0); address+=4;
    memory->writed(address, 0); address+=4;
    memory->writed(address, 0); address+=4;
    memory->writew(address, (U16)KSystem::processes.size()); address+=2;
    memory->writed(address, 0); address+=4;
    memory->writed(address, 0); address+=4;
    memory->writed(address, K_PAGE_SIZE);
    return 0;
}

U32 syscall_ioperm(U32 from, U32 num, U32 turn_on) {
    return 0;
}

void KSystem::printStacks() {
    for (auto& n : KSystem::processes) {
        const std::shared_ptr<KProcess>& process = n.value;

        klog("process %X %s%s", process->id, process->terminated?"TERMINATED ":"", process->commandLine.c_str());
        process->printStack();
    }
}

U32 KSystem::kill(S32 pid, U32 signal) {
    std::shared_ptr<KProcess> process;
    {
        BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(processesCond);

        if (pid>0) {        
            process = KSystem::processes[pid];
        } else if (pid == 0 || pid == -1) {
            kpanic("kill with pid = %d not implemented", pid);
        } else {
            process = KSystem::processes[-pid];
        }
        if (!process) {
            return -K_ESRCH;
        }
    }
    if (signal!=0) {
        return process->signal(signal);
    }
    return 0;
}

void KSystem::wakeThreadsWaitingOnProcessStateChanged() {
    BOXEDWINE_CONDITION_SIGNAL_ALL(processesCond);
}

U32 KSystem::waitpid(KThread* thread, S32 pid, U32 statusAddress, U32 options) {
    std::shared_ptr<KProcess> process;
    U32 result = 0;
    KMemory* memory = thread->memory;

    BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(processesCond);

    while (!process) {
        if (pid>0) {
            process = KSystem::processes[pid];		
            if (!process) {
                return -K_ECHILD;
            }	
            if (!process->isStopped() && !process->isTerminated()) {
                process = 0;			
            }
        } else {
            if (pid==0)
                pid = thread->process->groupId;
            for (auto& n : KSystem::processes) {
                std::shared_ptr<KProcess> p = n.value;
                if (p && (p->isStopped() || p->isTerminated())) {
                    if (pid == -1) {
                        if (p->parentId == thread->process->id) {
                            process = p;
                            break;
                        }
                    } else {
                        if (p->groupId == (U32)(-pid)) {
                            process = p;
                            break;
                        }
                    }
                }
            }
        }
        if (!process) {
            if (options & 1) { // WNOHANG
                return 0;
            } else {                
                BOXEDWINE_CONDITION_WAIT(processesCond);
#ifdef BOXEDWINE_MULTI_THREADED
				if (KThread::currentThread()->terminating) {
					return -K_EINTR;
				}
                if (KThread::currentThread()->startSignal) {
                    KThread::currentThread()->startSignal = false;
                    return -K_CONTINUE;
                }
#endif
            }
        }     
    }
    if (statusAddress!=0) {
        int s = 0;
        if (process->isStopped()) {
            s |= 0x7f;
            s|=((process->signaled & 0xFF)<< 8);
        } else if (process->isTerminated()) {
            s|=((process->exitCode & 0xFF) << 8);
            s|=(process->signaled & 0x7F);
        }
        memory->writed(statusAddress, s);
    }
    result = process->id;
    KSystem::internalEraseProcess(result);
    return result;
}

U32 KSystem::times(KThread* thread, U32 buf) {
    if (buf) {
        KMemory* memory = thread->memory;

        memory->writed(buf, (U32)thread->userTime * 10); // user time
        memory->writed(buf + 4, (U32)thread->kernelTime * 10); // system time
        memory->writed(buf + 8, 0); // user time of children
        memory->writed(buf + 12, 0); // system time of children
    }
    return (U32)Platform::getMicroCounter()*10;
}

U32 KSystem::setpgid(U32 pid, U32 gpid) {	
    std::shared_ptr<KProcess> process;

    if (pid==0) {
        process = KThread::currentThread()->process;
    } else {
        BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(processesCond);
        process = processes[pid];
    }
    if (!process) {
        return 0; // :TODO:
        return -K_ESRCH;
    }
    if ((S32)gpid<0)
        return -K_EINVAL;
    if (gpid==0)
        gpid=process->id;
    process->groupId = gpid;
    return 0;
}

U32 KSystem::getpgid(U32 pid) {	
    std::shared_ptr<KProcess> process;
    if (pid==0)
        process = KThread::currentThread()->process;
    else {
        BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(processesCond);
        process = processes[pid];
    }
    if (!process)
        return -K_ESRCH;
    return process->groupId;
}

U32 KSystem::gettimeofday(KThread* thread, U32 tv, U32 tz) {
    U64 m = Platform::getSystemTimeAsMicroSeconds();
    KMemory* memory = thread->memory;

    memory->writed(tv, (U32)(m / 1000000l));
    memory->writed(tv + 4, (U32)(m % 1000000l));
    return 0;
}

void KSystem::writeStat(KProcess* process, BString path, U32 buf, bool is64, U64 st_dev, U64 st_ino, U32 st_mode, U64 st_rdev, U64 st_size, U32 st_blksize, U64 st_blocks, U64 mtime, U32 linkCount) {
    KMemory* memory = process->memory;

    if (path == "/tmp/.X11-unix") {
        st_mode= K__S_IFDIR |K__S_ISVTX | K__S_IRWXU | K__S_IRWXG | K__S_IRWXO;
    }
    if (path == "/var/run/samba/msg.lock" || path == "/run/samba/msg.lock") {
        st_mode = K__S_IFDIR | 0x1ED; // 755
    }

     if (is64) {
        U32 t = (U32)(mtime/1000); // ms to sec
        U32 n = (U32)(mtime % 1000) * 1000000;
        
        memory->writeq(buf, st_dev);buf+=8;//st_dev               // 0
        buf+=4; // padding                                          // 8
        memory->writed(buf, (U32)st_ino); buf += 4;//__st_ino     // 12
        memory->writed(buf, st_mode); buf += 4;//st_mode          // 16
        memory->writed(buf, linkCount); buf += 4;//st_nlink       // 20
        if (path == "/etc/sudoers" || path == "/tmp/.X11-unix") {
            memory->writed(buf, 0); buf += 4;//st_uid             // 24
            memory->writed(buf, 0); buf += 4;//st_gid               // 28
        } else {
            memory->writed(buf, process->userId); buf += 4;//st_uid           // 24
            memory->writed(buf, process->groupId); buf += 4;//st_gid               // 28
        }        
        memory->writeq(buf, st_rdev); buf += 8;//st_rdev          // 32
        buf+=4;                                                     // 40
        memory->writeq(buf, st_size); buf += 8;//st_size          // 44
        memory->writed(buf, st_blksize); buf += 4;//st_blksize    // 52
        memory->writeq(buf, st_blocks); buf += 8; //st_blocks     // 56
        memory->writed(buf, t); buf += 4; // st_atime             // 64
        memory->writed(buf, n); buf += 4; // st_atime_nsec        // 68
        memory->writed(buf, t); buf += 4; // st_mtime             // 72
        memory->writed(buf, n); buf += 4; // st_mtime_nsec        // 76
        memory->writed(buf, t); buf += 4; // st_ctime             // 80
        memory->writed(buf, n); buf += 4; // st_ctime_nsec        // 84
        memory->writeq(buf, st_ino); // st_ino                    // 88
     } else {
        U32 t = (U32)(mtime/1000); // ms to sec
        memory->writed(buf, (U32)st_dev); buf += 4;//st_dev
        memory->writed(buf, (U32)st_ino); buf += 4;//st_ino
        memory->writed(buf, st_mode); buf += 4;//st_mode
        memory->writed(buf, linkCount); buf += 4;//st_nlink
        if (path == "/etc/sudoers" || path == "/tmp/.X11-unix") {
            memory->writed(buf, 0); buf += 4;//st_uid
            memory->writed(buf, 0); buf += 4;//st_gid
        } else {
            memory->writed(buf, process->userId); buf += 4;//st_uid
            memory->writed(buf, process->groupId); buf += 4;//st_gid
        } 
        memory->writed(buf, (U32)st_rdev); buf += 4;//st_rdev
        memory->writed(buf, (U32)st_size); buf += 4;//st_size
        memory->writed(buf, t); buf += 4;//st_atime
        memory->writed(buf, t); buf += 4;//st_mtime
        memory->writed(buf, t); buf += 4;//st_ctime
        memory->writed(buf, st_blksize); buf += 4;//st_blksize (not used on wine)
        memory->writed(buf, (U32)st_blocks);//st_blocks
     }
}

static BHashTable<U32, std::shared_ptr<SHM> > publicShm;
static BHashTable<U32, std::shared_ptr<SHM> > shmKey;

#define IPC_CREAT  00001000   /* create if key is nonexistent */
#define IPC_EXCL   00002000   /* fail if key exists */

#define PRIVATE_SHMID 0x40000000

SHM::~SHM() {

}

U32 KSystem::shmget(KThread* thread, U32 key, U32 size, U32 flags) {
    S32 index = -1;
    std::shared_ptr<SHM> result;

    if (key==0) { // IPC_PRIVATE
        result = thread->process->allocSHM(key, PRIVATE_SHMID);
    } else {
        result = shmKey[key];
        if (result) {
            result = shmKey[key];
            if (flags & IPC_EXCL)
                return -K_EEXIST;
            if (!(flags & IPC_CREAT))
                return -K_ENOENT;
            return result->id;
        }
        index=1;
        while (publicShm.contains(index)) {
            index++;
            if (index>=0x40000000)
                return -K_ENOSPC;
        }        
        result = std::make_shared<SHM>(index, key);
        publicShm.set(index, result);
        shmKey.set(key, result);
    }
    result->cpid = thread->process->id;
    result->cuid = thread->process->effectiveUserId;
    result->cgid = thread->process->effectiveGroupId;
    result->ctime = Platform::getSystemTimeAsMicroSeconds();
    result->len = size;
    U32 pageCount = (size+K_PAGE_SIZE-1) / K_PAGE_SIZE;
    for (U32 i=0;i<pageCount;i++) {
        result->pages.push_back(ramPageAlloc());
    }
    return result->id;
}

#define SHM_RDONLY      010000  /* read-only access */
#define SHM_RND         020000  /* round attach address to SHMLBA boundary */
#define SHM_REMAP       040000  /* take-over region on attach */
#define SHM_EXEC        0100000 /* execution access */

U32 KSystem::shmat(KThread* thread, U32 shmid, U32 shmaddr, U32 shmflg, U32 rtnAddr, U32* nativeRtnAddr) {
    U32 permissions = 0;
    std::shared_ptr<SHM> shm;

    if (shmid & PRIVATE_SHMID) {
        shm = thread->process->getSHM(shmid);
    } else {
        shm = publicShm[shmid];
    }

    if (!shm) {
        return -K_EINVAL;
    }
    if (shmaddr && (shmflg & SHM_RND)) {
        shmaddr = shmaddr & ~K_PAGE_MASK;
    }
    if (shmaddr && (shmaddr & K_PAGE_MASK)) {
        return -K_EINVAL;
    }
    if (shmflg & SHM_REMAP) {
        kpanic("syscall_shmat SHM_REMAP not implemented");
    }
    if (shmflg & SHM_RDONLY) {
        permissions = K_PROT_READ;
    } else {
        permissions = K_PROT_READ | K_PROT_WRITE;
    }
    U32 result = thread->process->memory->mapPages(thread, shmaddr >> K_PAGE_SHIFT, shm->pages, permissions);
    if (result == 0) {
        return -K_EINVAL;
    }
    if (rtnAddr) {
        thread->process->memory->writed(rtnAddr, result);
    }
    if (nativeRtnAddr) {
        *nativeRtnAddr = result;
    }
    thread->process->attachSHM(result, shm);
    return 0;
}


U32 KSystem::shmdt(KThread* thread, U32 shmaddr) {
   return thread->process->shmdt(shmaddr);
}

#define IPC_RMID 0     /* remove resource */
#define IPC_SET  1     /* set ipc_perm options */
#define IPC_STAT 2     /* get ipc_perm options */
#define IPC_INFO 3     /* see ipcs */

/* super user shmctl commands */
#define SHM_LOCK        11
#define SHM_UNLOCK      12

/* ipcs ctl commands */
#define SHM_STAT        13
#define SHM_INFO        14

#define IPC_64  0x0100  /* New version (support 32-bit UIDs, bigger
                           message sizes, etc. */

U32 KSystem::shmctl(KThread* thread, U32 shmid, U32 cmd, U32 buf) {
    std::shared_ptr<SHM> shm;
    KMemory* memory = thread->memory;

    if (shmid & PRIVATE_SHMID) {
        shm = thread->process->getSHM(shmid);
    } else {
        shm = publicShm[shmid];
    }

    if (!shm) {
        return -K_EINVAL;
    }
    if (cmd & IPC_64) {
        cmd &= ~IPC_64;
    }
    if (cmd == IPC_RMID) {
        shm->markedForDelete = 1;
    }
    if (!buf)
        return -K_EFAULT;
    if (cmd == IPC_STAT) {
        // ipc_perm
        memory->writed(buf, shm->key); buf+=4;
        memory->writed(buf, shm->cuid); buf += 4;
        memory->writed(buf, shm->cgid); buf += 4;
        memory->writed(buf, shm->cuid); buf += 4;
        memory->writed(buf, shm->cgid); buf += 4;
        memory->writew(buf, 0777); buf += 2;
        memory->writew(buf, 0); buf += 2;
        memory->writew(buf, shmid); buf += 2;
        memory->writew(buf, 0); buf += 2;
        memory->writed(buf, 0); buf += 4;
        memory->writed(buf, 0); buf += 4;
        memory->writed(buf, shm->len); buf += 4;
        memory->writed(buf, (U32)(shm->atime / 1000000)); buf += 4;
        memory->writed(buf, (U32)(shm->dtime / 1000000)); buf += 4;
        memory->writed(buf, (U32)(shm->ctime / 1000000)); buf += 4;
        memory->writed(buf, shm->cpid); buf += 4;
        memory->writed(buf, shm->lpid); buf += 4;
        memory->writew(buf, shm->nattch); buf += 2;
        memory->writew(buf, 0); buf += 2;
        memory->writed(buf, 0);
    } else {
        kpanic("Unknown syscall_shmctl cmd=%X", cmd);
    }
    return 0;
}

#define K_RLIM_INFINITY 0xFFFFFFFF

U32 KSystem::prlimit64(KThread* thread, U32 pid, U32 resource, U32 newlimit, U32 oldlimit) {
    std::shared_ptr<KProcess> process;
    KMemory* memory = thread->memory;

    if (pid==0) {
        process = thread->process;
    } else {
        BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(processesCond);
        process = processes[pid];
        if (!process)
            return -K_ESRCH;
    }
    switch (resource) {
        case 0: // RLIMIT_CPU
            if (oldlimit!=0) {
                memory->writeq(oldlimit, 0x7FFFFFFF);
                memory->writeq(oldlimit + 8, 0x7FFFFFFF);
            }
#ifdef _DEBUG
            if (newlimit!=0) {
                klog("prlimit64 RLIMIT_CPU set=%d ignored", (U32)memory->readq(newlimit));
            }
#endif
            break;
        case 1: // RLIMIT_FSIZE
            if (oldlimit!=0) {
                memory->writeq(oldlimit, 0x800000000);
                memory->writeq(oldlimit + 8, 0x800000000);
            }
#ifdef _DEBUG
            if (newlimit!=0) {
                klog("prlimit64 RLIMIT_FSIZE set=%d ignored", (U32)memory->readq(newlimit));
            }
#endif
            break;
        case 2: // RLIMIT_DATA
            if (oldlimit!=0) {
                memory->writeq(oldlimit, MAX_DATA_SIZE);
                memory->writeq(oldlimit + 8, MAX_DATA_SIZE);
            }
#ifdef _DEBUG
            if (newlimit!=0) {
                klog("prlimit64 RLIMIT_DATA set=%d ignored", (U32)memory->readq(newlimit));
            }
#endif
            break;
        case 3: // RLIMIT_STACK
            if (oldlimit!=0) {
                memory->writeq(oldlimit, MAX_STACK_SIZE);
                memory->writeq(oldlimit + 8, MAX_STACK_SIZE);
            }
#ifdef _DEBUG
            if (newlimit!=0) {
                klog("prlimit64 RLIMIT_STACK set=%d ignored", (U32)memory->readq(newlimit));
            }
#endif
            break;
        case 4: // RLIMIT_CORE
            if (oldlimit!=0) {
                memory->writeq(oldlimit, K_RLIM_INFINITY);
                memory->writeq(oldlimit + 8, K_RLIM_INFINITY);
            }
#ifdef _DEBUG
            if (newlimit!=0) {
                klog("prlimit64 RLIMIT_CORE set=%d ignored", (U32)memory->readq(newlimit));
            }
#endif
            break;
        case 5: // RLIMIT_RSS
            if (oldlimit!=0) {
                memory->writeq(oldlimit, MAX_DATA_SIZE);
                memory->writeq(oldlimit + 8, MAX_DATA_SIZE);
            }
#ifdef _DEBUG
            if (newlimit!=0) {
                klog("prlimit64 RLIMIT_RSS set=%d ignored", (U32)memory->readq(newlimit));
            }
#endif
            break;
        case 6: // RLIMIT_NPROC
            if (oldlimit!=0) {
                memory->writeq(oldlimit, 4096);
                memory->writeq(oldlimit + 8, 4096);
            }
#ifdef _DEBUG
            if (newlimit!=0) {
                klog("prlimit64 RLIMIT_NPROC set=%d ignored", (U32)memory->readq(newlimit));
            }
#endif
            break;
        case 7: // RLIMIT_NOFILE
            if (oldlimit!=0) {
                memory->writeq(oldlimit, 16*1024); // some apps might iterate all the possible file handles, so don't make this too big
                memory->writeq(oldlimit + 8, 16*1024);
            }
#ifdef _DEBUG
            if (newlimit!=0) {
                klog("prlimit64 RLIMIT_NOFILE set=%d ignored", (U32)memory->readq(newlimit));
            }
#endif
            break;
        case 9: // RLIMIT_AS
            if (oldlimit!=0) {
                memory->writeq(oldlimit, K_RLIM_INFINITY);
                memory->writeq(oldlimit + 8, K_RLIM_INFINITY);
            }
#ifdef _DEBUG
            if (newlimit!=0) {
                klog("prlimit64 RLIMIT_AS set=%d ignored", (U32)memory->readq(newlimit));
            }
#endif
            break;
        case 15: // RLIMIT_RTTIME
            if (oldlimit!=0) {
                memory->writeq(oldlimit, 200);
                memory->writeq(oldlimit + 8, 200);
            }
#ifdef _DEBUG
            if (newlimit!=0) {
                klog("prlimit64 RLIMIT_AS set=%d ignored", (U32)memory->readq(newlimit));
            }
#endif
            break;
        default:
            kpanic("prlimit64 resource %d not handled", resource);
    }
    return 0;
}

std::shared_ptr<KProcess> KSystem::getProcess(U32 id) {
    BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(processesCond);
    return KSystem::processes[id];
}

void KSystem::eraseFileCache(BString name) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(KSystem::fileCacheMutex);
    KSystem::fileCache.remove(name);
}

std::shared_ptr<MappedFileCache> KSystem::getFileCache(BString name) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(KSystem::fileCacheMutex);
    return KSystem::fileCache[name];
}

void KSystem::setFileCache(BString name, const std::shared_ptr<MappedFileCache>& fileCache) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(KSystem::fileCacheMutex);
    KSystem::fileCache.set(name, fileCache);
}

void KSystem::internalEraseProcess(U32 id) {
    KSystem::processes.remove(id);
    KSystem::procNode->removeChildByName(BString::valueOf(id));
}

void KSystem::eraseProcess(U32 id) {
    BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(processesCond);
    KSystem::internalEraseProcess(id);    
}

std::shared_ptr<FsNode> KSystem::addProcess(U32 id, const std::shared_ptr<KProcess>& process) {
    BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(processesCond);
    KSystem::processes.set(id, process);
    if (KSystem::procNode) {
        std::shared_ptr<FsNode> processNode = Fs::addFileNode("/proc/" + BString::valueOf(id), B(""), B(""), true, KSystem::procNode);
        KSystem::procNode->addChild(processNode);
        return processNode;
    }
    return nullptr;
}

U32 KSystem::getRunningProcessCount() {
    BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(processesCond);
    U32 count = 0;
    for (auto& n : KSystem::processes) {
        const std::shared_ptr<KProcess>& openProcess = n.value;
        if (openProcess && !openProcess->isStopped() && !openProcess->isTerminated()) {
            count++;
        }
    }
    return count;
}

U32 KSystem::getMilliesSinceStart() {
    if (!KSystem::adjustClock) {
        return KNativeSystem::getTicks();
    }
    U32 currentTicks = KNativeSystem::getTicks();
    U32 diff = currentTicks - KSystem::startTimeTicks;
    U32 adjustedDiff = diff * KSystem::adjustClockFactor / 100;
    return KSystem::startTimeTicks + adjustedDiff;
}

U64 KSystem::getSystemTimeAsMicroSeconds() {
    if (!KSystem::adjustClock) {
        return Platform::getSystemTimeAsMicroSeconds();
    }
    U64 currentTime = Platform::getSystemTimeAsMicroSeconds();
    U64 diff = currentTime - KSystem::startTimeSystemTime;
    U64 adjustedDiff = diff * KSystem::adjustClockFactor / 100;
    return KSystem::startTimeSystemTime + adjustedDiff;
}

U64 KSystem::getMicroCounter() {
    if (!KSystem::adjustClock) {
        return Platform::getMicroCounter();
    }
    U64 currentTicks = Platform::getMicroCounter();
    U64 diff = currentTicks - KSystem::startTimeMicroCounter;
    U64 adjustedDiff = diff * KSystem::adjustClockFactor / 100;
    return KSystem::startTimeMicroCounter + adjustedDiff;
}

void KSystem::startMicroCounter() {
    Platform::startMicroCounter();
}

U32 KSystem::emulatedMilliesToHost(U32 millies) {
    if (!KSystem::adjustClock) {
        return millies;
    }
    return millies * 100 / KSystem::adjustClockFactor;
}

U32 KSystem::getNextThreadId() {
    BOXEDWINE_CRITICAL_SECTION;
    return KSystem::nextThreadId++;
}

PixelFormat pfs[1024];
U32 numberOfPfs;

void KSystem::initDisplayModes() {
    if (!modesInitialized) {
        modesInitialized = 1;
        numberOfPfs = getPixelFormats(pfs, sizeof(pfs) / sizeof(PixelFormat));
    }
}

PixelFormat* KSystem::getPixelFormat(U32 index) {
    if (index < numberOfPfs) {
        return &pfs[index];
    }
    return nullptr;
}

U32 KSystem::describePixelFormat(KThread* thread, U32 hdc, U32 fmt, U32 size, U32 descr)
{
    KMemory* memory = thread->memory;

    initDisplayModes();

    if (!descr) return numberOfPfs;
    if (size < 40) return 0;
    if (fmt > numberOfPfs) {
        return 0;
    }

    memory->writew(descr, pfs[fmt].nSize); descr += 2;
    memory->writew(descr, pfs[fmt].nVersion); descr += 2;
    memory->writed(descr, pfs[fmt].dwFlags); descr += 4;
    memory->writeb(descr, pfs[fmt].iPixelType); descr++;
    memory->writeb(descr, pfs[fmt].cColorBits); descr++;
    memory->writeb(descr, pfs[fmt].cRedBits); descr++;
    memory->writeb(descr, pfs[fmt].cRedShift); descr++;
    memory->writeb(descr, pfs[fmt].cGreenBits); descr++;
    memory->writeb(descr, pfs[fmt].cGreenShift); descr++;
    memory->writeb(descr, pfs[fmt].cBlueBits); descr++;
    memory->writeb(descr, pfs[fmt].cBlueShift); descr++;
    memory->writeb(descr, pfs[fmt].cAlphaBits); descr++;
    memory->writeb(descr, pfs[fmt].cAlphaShift); descr++;
    memory->writeb(descr, pfs[fmt].cAccumBits); descr++;
    memory->writeb(descr, pfs[fmt].cAccumRedBits); descr++;
    memory->writeb(descr, pfs[fmt].cAccumGreenBits); descr++;
    memory->writeb(descr, pfs[fmt].cAccumBlueBits); descr++;
    memory->writeb(descr, pfs[fmt].cAccumAlphaBits); descr++;
    memory->writeb(descr, pfs[fmt].cDepthBits); descr++;
    memory->writeb(descr, pfs[fmt].cStencilBits); descr++;
    memory->writeb(descr, pfs[fmt].cAuxBuffers); descr++;
    memory->writeb(descr, pfs[fmt].iLayerType); descr++;
    memory->writeb(descr, pfs[fmt].bReserved); descr++;
    memory->writed(descr, pfs[fmt].dwLayerMask); descr += 4;
    memory->writed(descr, pfs[fmt].dwVisibleMask); descr += 4;
    memory->writed(descr, pfs[fmt].dwDamageMask);

    return numberOfPfs;
}

/*
void writePixelFormat(KThread* thread, PixelFormat* pf, U32 descr) {
    pf->nSize = readw(descr); descr += 2;
    pf->nVersion = readw(descr); descr += 2;
    pf->dwFlags = readd(descr); descr += 4;
    pf->iPixelType = readb(descr); descr++;
    pf->cColorBits = readb(descr); descr++;
    pf->cRedBits = readb(descr); descr++;
    pf->cRedShift = readb(descr); descr++;
    pf->cGreenBits = readb(descr); descr++;
    pf->cGreenShift = readb(descr); descr++;
    pf->cBlueBits = readb(descr); descr++;
    pf->cBlueShift = readb(descr); descr++;
    pf->cAlphaBits = readb(descr); descr++;
    pf->cAlphaShift = readb(descr); descr++;
    pf->cAccumBits = readb(descr); descr++;
    pf->cAccumRedBits = readb(descr); descr++;
    pf->cAccumGreenBits = readb(descr); descr++;
    pf->cAccumBlueBits = readb(descr); descr++;
    pf->cAccumAlphaBits = readb(descr); descr++;
    pf->cDepthBits = readb(descr); descr++;
    pf->cStencilBits = readb(descr); descr++;
    pf->cAuxBuffers = readb(descr); descr++;
    pf->iLayerType = readb(descr); descr++;
    pf->bReserved = readb(descr); descr++;
    pf->dwLayerMask = readd(descr); descr += 4;
    pf->dwVisibleMask = readd(descr); descr += 4;
    pf->dwDamageMask = readd(descr);
}
*/
