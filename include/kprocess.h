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

#ifndef __KPROCESS_H__
#define __KPROCESS_H__

#define ADDRESS_PROCESS_MMAP_START		0xD0000
#define ADDRESS_PROCESS_NATIVE          0xE0000
#define ADDRESS_PROCESS_LOADER			0xF0000
#define ADDRESS_PROCESS_STACK_START		0xF4000
#define ADDRESS_PROCESS_FRAME_BUFFER	0xF8000
#define ADDRESS_PROCESS_FRAME_BUFFER_ADDRESS 0xF8000000

class MappedFileCache;

class MappedFile : public BoxedPtrBase {
public:
    BoxedPtr<MappedFileCache> systemCacheEntry;
    std::shared_ptr<KFile> file;
    U32 address;
    U64 len;
    U64 offset;
};

#define K_SIG_INFO_SIZE 10

class KSigAction {
public:
    U32 handlerAndSigAction;
    U64 mask;
    U32 flags;
    U32 restorer;
    U32 sigInfo[K_SIG_INFO_SIZE];

    void writeSigAction(KMemory* memory, U32 address, U32 sigsetSize);
    void readSigAction(KMemory* memory, U32 address, U32 sigsetSize);
};

#define MAX_SIG_ACTIONS 64

#define K_PROT_NONE  0x0
#define K_PROT_READ  0x01
#define K_PROT_WRITE 0x02
#define K_PROT_EXEC  0x04

#define K_MAP_SHARED 0x01
#define K_MAP_PRIVATE 0x02
#define K_MAP_FIXED 0x10
#define K_MAP_ANONYMOUS 0x20
#define K_MAP_FIXED_NOREPLACE 0x100000

#define K_MADV_DONTNEED 4

class KProcessTimer : public KTimer { 
public:
    bool run();
private:
    friend class KProcess;
    std::weak_ptr<KProcess> process;
};

class AttachedSHM : public BoxedPtrBase {
public:
    AttachedSHM(const BoxedPtr<SHM>& shm, U32 address, U32 pid) : shm(shm), address(address), pid(pid) {
        this->shm->atime = KSystem::getSystemTimeAsMicroSeconds();
        this->shm->lpid = pid;
        this->shm->incAttach();
    }
    ~AttachedSHM() {
        this->shm->dtime = KSystem::getSystemTimeAsMicroSeconds();
        this->shm->lpid = pid;
        this->shm->decAttach();
    }
    BoxedPtr<SHM> shm;
    const U32 address;
    const U32 pid;
};

class KProcess : public std::enable_shared_from_this<KProcess> {
public:
    static std::shared_ptr<KProcess> create();    
    KProcess(U32 id);
    ~KProcess();

    KThread* createThread();
    void removeThread(KThread* thread);
    KThread* getThreadById(U32 tid);
    U32 getThreadCount();
	void deleteThread(KThread* thread);
    void killAllThreads(KThread* exceptThisOne = nullptr);
    BString getAbsoluteExePath();
    void clone(const std::shared_ptr<KProcess>& from);
    U32 getNextFileDescriptorHandle(int after);

    BString getModuleName(U32 eip);
    U32 getModuleEip(U32 eip);    
    KFileDescriptor* allocFileDescriptor(const std::shared_ptr<KObject>& kobject, U32 accessFlags, U32 descriptorFlags, S32 handle, U32 afterHandle);
    KFileDescriptor* getFileDescriptor(FD handle);
    void clearFdHandle(FD handle);
    U32 openFile(BString currentDirectory, BString localPath, U32 accessFlags, KFileDescriptor** result);
    bool isStopped();
    bool isTerminated();
    KThread* startProcess(BString currentDirectory, const std::vector<BString>& args, const std::vector<BString>& envValues, int userId, int groupId, int effectiveUserId, int effectiveGroupId);
    void signalProcess(U32 signal);
    void signalIO(U32 code, S32 band, FD fd);
    void signalCHLD(U32 code, U32 childPid, U32 sendingUID, S32 exitCode);
    void signalALRM();
    void printStack();
    U32 signal(U32 signal);
    void signalFd(KThread* thread, U32 signal);
    bool isSystemProcess() {return this->systemProcess;}

    void iterateThreads(std::function<bool(KThread*)> callback);
    void iterateThreadIds(std::function<bool(U32)> callback);

    // syscalls    
    U32 access(BString path, U32 mode);
    U32 alarm(U32 seconds);    
    U32 brk(KThread* thread, U32 address);
    U32 chdir(BString path);
    U32 chmod(BString path, U32 mode);
    U32 clone(KThread* thread, U32 flags, U32 child_stack, U32 ptid, U32 tls, U32 ctid);
    U32 close(FD fildes);
    U32 dup(U32 fildes);    
    U32 dup2(FD fildes, FD fildes2);
    U32 epollcreate(U32 size, U32 flags);
    U32 epollctl(FD epfd, U32 op, FD fd, U32 address);
    U32 epollwait(KThread* thread, FD epfd, U32 events, U32 maxevents, U32 timeout);
    U32 execve(KThread* thread, BString path, std::vector<BString>& args, const std::vector<BString>& envs);
    U32 exit(KThread* thread, U32 code);
    U32 exitgroup(KThread* thread, U32 code);
    U32 faccessat(U32 dirfd, BString path, U32 mode, U32 flags);
    U32 fchdir(FD fildes);
    U32 fcntrl(KThread* thread, FD fildes, U32 cmd, U32 arg);
    U32 fstat64(FD handle, U32 buf);
    U32 fstatat64(FD dirfd, BString path, U32 buf, U32 flag);
    U32 fstatfs64(FD fildes, U32 address);    
    U32 ftruncate64(FD fildes, U64 length);
    U32 getcwd(U32 buffer, U32 size);
    U32 getdents(FD fildes, U32 dirp, U32 count, bool is64);
    U32 getrusuage(KThread* thread, U32 who, U32 usage);
    U32 ioctl(KThread* thread, FD fildes, U32 request);
    U32 link(BString from, BString to);
    S64 llseek(FD fildes, S64 offset, U32 whence);
    U32 lseek(FD fd, S32 offset, U32 whence);
    U32 lstat64(BString path, U32 buffer);
    U32 mkdir(BString path);    
    U32 mkdirat(U32 dirfd, BString path, U32 mode);
    U32 mincore(U32 address, U32 length, U32 vec);    
    U32 msync(U32 addr, U32 len, U32 flags);
    U32 open(BString path, U32 flags);
    U32 openat(FD dirfd, BString path, U32 flags);
    U32 prctl(U32 option, U32 arg2);
    U32 pread64(KThread* thread, FD fildes, U32 address, U32 len, U64 offset);
    U32 pwrite64(KThread* thread, FD fildes, U32 address, U32 len, U64 offset);
    U32 read(KThread* thread, FD fildes, U32 bufferAddress, U32 bufferLen);
    U32 readlink(BString path, U32 buffer, U32 bufSize);
    U32 readlinkat(FD dirfd, BString path, U32 buf, U32 bufsiz);
    U32 rename(BString from, BString to);
    U32 renameat(FD olddirfd, BString from, FD newdirfd, BString to);
    U32 rmdir(BString path);        
    U32 set_thread_area(KThread* thread, U32 info);
    U32 setitimer(U32 which, U32 newValue, U32 oldValue);
    U32 shmdt(U32 shmaddr);
    U32 sigaction(U32 sig, U32 act, U32 oact, U32 sigsetSize);    
    U32 stat64(BString path, U32 buffer);
    U32 statfs(BString path, U32 address);
    U32 statfs64(BString path, U32 address);
    U32 symlink(BString target, BString linkpath);
    U32 symlinkat(BString, FD dirfd, BString linkpath);
    U32 umask(U32 umask);
    U32 unlinkFile(BString path);
    U32 unlinkat(FD dirfd, BString path, U32 flags);    
    U32 utimes(BString path, U32 times);
    U32 utimesat(FD dirfd, BString path, U32 times, U32 flags);
    U32 utimesat64(FD dirfd, BString path, U32 times, U32 flags);
    U32 write(KThread* thread, FD fildes, U32 bufferAddress, U32 bufferLen);
    U32 writev(KThread* thread, FD handle, U32 iov, S32 iovcnt);
    U32 memfd_create(BString name, U32 flags);

    user_desc* getLDT(U32 index);
    BoxedPtr<SHM> allocSHM(U32 key, U32 afterIndex);
    BoxedPtr<SHM> getSHM(U32 key);
    void attachSHM(U32 address, const BoxedPtr<SHM>& shm);
    void printMappedFiles();

    U32 id;
    U32 parentId;
    U32 groupId;
    U32 userId;
    U32 effectiveUserId;
    U32 effectiveGroupId;
    U64 pendingSignals;
    BOXEDWINE_MUTEX pendingSignalsMutex;
    U32 signaled;
    U32 exitCode;
    U32 umaskValue;
    bool terminated;
    KMemory* memory;

    BString currentDirectory;
    U32 brkEnd;    
    KSigAction sigActions[MAX_SIG_ACTIONS];
    KProcessTimer timer;
    BString commandLine;
    BString exe;
    BString name;
    std::vector<BString> path;        
    KThread* waitingThread;
    U32 loaderBaseAddress;
    U32 phdr;
    U32 phnum;
    U32 phentsize;
    U32 entry;
    U32 eventQueueFD;     
    BOXEDWINE_CONDITION exitOrExecCond;

    bool hasSetStackMask;
    bool hasSetSeg[6];

    std::unordered_map<U32, U32> glStrings;    
    U32 glStringsiExtensions;
    std::vector<U32> glStringsiExtensionsOffset;
    U32 numberOfExtensions;
#ifdef BOXEDWINE_BINARY_TRANSLATOR
    bool emulateFPU;
    void* reTranslateChunkAddress; // will be called when the program tries to jump to memory that hasn't been translated yet or needs to be retranslated
    void* syncToHostAddress;
    void* syncFromHostAddress;
    void* doSingleOpAddress;
#ifdef BOXEDWINE_64BIT_MMU
    void* reTranslateChunkAddressFromReg; // will be called when the program tries to jump to memory that hasn't been translated yet or needs to be retranslated
#endif
    void* returnToLoopAddress; // will be called after a syscall if x64CPU.exitToStartThreadLoop is set to true.  This return will cause the program to return to x64CPU::run()
#ifdef BOXEDWINE_BT_DEBUG_NO_EXCEPTIONS
    void* jmpAndTranslateIfNecessary;
#endif
#ifdef BOXEDWINE_POSIX
    void* runSignalAddress;
#endif
#endif
private:
    std::unordered_map<U32, KFileDescriptor*> fds;
    BOXEDWINE_MUTEX fdsMutex;

    std::unordered_map<U32, user_desc> ldt;
    BOXEDWINE_MUTEX ldtMutex;

    std::unordered_map<U32, BoxedPtr<SHM> > privateShm; // key is shmid
    BOXEDWINE_MUTEX privateShmMutex;

    std::unordered_map<U32, BoxedPtr<AttachedSHM> > attachedShm; // key is attached address
    BOXEDWINE_MUTEX attachedShmMutex;

    friend class KMemory;
    std::unordered_map<U32, BoxedPtr<MappedFile> > mappedFiles; // key is address
    BOXEDWINE_MUTEX mappedFilesMutex;

    std::unordered_map<U32, KThread*> threads;
    BOXEDWINE_MUTEX threadsMutex;
public:
    BOXEDWINE_CONDITION threadRemovedCondition; // will signal when a thread is removed
private:

    U32 usedTLS[TLS_ENTRIES];
    BOXEDWINE_MUTEX usedTlsMutex;

    U32 openFileDescriptor(BString currentDirectory, BString localPath, U32 accessFlags, U32 descriptorFlags, S32 handle, U32 afterHandle, KFileDescriptor** result);
    void cleanupProcess();
    void setupCommandlineNode();
    void initStdio();
    BoxedPtr<FsNode> findInPath(BString path);
    U32 readlinkInDirectory(BString currentDirectory, BString path, U32 buffer, U32 bufSize);
    void onExec(KThread* thread);
    U32 getCurrentDirectoryFromDirFD(FD dirfd, BString& currentDirectory);

    BoxedPtr<FsNode> commandLineNode;
    BoxedPtr<FsNode> procNode;
    bool systemProcess;
};

#endif
