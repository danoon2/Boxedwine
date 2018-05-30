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
class Memory;

class MappedFile : public BoxedPtrBase {
public:
    BoxedPtr<MappedFileCache> systemCacheEntry;
    BoxedPtr<KFile> file;
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

    void writeSigAction(U32 address, U32 sigsetSize);
    void readSigAction(U32 address, U32 sigsetSize);
};

#define NUMBER_OF_STRINGS 5
#define STRING_GL_VENDOR 0
#define STRING_GL_RENDERER 1
#define STRING_GL_VERSION 2
#define STRING_GL_SHADING_LANGUAGE_VERSION 3
#define STRING_GL_EXTENSIONS 4

#define MAX_SIG_ACTIONS 64

#define K_PROT_NONE  0x0
#define K_PROT_READ  0x01
#define K_PROT_WRITE 0x02
#define K_PROT_EXEC  0x04

#define K_MAP_SHARED 0x01
#define K_MAP_PRIVATE 0x02
#define K_MAP_FIXED 0x10
#define K_MAP_ANONYMOUS 0x20
#define K_MAP_TRY_FIXED 0x4000

#define K_MADV_DONTNEED 4

class KProcessTimer : public KTimer { 
public:
    KProcessTimer(KProcess* process) : process(process){};
    bool run();
private:
    KProcess* process;
};

class AttachedSHM : public BoxedPtrBase {
public:
    AttachedSHM(const BoxedPtr<SHM>& shm, U32 address, U32 pid) : shm(shm), address(address), pid(pid) {
        this->shm->atime = Platform::getSystemTimeAsMicroSeconds();
        this->shm->lpid = pid;
        this->shm->incAttach();
    }
    ~AttachedSHM() {
        this->shm->dtime = Platform::getSystemTimeAsMicroSeconds();
        this->shm->lpid = pid;
        this->shm->decAttach();
    }
    const U32 pid;
    const U32 address;
    BoxedPtr<SHM> shm;
};

class KProcess {
public:
    KProcess(U32 id);
    ~KProcess();

    KThread* createThread();
    void removeThread(KThread* thread);
    KThread* getThreadById(U32 tid);
    U32 getThreadCount();

    void clone(KProcess* from);
    U32 getNextFileDescriptorHandle(int after);

    std::string getModuleName(U32 eip);
    U32 getModuleEip(U32 eip);    
    KFileDescriptor* allocFileDescriptor(const BoxedPtr<KObject>& kobject, U32 accessFlags, U32 descriptorFlags, S32 handle, U32 afterHandle);
    KFileDescriptor* getFileDescriptor(FD handle);
    void clearFdHandle(FD handle);
    KFileDescriptor* openFile(std::string const &currentDirectory, std::string const &localPath, U32 accessFlags);       
    bool isStopped();
    bool isTerminated();
    KThread* startProcess(const std::string& currentDirectory, U32 argc, const char** args, U32 envc, const char** env, int userId, int groupId, int effectiveUserId, int effectiveGroupId);
    void signalProcess(U32 signal);
    void signalIO(U32 code, S32 band, FD fd);
    void signalCHLD(U32 code, U32 childPid, U32 sendingUID, S32 exitCode);
    void signalALRM();

    // syscalls    
    U32 access(const std::string& path, U32 mode);
    U32 alarm(U32 seconds);    
    U32 brk(U32 address);
    U32 chdir(const std::string& path);
    U32 chmod(const std::string& path, U32 mode);
    U32 clone(U32 flags, U32 child_stack, U32 ptid, U32 tls, U32 ctid);
    U32 close(FD fildes);
    U32 dup(U32 fildes);    
    U32 dup2(FD fildes, FD fildes2);
    U32 epollcreate(U32 size, U32 flags);
    U32 epollctl(FD epfd, U32 op, FD fd, U32 address);
    U32 epollwait(FD epfd, U32 events, U32 maxevents, U32 timeout);
    U32 execve(const std::string& path, std::vector<std::string>& args, const std::vector<std::string>& envs);
    U32 exit(U32 code);
    U32 exitgroup(U32 code);
    U32 faccessat(U32 dirfd, const std::string& path, U32 mode, U32 flags);
    U32 fchdir(FD fildes);
    U32 fcntrl(FD fildes, U32 cmd, U32 arg);
    U32 fstat64(FD handle, U32 buf);
    U32 fstatat64(FD dirfd, const std::string& path, U32 buf, U32 flag);
    U32 fstatfs64(FD fildes, U32 address);    
    U32 ftruncate64(FD fildes, U64 length);
    U32 getcwd(U32 buffer, U32 size);
    U32 getdents(FD fildes, U32 dirp, U32 count, bool is64);
    U32 getrusuage(U32 who, U32 usage);
    U32 ioctl(FD fildes, U32 request);
    U32 link(const std::string& from, const std::string& to);
    S64 llseek(FD fildes, S64 offset, U32 whence);
    U32 lseek(FD fd, S32 offset, U32 whence);
    U32 lstat64(const std::string& path, U32 buffer);
    U32 mkdir(const std::string& path);    
    U32 mkdirat(U32 dirfd, const std::string& path, U32 mode);
    U32 mincore(U32 address, U32 length, U32 vec);
    U32 mlock(U32 addr, U32 len);
    U32 mmap(U32 addr, U32 len, S32 prot, S32 flags, FD fildes, U64 off);
    U32 mprotect(U32 address, U32 len, U32 prot);
    U32 mremap(U32 oldaddress, U32 oldsize, U32 newsize, U32 flags);
    U32 msync(U32 addr, U32 len, U32 flags);
    U32 open(const std::string& path, U32 flags);
    U32 openat(FD dirfd, const std::string& path, U32 flags);
    U32 prctl(U32 option, U32 arg2);
    U32 pread64(FD fildes, U32 address, U32 len, U64 offset);
    U32 pwrite64(FD fildes, U32 address, U32 len, U64 offset);
    U32 read(FD fildes, U32 bufferAddress, U32 bufferLen);
    U32 readlink(const std::string& path, U32 buffer, U32 bufSize);
    U32 readlinkat(FD dirfd, const std::string& path, U32 buf, U32 bufsiz);
    U32 rename(const std::string& from, const std::string& to);
    U32 rmdir(const std::string& path);        
    U32 set_thread_area(U32 info);
    U32 setitimer(U32 which, U32 newValue, U32 oldValue);
    U32 shmdt(U32 shmaddr);
    U32 sigaction(U32 sig, U32 act, U32 oact, U32 sigsetSize);    
    U32 stat64(const std::string& path, U32 buffer);
    U32 statfs(const std::string& path, U32 address);
    U32 statfs64(const std::string& path, U32 address);
    U32 symlink(const std::string& target, const std::string& linkpath);
    U32 symlinkat(const std::string&, FD dirfd, const std::string& linkpath);
    U32 umask(U32 umask);
    U32 unlinkFile(const std::string& path);
    U32 unlinkat(FD dirfd, const std::string& path, U32 flags);
    U32 unmap(U32 address, U32 len);
    U32 utimes(const std::string& path, U32 times);
    U32 utimesat(FD dirfd, const std::string& path, U32 times, U32 flags);
    U32 write(FD fildes, U32 bufferAddress, U32 bufferLen);
    U32 writev(FD handle, U32 iov, S32 iovcnt);

    user_desc* getLDT(U32 index);
    const std::unordered_map<U32, KThread*>& getThreads();
    BoxedPtr<SHM> allocSHM(U32 key, U32 afterIndex);
    BoxedPtr<SHM> getSHM(U32 key);
    void attachSHM(U32 address, const BoxedPtr<SHM>& shm);
    const std::unordered_map<U32, BoxedPtr<MappedFile>>& getMappedFiles();

    U32 id {};
    U32 parentId {};
    U32 groupId {};
    U32 userId {};
    U32 effectiveUserId {};
    U32 effectiveGroupId {};
    U64 pendingSignals {};
    U32 signaled {};
    U32 exitCode {};
    U32 umaskValue {};
    bool terminated {};
    Memory* memory {};     
    std::string currentDirectory;
    U32 brkEnd {};    
    KSigAction sigActions[MAX_SIG_ACTIONS];
    KProcessTimer timer;
    std::string commandLine;
    std::string exe;
    std::string name;
    std::vector<std::string> path;        
    KThread* waitingThread {};
    U32 loaderBaseAddress {};
    U32 phdr {};
    U32 phnum {};
    U32 phentsize {};
    U32 entry {};
    U32 eventQueueFD {};    
    U32 usedTLS[TLS_ENTRIES];
    KThread* wakeOnExitOrExec {};
private:
    std::unordered_map<U32, KFileDescriptor*> fds;
    std::unordered_map<U32, user_desc> ldt;
    std::unordered_map<U32, BoxedPtr<SHM>> privateShm; // key is shmid
    std::unordered_map<U32, BoxedPtr<AttachedSHM>> attachedShm; // key is attached address
    std::unordered_map<U32, BoxedPtr<MappedFile>> mappedFiles; // key is address
    std::unordered_map<U32, KThread*> threads;

    KFileDescriptor* openFileDescriptor(const std::string& currentDirectory, const std::string& localPath, U32 accessFlags, U32 descriptorFlags, S32 handle, U32 afterHandle);
    void cleanupProcess();
    void setupCommandlineNode();
    void initStdio();
    BoxedPtr<FsNode> findInPath(const std::string& path);
    U32 readlinkInDirectory(const std::string& currentDirectory, const std::string& path, U32 buffer, U32 bufSize);
    void onExec();
    U32 getCurrentDirectoryFromDirFD(FD dirfd, std::string& currentDirectory);

    BoxedPtr<FsNode> commandLineNode;
    BoxedPtr<FsNode> procNode;
};

#endif