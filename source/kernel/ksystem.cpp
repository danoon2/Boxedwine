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

#include <time.h>
#include <SDL.h>

unsigned int KSystem::nextThreadId=10;
std::unordered_map<void*, SHM*> KSystem::shm;
std::unordered_map<U32, std::shared_ptr<KProcess> > KSystem::processes;
#ifdef BOXEDWINE_DEFAULT_MMU
std::unordered_map<std::string, BoxedPtr<MappedFileCache> > KSystem::fileCache;
BOXEDWINE_MUTEX KSystem::fileCacheMutex;
#endif
U32 KSystem::pentiumLevel = 4;
bool KSystem::shutingDown;
U32 KSystem::killTime;
bool KSystem::adjustClock;
U32 KSystem::adjustClockFactor=100;
U32 KSystem::startTimeSdlTicks;
U64 KSystem::startTimeMicroCounter;
U64 KSystem::startTimeSystemTime;
#ifdef BOXEDWINE_X64
bool KSystem::useLargeAddressSpace = true;
#endif
#ifdef BOXEDWINE_MULTI_THREADED
U32 KSystem::cpuAffinityCountForApp = 1;
#endif

BOXEDWINE_CONDITION KSystem::processesCond("KSystem::processesCond");

void KSystem::init() {
    KSystem::adjustClock = false;
    KSystem::nextThreadId=10;
    KSystem::shm.clear();
    KSystem::processes.clear();
#ifdef BOXEDWINE_DEFAULT_MMU
    KSystem::fileCache.clear();
#endif
    KSystem::pentiumLevel = 4;
	KSystem::shutingDown = false;
    KSystem::startTimeSdlTicks = SDL_GetTicks();
    KSystem::startTimeMicroCounter = Platform::getMicroCounter();
    KSystem::startTimeSystemTime = Platform::getSystemTimeAsMicroSeconds();
}

void KSystem::destroy() {
	KThread::setCurrentThread(NULL);
	KSystem::shutingDown = true;
    while (true) {
        std::shared_ptr<KProcess> p;
        {
            BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(processesCond);
            if (KSystem::processes.size()) {
                for (auto& process : KSystem::processes) {
                    if (process.second->getThreadCount()) {
                        p = process.second;
                        break;
                    }
                }
            }
        }
        if (!p) {
            break;
        }
        p->killAllThreadsExceptCurrent();
    }
	KSystem::processes.clear();
    KSystem::shm.clear();
#ifdef BOXEDWINE_DEFAULT_MMU
    KSystem::fileCache.clear();
#endif
	KSystem::shutingDown = false;
	Fs::shutDown();
    DecodedOp::clearCache();
    NormalCPU::clearCache();
}

U32 KSystem::getProcessCount() {
    BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(processesCond);
    return (U32)KSystem::processes.size();
}

U32 KSystem::uname(U32 address) {
    writeNativeString(address, "Linux");
    writeNativeString(address + 65, "Linux");
    //writeNativeString(address + 130, "4.19.0-6-686");
    writeNativeString(address + 130, "4.15.0-20-generic");
    writeNativeString(address + 260, "i686");
    return 0;
}

U32 KSystem::ugetrlimit(U32 resource, U32 rlim) {
    switch (resource) {
        case 2: // RLIMIT_DATA
            writed(rlim, MAX_DATA_SIZE);
            writed(rlim + 4, MAX_DATA_SIZE);
            break;
        case 3: // RLIMIT_STACK
            writed(rlim, MAX_STACK_SIZE);
            writed(rlim + 4, MAX_STACK_SIZE);
            break;
        case 4: // RLIMIT_CORE
            writed(rlim, 1024 * 1024 * 4);
            writed(rlim + 4, 1024 * 1024 * 4);
            break;
        case 5: // RLIMIT_DATA
            writed(rlim, MAX_DATA_SIZE);
            writed(rlim + 4, MAX_DATA_SIZE);
            break;
        case 6: // RLIMIT_MEMLOCK
            writed(rlim, 64 * 1024 * 1024);
            writed(rlim + 4, 64 * 1024 * 1024);
            break;
        case 7: // RLIMIT_NOFILE
            writed(rlim, MAX_NUMBER_OF_FILES);
            writed(rlim + 4, MAX_NUMBER_OF_FILES);
            break;
        case 9: // RLIMIT_AS
            writed(rlim, MAX_ADDRESS_SPACE);
            writed(rlim + 4, MAX_ADDRESS_SPACE);
            break;
        default:
            kpanic("ugetrlimit resource %d not implemented", resource);
    }
    return 0;
}

U32 KSystem::clock_gettime(U32 clock_id, U32 tp) {    
    if (clock_id==0 || clock_id==5) { // CLOCK_REALTIME / CLOCK_REALTIME_COARSE
        U64 m = KSystem::getSystemTimeAsMicroSeconds();
        writed(tp, (U32)(m / 1000000l));
        writed(tp + 4, (U32)(m % 1000000l) * 1000);
    } else if (clock_id==1 || clock_id==2 || clock_id==4 || clock_id==6) { // CLOCK_MONOTONIC_RAW, CLOCK_PROCESS_CPUTIME_ID , CLOCK_MONOTONIC_COARSE
        U64 diff = KSystem::getMicroCounter();
        writed(tp, (U32)(diff / 1000000l));
        writed(tp + 4, (U32)(diff % 1000000l) * 1000);
    } else {
        kpanic("Unknown clock id for clock_gettime: %d",clock_id);
    }
    return 0;
}

U32 KSystem::clock_getres(U32 clk_id, U32 timespecAddress) {
    writed(timespecAddress, 0);
    writed(timespecAddress+4, 1000000);
    return 0;
}

KThread* KSystem::getThreadById(U32 threadId) {
    KThread* result = NULL;
    BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(processesCond);

    for (auto& p : processes) {
        const std::shared_ptr<KProcess>& process = p.second;
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

U32 KSystem::sysinfo(U32 address) {
    BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(processesCond);

    writed(address, KSystem::getMilliesSinceStart()/1000); address+=4;
    writed(address, 0); address+=4;
    writed(address, 0); address+=4;
    writed(address, 0); address+=4;
    writed(address, 262144); address+=4; // 1 GB
    writed(address, 196608); address+=4;
    writed(address, 0); address+=4;
    writed(address, 0); address+=4;
    writed(address, 0); address+=4;
    writed(address, 0); address+=4;
    writew(address, (U16)KSystem::processes.size()); address+=2;
    writed(address, 0); address+=4;
    writed(address, 0); address+=4;
    writed(address, K_PAGE_SIZE);
    return 0;
}

U32 syscall_ioperm(U32 from, U32 num, U32 turn_on) {
    return 0;
}

void KSystem::printStacks() {
    for (auto& n : KSystem::processes) {
        const std::shared_ptr<KProcess>& process = n.second;

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

U32 KSystem::waitpid(S32 pid, U32 statusAddress, U32 options) {    
    std::shared_ptr<KProcess> process;
    U32 result;
    KThread* thread = KThread::currentThread();
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
                std::shared_ptr<KProcess> p = n.second;
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
        writed(statusAddress, s);
    }
    result = process->id;
    KSystem::eraseProcess(result);
    return result;
}

U32 KSystem::times(U32 buf) {
    if (buf) {
        KThread* thread = KThread::currentThread();
        writed(buf, (U32)thread->userTime * 10); // user time
        writed(buf + 4, (U32)thread->kernelTime * 10); // system time
        writed(buf + 8, 0); // user time of children
        writed(buf + 12, 0); // system time of children
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

U32 KSystem::gettimeofday(U32 tv, U32 tz) {
    U64 m = Platform::getSystemTimeAsMicroSeconds();
    
    writed(tv, (U32)(m / 1000000l));
    writed(tv + 4, (U32)(m % 1000000l));
    return 0;
}

void KSystem::writeStat(const std::string& path, U32 buf, bool is64, U64 st_dev, U64 st_ino, U32 st_mode, U64 st_rdev, U64 st_size, U32 st_blksize, U64 st_blocks, U64 mtime, U32 linkCount) {
    if (!path.compare("/tmp/.X11-unix")) {
        st_mode= K__S_IFDIR |K__S_ISVTX | K__S_IRWXU | K__S_IRWXG | K__S_IRWXO;
    }
    if (!path.compare("/var/run/samba/msg.lock") || !path.compare("/run/samba/msg.lock")) {
        st_mode = K__S_IFDIR | 0x1ED; // 755
    }

     if (is64) {
        U32 t = (U32)(mtime/1000); // ms to sec
        U32 n = (U32)(mtime % 1000) * 1000000;
        
        writeq(buf, st_dev);buf+=8;//st_dev               // 0
        buf+=4; // padding                                          // 8
        writed(buf, (U32)st_ino); buf += 4;//__st_ino     // 12
        writed(buf, st_mode); buf += 4;//st_mode          // 16
        writed(buf, linkCount); buf += 4;//st_nlink       // 20
        if (!path.compare("/etc/sudoers") || !path.compare("/tmp/.X11-unix")) {
            writed(buf, 0); buf += 4;//st_uid             // 24
            writed(buf, 0); buf += 4;//st_gid               // 28
        } else {
            writed(buf, KThread::currentThread()->process->userId); buf += 4;//st_uid           // 24
            writed(buf, KThread::currentThread()->process->groupId); buf += 4;//st_gid               // 28
        }        
        writeq(buf, st_rdev); buf += 8;//st_rdev          // 32
        buf+=4;                                                     // 40
        writeq(buf, st_size); buf += 8;//st_size          // 44
        writed(buf, st_blksize); buf += 4;//st_blksize    // 52
        writeq(buf, st_blocks); buf += 8; //st_blocks     // 56
        writed(buf, t); buf += 4; // st_atime             // 64
        writed(buf, n); buf += 4; // st_atime_nsec        // 68
        writed(buf, t); buf += 4; // st_mtime             // 72
        writed(buf, n); buf += 4; // st_mtime_nsec        // 76
        writed(buf, t); buf += 4; // st_ctime             // 80
        writed(buf, n); buf += 4; // st_ctime_nsec        // 84
        writeq(buf, st_ino); // st_ino                    // 88
     } else {
        U32 t = (U32)(mtime/1000); // ms to sec
        writed(buf, (U32)st_dev); buf += 4;//st_dev
        writed(buf, (U32)st_ino); buf += 4;//st_ino
        writed(buf, st_mode); buf += 4;//st_mode
        writed(buf, linkCount); buf += 4;//st_nlink
        if (!path.compare("/etc/sudoers") || !path.compare("/tmp/.X11-unix")) {
            writed(buf, 0); buf += 4;//st_uid
            writed(buf, 0); buf += 4;//st_gid
        } else {
            writed(buf, KThread::currentThread()->process->userId); buf += 4;//st_uid
            writed(buf, KThread::currentThread()->process->groupId); buf += 4;//st_gid
        } 
        writed(buf, (U32)st_rdev); buf += 4;//st_rdev
        writed(buf, (U32)st_size); buf += 4;//st_size
        writed(buf, t); buf += 4;//st_atime
        writed(buf, t); buf += 4;//st_mtime
        writed(buf, t); buf += 4;//st_ctime
        writed(buf, st_blksize); buf += 4;//st_blksize (not used on wine)
        writed(buf, (U32)st_blocks);//st_blocks
     }
}

static std::unordered_map<U32, BoxedPtr<SHM> > publicShm;
static std::unordered_map<U32, BoxedPtr<SHM> > shmKey;

#define IPC_CREAT  00001000   /* create if key is nonexistent */
#define IPC_EXCL   00002000   /* fail if key exists */

#define PRIVATE_SHMID 0x40000000

SHM::~SHM() {
    for (U32 i=0;i<(U32)this->pages.size();i++) {
        ramPageDecRef(this->pages[i]);
    }
}

U32 KSystem::shmget(U32 key, U32 size, U32 flags) {
    KThread* thread = KThread::currentThread();
    S32 index = -1;
    BoxedPtr<SHM> result;

    if (key==0) { // IPC_PRIVATE
        result = thread->process->allocSHM(key, PRIVATE_SHMID);
    } else {
        if (shmKey.count(key)) {
            result = shmKey[key];
            if (flags & IPC_EXCL)
                return -K_EEXIST;
            if (!(flags & IPC_CREAT))
                return -K_ENOENT;
            return result->id;
        }
        index=1;
        while (publicShm.count(index)) {
            index++;
            if (index>=0x40000000)
                return -K_ENOSPC;
        }        
        result = new SHM(index, key);
        publicShm[index] = result;
        shmKey[key] = result;
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

U32 KSystem::shmat(U32 shmid, U32 shmaddr, U32 shmflg, U32 rtnAddr) {	
    KThread* thread = KThread::currentThread();
    U32 result = 0;
    U32 permissions;
    BoxedPtr<SHM> shm;

    if (shmid & PRIVATE_SHMID)
        shm = thread->process->getSHM(shmid);
    else if (publicShm.count(shmid))
        shm = publicShm[shmid];

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
    if (!shmaddr) {
        shmaddr = ADDRESS_PROCESS_MMAP_START << K_PAGE_SHIFT;
    }
    if (thread->process->memory->findFirstAvailablePage(shmaddr >> K_PAGE_SHIFT, (shm->len + K_PAGE_SIZE - 1) / K_PAGE_SIZE, &result, 0)) {
        return -K_EINVAL;
    }
    if (shmflg & SHM_RDONLY) {
        permissions = PAGE_READ;
    } else {
        permissions = PAGE_READ|PAGE_WRITE;
    }
    thread->process->memory->map(result >> K_PAGE_SHIFT, shm->pages, permissions);    
    thread->process->attachSHM(result, shm);
    return 0;
}


U32 KSystem::shmdt(U32 shmaddr) {
   return KThread::currentThread()->process->shmdt(shmaddr);
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

U32 KSystem::shmctl(U32 shmid, U32 cmd, U32 buf) {	
    BoxedPtr<SHM> shm;
    KThread* thread = KThread::currentThread();

    if (shmid & PRIVATE_SHMID)
        shm = thread->process->getSHM(shmid);
    else if (publicShm.count(shmid))
        shm = publicShm[shmid];

    if (!shm) {
        return -K_EINVAL;
    }

    if (!buf)
        return -K_EFAULT;
    if (cmd == (IPC_64 | IPC_STAT)) {
        // ipc_perm
        writed(buf, shm->key); buf+=4;
        writed(buf, shm->cuid); buf += 4;
        writed(buf, shm->cgid); buf += 4;
        writed(buf, shm->cuid); buf += 4;
        writed(buf, shm->cgid); buf += 4;
        writew(buf, 0777); buf += 2;
        writew(buf, 0); buf += 2;
        writew(buf, shmid); buf += 2;
        writew(buf, 0); buf += 2;
        writed(buf, 0); buf += 4;
        writed(buf, 0); buf += 4;
        writed(buf, shm->len); buf += 4;
        writed(buf, (U32)(shm->atime / 1000000)); buf += 4;
        writed(buf, (U32)(shm->dtime / 1000000)); buf += 4;
        writed(buf, (U32)(shm->ctime / 1000000)); buf += 4;
        writed(buf, shm->cpid); buf += 4;
        writed(buf, shm->lpid); buf += 4;
        writew(buf, shm->nattch); buf += 2;
        writew(buf, 0); buf += 2;
        writed(buf, 0);
    }  else if (cmd == (IPC_64 | IPC_RMID)) {
        shm->markedForDelete = 1;
    } else {
        kpanic("Unknown syscall_shmctl cmd=%X", cmd);
    }
    return 0;
}

#define K_RLIM_INFINITY 0xFFFFFFFF

U32 KSystem::prlimit64(U32 pid, U32 resource, U32 newlimit, U32 oldlimit) {
    std::shared_ptr<KProcess> process;

    if (pid==0) {
        process = KThread::currentThread()->process;
    } else {
        BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(processesCond);
        process = processes[pid];
        if (!process)
            return -K_ESRCH;
    }
    switch (resource) {
        case 0: // RLIMIT_CPU
            if (oldlimit!=0) {
                writeq(oldlimit, 0x7FFFFFFF);
                writeq(oldlimit + 8, 0x7FFFFFFF);
            }
#ifdef _DEBUG
            if (newlimit!=0) {
                klog("prlimit64 RLIMIT_CPU set=%d ignored", (U32)readq(newlimit));
            }
#endif
            break;
        case 1: // RLIMIT_FSIZE
            if (oldlimit!=0) {
                writeq(oldlimit, 0x800000000);
                writeq(oldlimit + 8, 0x800000000);
            }
#ifdef _DEBUG
            if (newlimit!=0) {
                klog("prlimit64 RLIMIT_FSIZE set=%d ignored", (U32)readq(newlimit));
            }
#endif
            break;
        case 2: // RLIMIT_DATA
            if (oldlimit!=0) {
                writeq(oldlimit, MAX_DATA_SIZE);
                writeq(oldlimit + 8, MAX_DATA_SIZE);
            }
#ifdef _DEBUG
            if (newlimit!=0) {
                klog("prlimit64 RLIMIT_DATA set=%d ignored", (U32)readq(newlimit));
            }
#endif
            break;
        case 3: // RLIMIT_STACK
            if (oldlimit!=0) {
                writeq(oldlimit, MAX_STACK_SIZE);
                writeq(oldlimit + 8, MAX_STACK_SIZE);
            }
#ifdef _DEBUG
            if (newlimit!=0) {
                klog("prlimit64 RLIMIT_STACK set=%d ignored", (U32)readq(newlimit));
            }
#endif
            break;
        case 4: // RLIMIT_CORE
            if (oldlimit!=0) {
                writeq(oldlimit, K_RLIM_INFINITY);
                writeq(oldlimit + 8, K_RLIM_INFINITY);
            }
#ifdef _DEBUG
            if (newlimit!=0) {
                klog("prlimit64 RLIMIT_CORE set=%d ignored", (U32)readq(newlimit));
            }
#endif
            break;
        case 5: // RLIMIT_RSS
            if (oldlimit!=0) {
                writeq(oldlimit, MAX_DATA_SIZE);
                writeq(oldlimit + 8, MAX_DATA_SIZE);
            }
#ifdef _DEBUG
            if (newlimit!=0) {
                klog("prlimit64 RLIMIT_RSS set=%d ignored", (U32)readq(newlimit));
            }
#endif
            break;
        case 6: // RLIMIT_NPROC
            if (oldlimit!=0) {
                writeq(oldlimit, 4096);
                writeq(oldlimit + 8, 4096);
            }
#ifdef _DEBUG
            if (newlimit!=0) {
                klog("prlimit64 RLIMIT_NPROC set=%d ignored", (U32)readq(newlimit));
            }
#endif
            break;
        case 7: // RLIMIT_NOFILE
            if (oldlimit!=0) {
                writeq(oldlimit, 16*1024); // some apps might iterate all the possible file handles, so don't make this too big
                writeq(oldlimit + 8, 16*1024);
            }
#ifdef _DEBUG
            if (newlimit!=0) {
                klog("prlimit64 RLIMIT_NOFILE set=%d ignored", (U32)readq(newlimit));
            }
#endif
            break;
        case 9: // RLIMIT_AS
            if (oldlimit!=0) {
                writeq(oldlimit, K_RLIM_INFINITY);
                writeq(oldlimit + 8, K_RLIM_INFINITY);
            }
#ifdef _DEBUG
            if (newlimit!=0) {
                klog("prlimit64 RLIMIT_AS set=%d ignored", (U32)readq(newlimit));
            }
#endif
            break;
        case 15: // RLIMIT_RTTIME
            if (oldlimit!=0) {
                writeq(oldlimit, 200);
                writeq(oldlimit + 8, 200);
            }
#ifdef _DEBUG
            if (newlimit!=0) {
                klog("prlimit64 RLIMIT_AS set=%d ignored", (U32)readq(newlimit));
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
    if (KSystem::processes.count(id))
        return KSystem::processes[id];
    return NULL;
}

#ifdef BOXEDWINE_DEFAULT_MMU
void KSystem::eraseFileCache(const std::string& name) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(KSystem::fileCacheMutex);
    KSystem::fileCache.erase(name);
}

BoxedPtr<MappedFileCache> KSystem::getFileCache(const std::string& name) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(KSystem::fileCacheMutex);
    if (KSystem::fileCache.count(name))
        return KSystem::fileCache[name];
    return NULL;
}

void KSystem::setFileCache(const std::string& name, const BoxedPtr<MappedFileCache>& fileCache) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(KSystem::fileCacheMutex);
    KSystem::fileCache[name] = fileCache;
}
#endif
void KSystem::eraseProcess(U32 id) {
    BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(processesCond);
    KSystem::processes.erase(id);
}

void KSystem::addProcess(U32 id, const std::shared_ptr<KProcess>& process) {
    BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(processesCond);
    KSystem::processes[id] = process;
}

U32 KSystem::getRunningProcessCount() {
    BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(processesCond);
    U32 count = 0;
    for (auto& n : KSystem::processes) {
        const std::shared_ptr<KProcess>& openProcess = n.second;
        if (openProcess && !openProcess->isStopped() && !openProcess->isTerminated()) {
            count++;
        }
    }
    return count;
}

U32 KSystem::getMilliesSinceStart() {
    if (!KSystem::adjustClock) {
        return SDL_GetTicks();
    }
    U32 currentTicks = SDL_GetTicks();
    U32 diff = currentTicks - KSystem::startTimeSdlTicks;
    U32 adjustedDiff = diff * KSystem::adjustClockFactor / 100;
    return KSystem::startTimeSdlTicks + adjustedDiff;
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