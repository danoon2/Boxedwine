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

#include "kscheduler.h"
#include "loader.h"
#include "kstat.h"
#include "bufferaccess.h"
#include "ksignal.h"
#include "kepoll.h"
#include "../io/fsmemnode.h"
#include "../io/fsmemopennode.h"
#include "../io/fsfilenode.h"

#ifdef BOXEDWINE_BINARY_TRANSLATOR
#include "../emulation/cpu/binaryTranslation/btCodeMemoryWrite.h"
#endif

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h> 

#define MAX_ARG_COUNT 1024

bool KProcessTimer::run() {
    bool result = false;
    if (this->resetMillies==0) {
        result = true;
        this->millies = 0;
    } else {
        this->millies = this->resetMillies + KSystem::getMilliesSinceStart();
    }
    std::shared_ptr<KProcess> p = this->process.lock();
    if (p) {
        p->signalALRM();
    }
    return result;
}

std::shared_ptr<KProcess> KProcess::create() {
    std::shared_ptr<KProcess> process = std::make_shared<KProcess>(KSystem::getNextThreadId());
    KSystem::addProcess(process->id, process);
    process->timer.process = process; // can't use shared_from_this in constructor
    return process;
}

KProcess::KProcess(U32 id) : id(id), 
    parentId(0),
    groupId(0),
    userId(0),
    effectiveUserId(0),
    effectiveGroupId(0),
    pendingSignals(0),
    signaled(0),
    exitCode(0),
    umaskValue(0x1a4), // 0644
    terminated(false),
    memory(NULL),
    brkEnd(0), 
    waitingThread(NULL),
    loaderBaseAddress(0),
    phdr(0),
    phnum(0),
    phentsize(0),
    entry(0),
    eventQueueFD(0),
    exitOrExecCond("KProcess::exitOrExecCond"),
    hasSetStackMask(false),
    threadsCondition("KProcess::threadsCond"),
    systemProcess(false) {

#ifdef BOXEDWINE_BINARY_TRANSLATOR
    emulateFPU=false;
    returnToLoopAddress = NULL;
    reTranslateChunkAddress = NULL;
    reTranslateChunkAddressFromReg = NULL;
#ifdef BOXEDWINE_BT_DEBUG_NO_EXCEPTIONS
    jmpAndTranslateIfNecessary = NULL;
#endif
#ifdef BOXEDWINE_POSIX
    runSignalAddress = NULL;
#endif
#endif
    for (int i=0;i<LDT_ENTRIES;i++) {
        this->ldt[i].seg_not_present = 1;
        this->ldt[i].read_exec_only = 1;
    }
    memset(this->usedTLS, 0, sizeof(this->usedTLS));

    this->ldt[1].base_addr = 0;
    this->ldt[1].entry_number = 1;
    this->ldt[1].seg_32bit = 1;
    this->ldt[1].seg_not_present = 0;
    this->ldt[1].read_exec_only = 0;

    this->ldt[2].base_addr = 0;
    this->ldt[2].entry_number = 2;
    this->ldt[2].seg_32bit = 1;
    this->ldt[2].seg_not_present = 0;
    this->ldt[2].read_exec_only = 0;

    memset(this->sigActions, 0, sizeof(KSigAction)*MAX_SIG_ACTIONS);

    for (int i=0;i<6;i++) {
        this->hasSetSeg[i] = false;
    }
    this->hasSetSeg[GS] = true;
    this->hasSetSeg[FS] = true;    

#ifdef BOXEDWINE_64BIT_MMU
    this->nextNativeAddress = ADDRESS_PROCESS_NATIVE;  
	this->previousMemory = NULL;
#endif
    this->glStringsiExtensions = 0;
    this->numberOfExtensions = 0;
}

void KProcess::onExec() {
    std::unordered_map<U32, KFileDescriptor*> fdsToClose = this->fds; // make a copy since we can't remove from it while iterating
    for( const auto& n : fdsToClose ) {
        KFileDescriptor* fd = n.second;
        if (fd->descriptorFlags) {
            fd->refCount = 1; // make sure it is really closed
            fd->close();
        }
    }
    this->attachedShm.clear();
    this->privateShm.clear();
    this->mappedFiles.clear();

    memset(this->sigActions, 0, sizeof(KSigAction)*MAX_SIG_ACTIONS);

    if (this->timer.active) {
        removeTimer(&this->timer);
    }

    std::vector<KThread*> toDelete;
    for (auto& n : this->threads) {
        KThread* thread = n.second;
        if (thread!=KThread::currentThread()) {
            toDelete.push_back(thread);
        }
    }
    for (auto& thread : toDelete) {
        terminateOtherThread(shared_from_this(), thread->id);
    }
    this->threads.clear();
    this->threads[KThread::currentThread()->id] = KThread::currentThread();

    this->initStdio();
    
    for (int i=0;i<LDT_ENTRIES;i++) {
        this->ldt[i].seg_not_present = 1;
        this->ldt[i].read_exec_only = 1;
    }
    memset(this->usedTLS, 0, sizeof(this->usedTLS));

    this->ldt[1].base_addr = 0;
    this->ldt[1].entry_number = 1;
    this->ldt[1].seg_32bit = 1;
    this->ldt[1].seg_not_present = 0;
    this->ldt[1].read_exec_only = 0;

    this->ldt[2].base_addr = 0;
    this->ldt[2].entry_number = 2;
    this->ldt[2].seg_32bit = 1;
    this->ldt[2].seg_not_present = 0;
    this->ldt[2].read_exec_only = 0;

    for (int i=0;i<6;i++) {
        this->hasSetSeg[i] = false;
    }
    this->hasSetSeg[GS] = true;
    this->hasSetSeg[FS] = true;
    this->hasSetStackMask = false;

#ifdef BOXEDWINE_BINARY_TRANSLATOR
    returnToLoopAddress = NULL;
    reTranslateChunkAddress = NULL;
    reTranslateChunkAddressFromReg = NULL;
#ifdef BOXEDWINE_BT_DEBUG_NO_EXCEPTIONS
    jmpAndTranslateIfNecessary = NULL;
#endif
#ifdef BOXEDWINE_POSIX
    runSignalAddress = NULL;
#endif
#endif
}

KProcess::~KProcess() {
    killAllThreadsExceptCurrent();
    this->cleanupProcess();
	if (this->memory) {
		this->memory->decRefCount();
	}
}

void KProcess::cleanupProcess() {    
    removeTimer(&this->timer);

    std::unordered_map<U32, KFileDescriptor*> fdsToClose = this->fds; // make a copy since we can't remove from it while iterating
    for( const auto& n : fdsToClose ) {
        KFileDescriptor* fd = n.second;
        fd->refCount = 1; // make sure it is really closed
        fd->close();
    }
    this->attachedShm.clear();
    this->privateShm.clear();
    this->mappedFiles.clear();    
}

KThread* KProcess::createThread() {
	BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(threadsCondition);
    KThread* thread = new KThread(KSystem::getNextThreadId(), shared_from_this());
    this->threads[thread->id] = thread;
    return thread;
}

void KProcess::removeThread(KThread* thread) {
	BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(threadsCondition);
	BOXEDWINE_CONDITION_SIGNAL(threadsCondition);
    this->threads.erase(thread->id);
}

KThread* KProcess::getThreadById(U32 tid) {
	BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(threadsCondition);
    if (this->threads.count(tid))
        return this->threads[tid];
    return NULL;
}

U32 KProcess::getThreadCount() {
	BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(threadsCondition);
    return (U32)this->threads.size();
}

void KProcess::deleteThread(KThread* thread) {
    {
        BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(threadsCondition);
        thread->cleanup();
        if (this->threads.size() == 0) {
            if (this->memory) {
                this->memory->decRefCount();
                this->memory = NULL;
            }
        }
        delete thread;
    }
    // don't call into getProcess while holding threadsCondition
    if (!this->terminated && this->getThreadCount() == 0) {
        std::shared_ptr<KProcess> parent = KSystem::getProcess(this->parentId);
        if (!parent || parent->getThreadCount() == 0) {
            KSystem::eraseProcess(this->id);
        }
    }
}

FsOpenNode* openCommandLine(const BoxedPtr<FsNode>& node, U32 flags, U32 data) {
    return new BufferAccess(node, flags, KThread::currentThread()->process->commandLine);
}

void KProcess::setupCommandlineNode() {
    // :TODO: this will replace the previous one if it exists and leak memory 
    if (!this->procNode) {
        BoxedPtr<FsNode> proc = Fs::getNodeFromLocalPath("", "/proc", true);
        this->procNode = Fs::addFileNode(std::string("/proc/")+std::to_string(this->id), "", "", true, proc);
    }
    this->commandLineNode = Fs::addVirtualFile(std::string("/proc/")+std::to_string(this->id)+std::string("/cmdline"), openCommandLine, K__S_IREAD, 0, this->procNode);
    std::string exePath = std::string("/proc/") + std::to_string(this->id) + std::string("/exe");
    BoxedPtr<FsNode> exeNode = Fs::getNodeFromLocalPath("", exePath, true);
    if (!exeNode) {
        U32 id = this->id;
        Fs::addDynamicLinkFile(exePath, 0, this->procNode, false, [id] {
            std::shared_ptr<KProcess> p = KSystem::getProcess(id);
            if (p) {
                return Fs::getNodeFromLocalPath("", p->exe, true)->path;
            }
            return std::string("(deleted)");
            });
    }
}

std::string KProcess::getAbsoluteExePath() { 
    std::string path = Fs::getNodeFromLocalPath("", this->exe, true)->path;
    return Fs::getParentPath(path);
}
void KProcess::clone(const std::shared_ptr<KProcess>& from) {
    U32 i;

    this->parentId = from->id;;
    this->groupId = from->groupId;
    this->userId = from->userId;
    this->effectiveUserId = from->effectiveUserId;
    this->effectiveGroupId = from->effectiveGroupId;
    this->currentDirectory = from->currentDirectory;
    this->brkEnd = from->brkEnd;
    for (auto& n : from->fds) {
        KFileDescriptor* fd = n.second;
        this->fds[n.first] = this->allocFileDescriptor(fd->kobject, fd->accessFlags, fd->descriptorFlags, n.first, 0);
        this->fds[n.first]->refCount = fd->refCount;
    }
    // :TODO: not thread safe if from has multiple threads
    this->mappedFiles = from->mappedFiles;
    std::copy(from->sigActions, from->sigActions+MAX_SIG_ACTIONS, this->sigActions);
    this->path = from->path;
    this->commandLine = from->commandLine;
    this->exe = from->exe;
    this->name = from->name;
    this->setupCommandlineNode();
    this->umaskValue = from->umaskValue;

    this->privateShm = from->privateShm;

    for (auto& n : from->attachedShm) {
        BoxedPtr<AttachedSHM> attached = new AttachedSHM(n.second->shm, n.first, this->id);
        this->attachedShm[n.first] = attached;
    }

    for (i=0;i<LDT_ENTRIES;i++) {
        this->ldt[i] = from->ldt[i];
    }
    this->ldt = from->ldt;
    this->loaderBaseAddress = from->loaderBaseAddress;
    this->phdr = from->phdr;
    this->phnum = from->phnum;
    this->entry = from->entry;

    for (i=0;i<6;i++) {
        this->hasSetSeg[i] = from->hasSetSeg[i];
    }
    this->hasSetStackMask = from->hasSetStackMask;
    this->systemProcess = from->systemProcess;
}

static void writeStackString(KThread* thread, CPU * cpu, const char* s) {
    int count = (int)((strlen(s)+4)/4);
    int i;
    for (i=0;i<count;i++) {
        cpu->push32(0);
    }
    writeNativeString(ESP, s);
}

#define HWCAP_I386_FPU   1 << 0
#define HWCAP_I386_VME   1 << 1
#define HWCAP_I386_DE    1 << 2
#define HWCAP_I386_PSE   1 << 3
#define HWCAP_I386_TSC   1 << 4
#define HWCAP_I386_MSR   1 << 5
#define HWCAP_I386_PAE   1 << 6
#define HWCAP_I386_MCE   1 << 7
#define HWCAP_I386_CX8   1 << 8
#define HWCAP_I386_APIC  1 << 9
#define HWCAP_I386_SEP   1 << 11
#define HWCAP_I386_MTRR  1 << 12
#define HWCAP_I386_PGE   1 << 13
#define HWCAP_I386_MCA   1 << 14
#define HWCAP_I386_CMOV  1 << 15
#define HWCAP_I386_FCMOV 1 << 16
#define HWCAP_I386_MMX   1 << 23
#define HWCAP_I386_OSFXSR 1 << 24
#define HWCAP_I386_XMM   1 << 25
#define HWCAP_I386_XMM2  1 << 26
#define HWCAP_I386_AMD3D 1 << 31

static void pushThreadStack(KThread* thread, CPU* cpu, int argc, U32* a, int envc, U32* e) {
    int i;
    std::shared_ptr<KProcess> process = cpu->thread->process;
    U32 randomAddress;
    U32 platform;

    cpu->push32(rand());
    cpu->push32(rand());
    cpu->push32(rand());
    cpu->push32(rand());
    randomAddress = ESP;
    cpu->push32(0);
    cpu->push32(0);
    writeStackString(thread, cpu, "i686");
    platform = ESP;

    cpu->push32(0);	
    cpu->push32(0);	
    
    // end of auxv
    cpu->push32(0);	
    cpu->push32(0);		
    

    cpu->push32(randomAddress);
    cpu->push32(25); // AT_RANDOM
    cpu->push32(100);
    cpu->push32( 17); // AT_CLKTCK
    //push32(cpu, HWCAP_I386_FPU|HWCAP_I386_VME|HWCAP_I386_TSC|HWCAP_I386_CX8|HWCAP_I386_CMOV|HWCAP_I386_FCMOV);
    //push32(cpu, 16); // AT_HWCAP
    cpu->push32(platform);
    cpu->push32(15); // AT_PLATFORM
    cpu->push32(process->effectiveGroupId);
    cpu->push32(14); // AT_EGID
    cpu->push32(process->groupId);
    cpu->push32(13); // AT_GID
    cpu->push32(process->effectiveUserId);
    cpu->push32(12); // AT_EUID
    cpu->push32(process->userId);
    cpu->push32(11); // AT_UID
    cpu->push32(process->entry);
    cpu->push32(9); // AT_ENTRY
    cpu->push32(process->loaderBaseAddress); 
    cpu->push32(7); // AT_BASE
    cpu->push32(4096);
    cpu->push32(6); // AT_PAGESZ
    cpu->push32(process->phnum);
    cpu->push32(5); // AT_PHNUM
    cpu->push32(process->phentsize);
    cpu->push32(4); // AT_PHENT
    cpu->push32(process->phdr);
    cpu->push32(3); // AT_PHDR

    // /libgdk-pixbuf2.0 package on buster needed this
    cpu->push32(0);
    cpu->push32(23); // AT_SECURE

    cpu->push32(0);	
    for (i=envc-1;i>=0;i--) {
        cpu->push32(e[i]);
    }
    cpu->push32(0);
    for (i=argc-1;i>=0;i--) {
        cpu->push32(a[i]);
    }
    cpu->push32(argc);
}

static void setupThreadStack(KThread* thread, CPU* cpu, const std::string& programName, const std::vector<std::string>& args, const std::vector<std::string>& env) {
    U32 a[MAX_ARG_COUNT];
    U32 e[MAX_ARG_COUNT];
    int i;

    cpu->push32(0);
    cpu->push32(0);
    cpu->push32(0);	
    writeStackString(thread, cpu, programName.c_str());
    if (args.size()>MAX_ARG_COUNT)
        kpanic("Too many args: %d is max", MAX_ARG_COUNT);
    if (env.size()>MAX_ARG_COUNT)
        kpanic("Too many env: %d is max", MAX_ARG_COUNT);
    //klog("env");
    for (i=0;i<(int)env.size();i++) {
        writeStackString(thread, cpu, env[i].c_str());
        if (strncmp(env[i].c_str(), "PATH=", 5)==0) {
            stringSplit(cpu->thread->process->path,env[i].substr(5),':');
        }
        //klog("    %s", env[i]);
        e[i]=ESP;
    }
    for (i=(int)args.size()-1;i>=0;i--) {
        writeStackString(thread, cpu, args[i].c_str());
        a[i]=ESP;
    }

    pushThreadStack(thread, cpu, (U32)args.size(), a, (U32)env.size(), e);
}

U32 KProcess::getNextFileDescriptorHandle(int after) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(fdsMutex);
    U32 i=after;

    while (1) {
		if (!this->fds[i])
            return i;
        i++;
    }
}

KFileDescriptor* KProcess::allocFileDescriptor(const std::shared_ptr<KObject>& kobject, U32 accessFlags, U32 descriptorFlags, S32 handle, U32 afterHandle) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(fdsMutex);
    KFileDescriptor* result;

    if (handle<0) {
        handle = this->getNextFileDescriptorHandle(afterHandle);
    }
    result = new KFileDescriptor(shared_from_this(), kobject, accessFlags, descriptorFlags, handle);

    KFileDescriptor* old = this->getFileDescriptor(handle);
    if (old)
        old->close();  
    this->fds[handle] = result;
    return result;
}

U32 translateOpenError() {
    switch (errno) {
    case EACCES: return -K_EACCES;
    case EEXIST: return -K_EEXIST;
    case ENOENT: return -K_ENOENT;
    case EISDIR: return -K_EISDIR;
    }
    return -K_EINVAL;
}

U32 KProcess::openFileDescriptor(const std::string& currentDirectory, std::string localPath, U32 accessFlags, U32 descriptorFlags, S32 handle, U32 afterHandle, KFileDescriptor** result) {
    BoxedPtr<FsNode> node;
    FsOpenNode* openNode;
    std::shared_ptr<KObject> kobject;

    node = Fs::getNodeFromLocalPath(currentDirectory, localPath, true);
    if (!node && (accessFlags & (K_O_CREAT|K_O_TMPFILE))==0) {
        return -K_ENOENT;
    }
    if (accessFlags & K_O_TMPFILE) {
        if ((accessFlags & K_O_ACCMODE)!=K_O_WRONLY && (accessFlags & K_O_ACCMODE)!=K_O_RDWR) {
            return -K_EINVAL;
        }
        if (!node || !node->isDirectory()) {
            return -K_ENOENT;
        }
        accessFlags&= ~K_O_TMPFILE;
        accessFlags|=K_O_CREAT;
        localPath = FsFileNode::getLocalTmpPath();
        node = Fs::getNodeFromLocalPath(currentDirectory, localPath, true);
    } else if (node && node->isDirectory()) {
        if ((accessFlags & K_O_ACCMODE)!=K_O_RDONLY) {
            return -K_EISDIR;
        }
    }
    if (!node) {
        std::string fullPath = Fs::getFullPath(currentDirectory, localPath);
        std::string parentPath = Fs::getParentPath(fullPath);
        std::string fileName = Fs::getFileNameFromPath(fullPath);
        BoxedPtr<FsNode> parent = Fs::getNodeFromLocalPath("", parentPath, true);
        if (!parent) {
            return -K_ENOENT;
        }       
        std::string nativePath = Fs::getNativePathFromParentAndLocalFilename(parent, fileName);
        BoxedPtr<FsNode> mixedSibling = parent->getChildByNameIgnoreCase(fileName);
        if (mixedSibling) {
            nativePath = nativePath + ".mixed";
            if (Fs::doesNativePathExist(nativePath)) {
                kwarn("KProcess::openFileDescriptor mixed file already exists");
            }
        }
        node = Fs::addFileNode(parent->path+"/"+fileName, "", nativePath, false, parent);
        Fs::makeLocalDirs(parent->path);
    }
    std::shared_ptr<KObject> nodeObject = node->kobject.lock();
    if (nodeObject) {
        kobject = nodeObject;
    } else {
        openNode = node->open(accessFlags);
        if (!openNode) {
            node->removeNodeFromParent();
            return translateOpenError();
        }
        openNode->openedPath = Fs::getFullPath(currentDirectory, localPath);
        kobject = std::make_shared<KFile>(openNode);
    }
    KFileDescriptor* f = this->allocFileDescriptor(kobject, accessFlags, descriptorFlags, handle, afterHandle);
    if (result) {
        *result = f;
    }
    return 0;
}

U32 KProcess::openFile(const std::string& currentDirectory, const std::string& localPath, U32 accessFlags, KFileDescriptor** result) {
    return this->openFileDescriptor( currentDirectory, localPath, accessFlags, (accessFlags & K_O_CLOEXEC)?FD_CLOEXEC:0, -1, 0, result);
}

void KProcess::initStdio() {
    if (!this->getFileDescriptor(0)) {
        this->openFileDescriptor("", "/dev/tty0", K_O_RDONLY, 0, 0, 0, NULL);
    }
    if (!this->getFileDescriptor(1)) {
        this->openFileDescriptor("", "/dev/tty0", K_O_WRONLY, 0, 1, 0, NULL);
    }
    if (!this->getFileDescriptor(2)) {
        this->openFileDescriptor("", "/dev/tty0", K_O_WRONLY, 0, 2, 0, NULL);
    }
}

KThread* KProcess::startProcess(const std::string& currentDirectory, const std::vector<std::string>& argValues, const std::vector<std::string>& envValues, int userId, int groupId, int effectiveUserId, int effectiveGroupId) {
    BoxedPtr<FsNode> node = Fs::getNodeFromLocalPath(currentDirectory, argValues[0], true);

    if (!node) {
        kwarn("Could not find %s", argValues[0].c_str());
        return 0;
    }

    std::string interpreter;
    std::string loader;
    std::vector<std::string> interpreterArgs;
    std::vector<std::string> args;
    std::vector<std::string> env;
    this->memory = new Memory();		
    KThread* thread = this->createThread();
    U32 i;
    FsOpenNode* openNode = NULL;
    std::string name;

    thread->setupStack();
    this->parentId = 1;
    this->userId = userId;
    this->effectiveUserId = effectiveUserId;
    this->groupId = groupId;	
    this->effectiveGroupId = effectiveGroupId;
    this->setupCommandlineNode();
    this->exe = argValues[0];
    this->name = Fs::getFileNameFromPath(this->exe);
    
    for (i=0;i<argValues.size();i++) {
        if (i>0)
            this->commandLine+=" ";
        this->commandLine+=argValues[i];
    }

    KThread::setCurrentThread(thread);
    this->initStdio();
    openNode=ElfLoader::inspectNode(currentDirectory, node, loader, interpreter, interpreterArgs);
    if (!openNode) {
        return 0;
    }    
    if (ElfLoader::loadProgram(shared_from_this(), openNode, &thread->cpu->eip.u32)) {
        // :TODO: why will it crash in strchr libc if I remove this
        //syscall_mmap64(thread, ADDRESS_PROCESS_LOADER << PAGE_SHIFT, 4096, K_PROT_READ | K_PROT_WRITE, K_MAP_ANONYMOUS|K_MAP_PRIVATE, -1, 0);
        
        if (loader.length())
            args.push_back(loader);
        if (interpreter.length())
            args.push_back(interpreter);        
        args.push_back(std::string(Fs::getFullPath(currentDirectory, argValues[0])));
        for (i=1;i<argValues.size();i++) {
            args.push_back(argValues[i]);
        }
        for (i=0;i<envValues.size();i++) {
            env.push_back(envValues[i]);
        }
        setupThreadStack(thread, thread->cpu, this->name, args, env);

        this->currentDirectory = currentDirectory;

        if (openNode) {
            openNode->close();
            delete openNode;
        }
        scheduleThread(thread);
    } else {
        if (openNode) {
            openNode->close();
            delete openNode;
        }
    }
    return thread;
}

U32 KProcess::exit(U32 code) {
    this->exitCode = code;
    if (this->getThreadCount()==1)
        return this->exitgroup(code);    

    KThread::currentThread()->cpu->yield = true;
    KThread::currentThread()->cleanup(); // will unschedule the thread
	terminateCurrentThread(KThread::currentThread());

    return 0;
}

KFileDescriptor* KProcess::getFileDescriptor(FD handle) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(fdsMutex);
    if (this->fds.count(handle))
        return this->fds[handle];
    return NULL;
}

void KProcess::clearFdHandle(FD handle) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(fdsMutex);
    this->fds.erase(handle);
}

bool KProcess::isStopped() {
    return false;
}

bool KProcess::isTerminated() {
    return this->terminated;
}

std::string KProcess::getModuleName(U32 eip) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(mappedFilesMutex);
    for (auto& n : this->mappedFiles) {
        BoxedPtr<MappedFile> mappedFile = n.second;
        if (eip >= mappedFile->address && eip < mappedFile->address + mappedFile->len) {
            return mappedFile->file->openFile->node->name;
        }
    }
    return "Unknown";
}

U32 KProcess::getModuleEip(U32 eip) {    
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(mappedFilesMutex);
    if (eip<0xd0000000)
        return eip;
    for (auto& n : this->mappedFiles) {
        BoxedPtr<MappedFile> mappedFile = n.second;
        if (eip>=mappedFile->address && eip<mappedFile->address+mappedFile->len)
            return (U32)(eip-mappedFile->address+mappedFile->offset);
    }
    return 0;
}

U32 KProcess::alarm(U32 seconds) {
    U32 prev = this->timer.millies;
    if (seconds == 0) {
        if (this->timer.millies!=0) {
            removeTimer(&this->timer);
            this->timer.millies = 0;
        }
    } else {
        this->timer.resetMillies = 0;
        if (this->timer.millies!=0) {
            this->timer.millies = seconds*1000 + KSystem::getMilliesSinceStart();
        } else {
            this->timer.millies = seconds*1000 + KSystem::getMilliesSinceStart();
            addTimer(&this->timer);
        }
    }
    if (prev) {
        return (prev - KSystem::getMilliesSinceStart())/1000;
    }
    return 0;
}

BoxedPtr<FsNode> KProcess::findInPath(const std::string& path) {
    BoxedPtr<FsNode> node = Fs::getNodeFromLocalPath(this->currentDirectory, path, true);

    if (!node && !stringStartsWith(path, "/")) {
        for( const auto& n : this->path ) {
            node = Fs::getNodeFromLocalPath(n, path, true);
            if (node)
                break;
        }
    }
    return node;
}

U32 KProcess::execve(const std::string& path, std::vector<std::string>& args, const std::vector<std::string>& envs) {
    BoxedPtr<FsNode> node;
    FsOpenNode* openNode = 0;
    std::string interpreter;
    std::vector<std::string> interpreterArgs;
    std::string loader;
    std::string name;
    std::vector<std::string> cmdLine;

    this->systemProcess = false;
    if (stringHasEnding(args[args.size() - 1], "wineserver", true)) {
        Platform::setCurrentThreadPriorityHigh();
        this->systemProcess = true;
    } else {
        for (auto& s : args) {
            if (stringHasEnding(s, "wineboot.exe", true)) {
                this->systemProcess = true;
            } else if (stringHasEnding(s, "winemenubuilder.exe", true)) {
                this->systemProcess = true;
            } else if (stringHasEnding(s, "services.exe", true)) {
                this->systemProcess = true;
            } else if (stringHasEnding(s, "plugplay.exe", true)) {
                this->systemProcess = true;
            } else if (stringHasEnding(s, "winedevice.exe", true)) {
                this->systemProcess = true;
            } else if (stringHasEnding(s, "explorer.exe", true)) {
                this->systemProcess = true;
            }
        }
    }
    node = this->findInPath(path);
    if (!node) {
        return 0;
    }
    openNode = ElfLoader::inspectNode(this->currentDirectory, node, loader, interpreter, interpreterArgs);
    if (!openNode) {
        return 0;
    }
#ifdef BOXEDWINE_MULTI_THREADED
    if (KSystem::cpuAffinityCountForApp) {
        Platform::setCpuAffinityForThread(KThread::currentThread(), this->isSystemProcess()?0:KSystem::cpuAffinityCountForApp);
    }
#endif
    args[0] = std::string(Fs::getFullPath(currentDirectory, path)); // if path is a link, we should use the link not the actual path       
    if (interpreter.length()) {
        args.insert(args.begin(), interpreterArgs.begin(), interpreterArgs.end());
        args.insert(args.begin(), interpreter);
    }
    if (loader.length()) {
        args.insert(args.begin(), loader);
    }
    this->exe = path; // if path is a link, we should use the link not the actual path       
    this->name = Fs::getFileNameFromPath(path);
    
    this->commandLine = stringJoin(args, "\0");
            
    this->path.clear();
    for (U32 i=0;i<envs.size();i++) {
        if (strncmp(envs[i].c_str(), "PATH=", 5)==0) {
            stringSplit(this->path,envs[i].substr(5),':');
        }
    }

    // reset memory must come after we grab the args and env
#ifdef BOXEDWINE_BINARY_TRANSLATOR
	this->previousMemory = this->memory;
	this->memory = new Memory();
	this->memory->onThreadChanged();
	KThread::currentThread()->memory = this->memory;
#else
	if (this->memory->getRefCount() == 1) {
		this->memory->reset();
	}
	else {
		this->memory->decRefCount();
		this->memory = new Memory();
		this->memory->onThreadChanged();
		KThread::currentThread()->memory = this->memory;
	}
#endif    

    KThread::currentThread()->reset();
    this->onExec();

    // not sure why x64 doesn't catch setting the CS segment this in time
    if (stringContainsIgnoreCase(this->name, "winevdm.exe") || vectorContainsIgnoreCase(args, "winevdm.exe")) {
        for (int i=0;i<6;i++) {
            this->hasSetSeg[i] = true;
            this->hasSetStackMask = true;
        }
    }

    if (!ElfLoader::loadProgram(shared_from_this(), openNode, &KThread::currentThread()->cpu->eip.u32)) {
        // :TODO: maybe alloc a new memory object and keep the old one until we know we are loaded
        kpanic("program failed to load, but memory was already reset");
    }	
    // must come after loadProgram because of process->phdr
    setupThreadStack(KThread::currentThread(), KThread::currentThread()->cpu, this->name, args, envs);
    openNode->close();
    delete openNode;

    BOXEDWINE_CONDITION_LOCK(this->exitOrExecCond);
    BOXEDWINE_CONDITION_SIGNAL_ALL(this->exitOrExecCond);
    BOXEDWINE_CONDITION_UNLOCK(this->exitOrExecCond);

    //klog("%d/%d exec %s (cwd=%s)", KThread::currentThread()->id, this->id, this->commandLine.c_str(), this->currentDirectory.c_str());

    KThread::currentThread()->cpu->restart();
    
    return 1;
}

void KProcess::signalProcess(U32 signal) {	
    U64 signalMask = ((U64)1 << (signal-1));
    {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(this->pendingSignalsMutex);
        this->pendingSignals |= signalMask;
    }
	BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(threadsCondition);
    // give each thread a chance to run a signal, some or all of them might have the signal masked off.  
    // In that case when the user unmasks the signal with sigprocmask it will be caught then
#ifdef BOXEDWINE_MULTI_THREADED
    for (auto& t : this->threads) {
        KThread* thread = t.second;
        if (thread->waitingCond) {
            BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(thread->waitingCondSync);
            if (thread->waitingCond) {
                thread->runSignals();
            }
        }
    }
#else
    for (auto& t : this->threads) {
        KThread* thread = t.second;
        thread->runSignals();
    }
#endif
    {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(this->pendingSignalsMutex);
        if (this->pendingSignals & signalMask) {
            this->signalFd(NULL, signal);
        }
    }
}

void KProcess::signalIO(U32 code, S32 band, FD fd) {
    memset(this->sigActions[K_SIGIO].sigInfo, 0, sizeof(this->sigActions[K_SIGIO].sigInfo));
    this->sigActions[K_SIGIO].sigInfo[0] = K_SIGIO;
    this->sigActions[K_SIGIO].sigInfo[2] = code;
    this->sigActions[K_SIGIO].sigInfo[3] = band;
    this->sigActions[K_SIGIO].sigInfo[4] = fd;
    signalProcess(K_SIGIO);
}

void KProcess::signalCHLD(U32 code, U32 childPid, U32 sendingUID, S32 exitCode) {
    memset(this->sigActions[K_SIGCHLD].sigInfo, 0, sizeof(this->sigActions[K_SIGCHLD].sigInfo));
    this->sigActions[K_SIGCHLD].sigInfo[0] = K_SIGCHLD;
    this->sigActions[K_SIGCHLD].sigInfo[2] = code;
    this->sigActions[K_SIGCHLD].sigInfo[3] = childPid;
    this->sigActions[K_SIGCHLD].sigInfo[4] = sendingUID;
    this->sigActions[K_SIGCHLD].sigInfo[5] = exitCode;
    // not sure why this causes crashes
    //signalProcess(K_SIGCHLD);
}

void KProcess::signalALRM() {
    memset(this->sigActions[K_SIGALRM].sigInfo, 0, sizeof(this->sigActions[K_SIGALRM].sigInfo));
    this->sigActions[K_SIGALRM].sigInfo[0] = K_SIGALRM;
    this->sigActions[K_SIGALRM].sigInfo[2] = K_SI_USER;
    this->sigActions[K_SIGALRM].sigInfo[3] = this->id;
    this->sigActions[K_SIGALRM].sigInfo[4] = this->userId;
    signalProcess(K_SIGALRM);
}

U32 KProcess::dup(U32 fildes) {
    KFileDescriptor* fd = this->getFileDescriptor(fildes);

    if (!fd) {
        return -K_EBADF;
    }
    return this->allocFileDescriptor(fd->kobject, fd->accessFlags, 0, -1, 0)->handle; // do not copy file descriptor flags
}

U32 KProcess::rmdir(const std::string& path) {
    BoxedPtr<FsNode> node = Fs::getNodeFromLocalPath(this->currentDirectory, path, false);

    if (!node)
        return -K_ENOENT;
    return node->removeDir();
}

U32 KProcess::mkdir(const std::string& path) {
    BoxedPtr<FsNode> node = Fs::getNodeFromLocalPath(this->currentDirectory, path, false);   
    if (node) {
        return -K_EEXIST;
    }
    std::string fullpath = Fs::getFullPath(this->currentDirectory, path);
    std::string parentPath = Fs::getParentPath(fullpath);
    node = Fs::getNodeFromLocalPath("", parentPath, false); 
    if (!node) {
        return -K_ENOENT;
    }
    return Fs::makeLocalDirs(fullpath);
}

U32 KProcess::rename(const std::string& from, const std::string& to) {
    BoxedPtr<FsNode> node = Fs::getNodeFromLocalPath(this->currentDirectory, from, false);    
    if (!node)
        return -K_ENOENT;
    std::string fullPath = Fs::getFullPath(this->currentDirectory, to);
    return node->rename(fullPath);
}

U32 KProcess::renameat(FD olddirfd, const std::string& from, FD newdirfd, const std::string& to) {
    std::string currentDirectory;
    U32 result = this->getCurrentDirectoryFromDirFD(olddirfd, currentDirectory);
    if (result)
        return result;

    BoxedPtr<FsNode> node = Fs::getNodeFromLocalPath(currentDirectory, from, false);    
    if (!node)
        return -K_ENOENT;
    result = this->getCurrentDirectoryFromDirFD(newdirfd, currentDirectory);
    if (result)
        return result;
    std::string fullPath = Fs::getFullPath(currentDirectory, to);
    return node->rename(fullPath);
}

static S32 internalAccess(BoxedPtr<FsNode> node, U32 flags) {
    if (!node) {
        return -K_ENOENT;
    }
    if (flags==0)
        return 0;
    if ((flags & 4)!=0) {
        if (!node->canRead()) {
            return -K_EACCES;
        }
    }
    if ((flags & 2)!=0) {
        if (!node->canWrite()) {
            return -K_EACCES;
        }
    }
    if ((flags & 1)!=0) {
        kdebug("access not fully implemented.  Can't test for executable permission");
    }
    return 0;
}

U32 KProcess::access(const std::string& path, U32 mode) {
    BoxedPtr<FsNode> node = Fs::getNodeFromLocalPath(this->currentDirectory, path, true);
    return internalAccess(node, mode); 
}

U32 KProcess::lseek(FD fildes, S32 offset, U32 whence) {
    KFileDescriptor* fd = this->getFileDescriptor(fildes);
    S64 pos;

    if (!fd) {
        return -K_EBADF;
    }
    if (whence == 0) {
        pos = offset;
    } else if (whence == 1) {
        pos = offset + fd->kobject->getPos();
    } else if (whence == 2) {
        pos = fd->kobject->length() + offset;
    } else {
        return -K_EINVAL;
    }
    return (U32)fd->kobject->seek(pos);
}

U32 KProcess::chmod(const std::string& path, U32 mode) {
    BoxedPtr<FsNode> node = Fs::getNodeFromLocalPath(this->currentDirectory, path, true);
    if (!node)
        return -K_ENOENT;
    return 0;
}

U32 KProcess::chdir(const std::string& path) {
    BoxedPtr<FsNode> node = Fs::getNodeFromLocalPath(this->currentDirectory, path, true);
    if (!node)
        return -K_ENOENT;
    if (!node->isDirectory())
        return -K_ENOTDIR;
    if (stringStartsWith(path, "/")) {
        this->currentDirectory = path;
    } else {
        this->currentDirectory = this->currentDirectory+"/"+path;
    }
    if (stringHasEnding(this->currentDirectory, "/")) {
        this->currentDirectory = this->currentDirectory.substr(0, this->currentDirectory.length()-1);
    }
    return 0;
}

U32 KProcess::unlinkFile(const std::string& path) {
    BoxedPtr<FsNode> node = Fs::getNodeFromLocalPath(this->currentDirectory, path, false);
    if (!node) {
        return -K_ENOENT;
    }
    if (!node->remove()) {
        kwarn("failed to remove file: errno=%d", errno);
        return -K_EBUSY;
    }
    return 0;
}

U32 KProcess::link(const std::string& from, const std::string& to) {
    BoxedPtr<FsNode> fromNode = Fs::getNodeFromLocalPath(this->currentDirectory, from, false);
    BoxedPtr<FsNode> toNode = Fs::getNodeFromLocalPath(this->currentDirectory, to, false);

    if (!fromNode) {
        return -K_ENOENT;
    }

    if (toNode) {
        return -K_EEXIST;
    }
    if (fromNode->isDirectory()) {
        return -K_EPERM;
    }

    FsOpenNode* fromOpenNode = fromNode->open(K_O_RDONLY);
    if (!fromOpenNode)
        return -K_EIO;

    BoxedPtr<FsNode> toParentNode = Fs::getNodeFromLocalPath(this->currentDirectory, Fs::getParentPath(to), false);
    if (!toParentNode)
        return -K_ENOENT;

    toNode = Fs::addFileNode(to, "", Fs::getNativePathFromParentAndLocalFilename(toParentNode, Fs::getFileNameFromPath(to)), false, toParentNode);
    FsOpenNode* toOpenNode = toNode->open(K_O_WRONLY|K_O_CREAT);
    if (!toOpenNode) {
        fromOpenNode->close();
        return -K_EIO;
    }

    while (1) {
        U8 buffer[K_PAGE_SIZE];
        U32 r = fromOpenNode->readNative(buffer, K_PAGE_SIZE);	
        toOpenNode->writeNative(buffer, r);
        if (r<K_PAGE_SIZE)
            break;
    }
    toOpenNode->close();
    fromOpenNode->close();
    toNode->hardLinkCount++;
    fromNode->hardLinkCount++;
    kdebug("Hard link not implemented");
    return 0;
}

U32 KProcess::close(FD fildes) {
    KFileDescriptor* fd = this->getFileDescriptor(fildes);
    if (!fd) {
        return -K_EBADF;
    }
    fd->close();
    return 0;
}

U32 KProcess::open(const std::string& path, U32 flags) {
    KFileDescriptor* fd = NULL;
        
    U32 result = this->openFile(this->currentDirectory, path, flags, &fd);
    if (result || !fd) {
        return result;
    }
    return fd->handle;
}

U32 KProcess::read(FD fildes, U32 bufferAddress, U32 bufferLen) {
    KFileDescriptor* fd = this->getFileDescriptor(fildes);
    if (!fd) {
        return -K_EBADF;
    }
    if (!fd->canRead()) {
        return -K_EINVAL;
    }
#ifdef BOXEDWINE_BINARY_TRANSLATOR
    BtCodeMemoryWrite w((BtCPU*)KThread::currentThread()->cpu, bufferAddress, bufferLen);
#endif
    return fd->kobject->read(bufferAddress, bufferLen);
}

U32 KProcess::write(FD fildes, U32 bufferAddress, U32 bufferLen) {
    KFileDescriptor* fd = this->getFileDescriptor(fildes);
    if (!fd) {
        return -K_EBADF;
    }
    if (!fd->canWrite()) {
        return -K_EINVAL;
    }
    return fd->kobject->write(bufferAddress, bufferLen);
}

U32 KProcess::brk(U32 address) {    
    // :TODO: why is this 1MB limit required, without it debian stretch "dpkg -i /var/local/python-crypto_2.6.1-7_i386.deb" will fail
    if (address!=0 && address-this->loaderBaseAddress>0x00100000) {
        return this->brkEnd;
    }
    if (address > this->brkEnd) {
        U32 len = address-this->brkEnd;
        U32 alreadyAllocated = K_ROUND_UP_TO_PAGE(this->brkEnd) - this->brkEnd;

        if (len<=alreadyAllocated) {
            this->brkEnd+=len;
        } else {
            U32 aligned = (this->brkEnd+4095) & 0xFFFFF000;
            U32 startPage = aligned >> K_PAGE_SHIFT;
            U32 stopPage = startPage+((len-alreadyAllocated+K_PAGE_SIZE-1)>>K_PAGE_SHIFT);

            for (U32 i=startPage;i<=stopPage;i++) {
                if (this->memory->isPageAllocated(i)) {
                    return -K_ENOMEM;
                }
            }
            if (this->mmap(aligned, len - alreadyAllocated, K_PROT_READ | K_PROT_WRITE | K_PROT_EXEC, K_MAP_PRIVATE|K_MAP_ANONYMOUS|K_MAP_FIXED, -1, 0)==aligned) {
                this->brkEnd+=len;
            }				
        }
    } else if (address!=0 && address < this->brkEnd) {
        U32 startAddress = (address+4095) & 0xFFFFF000;
        U32 len = this->brkEnd - startAddress;
        this->unmap(startAddress, len);
        this->brkEnd = startAddress;
    }
    return this->brkEnd; // intentional, should return the new end, even though the libc brk returns the old value
}

U32 KProcess::ioctl(FD fildes, U32 request) {
    KFileDescriptor* fd = this->getFileDescriptor(fildes);

    if (fd==0) {
        return -K_EBADF;
    }
    return fd->kobject->ioctl(request);
}

U32 KProcess::umask(U32 umask) {	
    U32 old = this->umaskValue;
    this->umaskValue = umask;
    return old;
}

U32 KProcess::dup2(FD fildes, FD fildes2) {
    KFileDescriptor* fd = this->getFileDescriptor(fildes);
    KFileDescriptor* fd2;

    if (!fd || fildes2<0) {
        return -K_EBADF;
    }
    if (fildes == fildes2) {
        return fildes;
    }
    fd2 = this->getFileDescriptor(fildes2);
    if (fd2) {
        if (fd2->refCount>1) {
            kpanic("Not sure what to do on a dup2 where the refcount is %d", fd2->refCount);
        }
        fd2->close();
    } 
    this->allocFileDescriptor(fd->kobject, fd->accessFlags, 0, fildes2, 0); // do not copy file descriptor flags
    return fildes2;
}

U32 KProcess::getrusuage(U32 who, U32 usage) {    
    U32 userSeconds = 0;
    U32 userMicroSeconds = 0;
    U32 kernelSeconds = 0;
    U32 kernelMicroSeconds = 0;

    if (who==0) { // RUSAGE_SELF
        BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(threadsCondition);
        for (auto& t : this->threads ) {
            KThread* thread = t.second;
            userSeconds += (U32)(thread->userTime / 1000000l);
            userMicroSeconds += (U32)(thread->userTime % 1000000l);
            kernelSeconds += (U32)(thread->kernelTime / 1000000l);
            kernelMicroSeconds += (U32)(thread->kernelTime % 1000000l);
        }
        
    } else if ((S32)who < 0) { // RUSAGE_CHILDREN
        klog("getrusuage: RUSAGE_CHILDREN not implemented");
    } else { // RUSAGE_THREAD
        KThread* thread = KThread::currentThread();
        userSeconds = (U32)(thread->userTime / 1000000l);
        userMicroSeconds = (U32)(thread->userTime % 1000000l);
        kernelSeconds = (U32)(thread->kernelTime / 1000000l);
        kernelMicroSeconds = (U32)(thread->kernelTime % 1000000l);
    }

    // user time
    writed(usage, userSeconds);
    writed(usage + 4, userMicroSeconds);
    // system time
    writed(usage + 8, kernelSeconds);
    writed(usage + 12, kernelMicroSeconds);
    zeroMemory(usage+16, 56);
    return 0;
}

U32 symlinkInDirectory(const std::string& currentDirectory, const std::string& target, const std::string& linkpath) {
    BoxedPtr<FsNode> node;
    FsOpenNode* openNode;

    node = Fs::getNodeFromLocalPath(currentDirectory, linkpath, false);
    if (node) {
        return -K_EEXIST;
    }
    std::string fullPath = Fs::getFullPath(currentDirectory, linkpath);
    std::string parentPath = Fs::getParentPath(fullPath);
    BoxedPtr<FsNode> parentNode = Fs::getNodeFromLocalPath("", parentPath, true);

    if (!parentNode) {
        return -K_ENOENT;
    }
    node = Fs::addFileNode(fullPath, target, Fs::getNativePathFromParentAndLocalFilename(parentNode, Fs::getFileNameFromPath(linkpath))+".link", false, parentNode);

    if (!node->canWrite()) {
        node->removeNodeFromParent();
        return -K_EACCES;
    }
    openNode = node->open( K_O_WRONLY|K_O_CREAT);
    if (!openNode) {
        node->removeNodeFromParent();
        return -K_EIO;
    }
    openNode->writeNative((U8*)target.c_str(), (U32)target.length());
    openNode->close();
    return 0;
}

U32 KProcess::symlinkat(const std::string& target, FD dirfd, const std::string& linkpath) {
    std::string currentDirectory;
    U32 result = this->getCurrentDirectoryFromDirFD(dirfd, currentDirectory);
    if (result)
        return result;
    return symlinkInDirectory(currentDirectory, target, linkpath);
}

U32 KProcess::symlink(const std::string& target, const std::string& linkpath) {
    return symlinkInDirectory(this->currentDirectory, target, linkpath);
}

U32 KProcess::readlinkInDirectory(const std::string& currentDirectory, const std::string& path, U32 buffer, U32 bufSize) {
    // :TODO: move these to the virtual filesystem
    if (!strcmp(path.c_str(), "/proc/self/exe")) {
        const std::string& exe = KThread::currentThread()->process->exe;
        U32 len = (U32)exe.length();
        if (len>bufSize)
            len = bufSize;
        memcopyFromNative(buffer, exe.c_str(), len);
        return len;
    } else if (stringStartsWith(path, "/proc/self/fd/")) {
        FD h = atoi(path.c_str()+14);
        KFileDescriptor* fd = this->getFileDescriptor(h);

        if (!fd)
            return -K_EINVAL;
        if (fd->kobject->type!=KTYPE_FILE) {
            return -K_EINVAL;
        }
        std::shared_ptr<KFile> p = std::dynamic_pointer_cast<KFile>(fd->kobject);
        const std::string& fdpath =  p->openFile->node->path;
        U32 len = (U32)fdpath.length();
        if ((int)fdpath.length()>(int)bufSize)
            len=bufSize;
        memcopyFromNative(buffer, fdpath.c_str(), len);
        return len;        
    }

    BoxedPtr<FsNode> node = Fs::getNodeFromLocalPath(currentDirectory, path, false);
    if (!node || !node->isLink())
        return -K_EINVAL;
    U32 len = (U32)node->getLink().length();
    if (len>bufSize)
        len = bufSize;
    memcopyFromNative(buffer, node->getLink().c_str(), len);
    return len; 
}

U32 KProcess::readlinkat(FD dirfd, const std::string& path, U32 buf, U32 bufsiz) {
    std::string currentDirectory;
    U32 result = this->getCurrentDirectoryFromDirFD(dirfd, currentDirectory);
    if (result)
        return result;
    return readlinkInDirectory(currentDirectory, path, buf, bufsiz);
}

U32 KProcess::readlink(const std::string& path, U32 buffer, U32 bufSize) {
    return readlinkInDirectory(this->currentDirectory, path, buffer, bufSize);
}

U32 KProcess::mmap(U32 addr, U32 len, S32 prot, S32 flags, FD fildes, U64 off) {
    bool shared = (flags & K_MAP_SHARED)!=0;
    bool priv = (flags & K_MAP_PRIVATE)!=0;
    bool read = (prot & K_PROT_READ)!=0;
    bool write = (prot & K_PROT_WRITE)!=0;
    bool exec = (prot & K_PROT_EXEC)!=0;
    U32 pageStart = addr >> K_PAGE_SHIFT;
    U32 pageCount = (len+K_PAGE_SIZE-1)>>K_PAGE_SHIFT;
    KFileDescriptor* fd = 0;

    if (0xFFFFFFFF-addr<len) {
        return -K_EINVAL;
    }
    if ((shared && priv) || (!shared && !priv)) {
        return -K_EINVAL;
    }

    if (!(flags & K_MAP_ANONYMOUS) && fildes>=0) {
        fd = this->getFileDescriptor(fildes);
        if (!fd) {
            return -K_EBADF;
        }
        if (!fd->kobject->canMap()) {
            return -K_EACCES;
        }
        if (len==0 || (off & 0xFFF)!=0) {
            return -K_EINVAL;
        }
        if ((!fd->canRead() && read) || (!priv && (!fd->canWrite() && write))) {
            return -K_EACCES;
        }
    }        
    if (flags & K_MAP_FIXED) {
        if (addr & (K_PAGE_SIZE-1)) {
#ifdef _DEBUG
            klog("tried to call mmap with invalid address: %X", addr);
            return -K_EINVAL;
#endif
        }
    } else {		
        if (pageStart + pageCount> ADDRESS_PROCESS_MMAP_START)
            return -K_ENOMEM;
        if (pageStart == 0)
            pageStart = ADDRESS_PROCESS_MMAP_START;
        if (!this->memory->findFirstAvailablePage(pageStart, pageCount, &pageStart, addr!=0)) {
            // :TODO: what erro
            return -K_EINVAL;
        }
        if (addr!=0 && pageStart+pageCount> ADDRESS_PROCESS_MMAP_START)
            return -K_ENOMEM;
        addr = pageStart << K_PAGE_SHIFT;	
    }
    if (fd) {
        U32 result = fd->kobject->map(addr, len, prot, flags, off);
        if (result) {
            return result;
        }
    }

	// even if there are no permissions, it is important for MAP_ANONYMOUS|MAP_FIXED existing memory to be 0'd out
    // if (write || read || exec)
	{		
        U32 permissions = PAGE_MAPPED;

        if (write)
            permissions|=PAGE_WRITE;
        if (read)
            permissions|=PAGE_READ;
        if (exec)
            permissions|=PAGE_EXEC;
        if (shared)
            permissions|=PAGE_SHARED;
        if (fd) {	
            BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(mappedFilesMutex);
            BoxedPtr<MappedFile> mappedFile = new MappedFile();

            mappedFile->address = pageStart << K_PAGE_SHIFT;
            mappedFile->len = ((U64)pageCount) << K_PAGE_SHIFT;
            mappedFile->offset = off;     
            mappedFile->file = std::dynamic_pointer_cast<KFile>(fd->kobject);            
#ifdef BOXEDWINE_DEFAULT_MMU
            bool addFileToSystemCache = true;
#else
            bool addFileToSystemCache = shared;
#endif
            if (addFileToSystemCache) {
                BoxedPtr<MappedFileCache> cache = KSystem::getFileCache(mappedFile->file->openFile->node->path);
                if (!cache) {
                    cache = new MappedFileCache(mappedFile->file->openFile->node->path);
                    KSystem::setFileCache(mappedFile->file->openFile->node->path, cache);
                    cache->file = mappedFile->file;
#ifdef BOXEDWINE_DEFAULT_MMU
                    U32 size = ((U32)((fd->kobject->length() + K_PAGE_SIZE - 1) >> K_PAGE_SHIFT));
#else
                    U32 size = 1;
#endif
                    cache->data = new U8 * [size];
                    cache->dataSize = size;
                    memset(cache->data, 0, size * sizeof(U8*));
                }
                mappedFile->systemCacheEntry = cache;
            }
            KThread::currentThread()->process->mappedFiles[mappedFile->address] = mappedFile;
            this->memory->allocPages(pageStart, pageCount, permissions, fildes, off, mappedFile);
        } else {
            if (shared) {
                int ii = 0;
            }
            this->memory->allocPages(pageStart, pageCount, permissions, 0, 0, NULL);
        }		
    }
    return addr;
}

U32 KProcess::unmap(U32 address, U32 len) {
    U32 pageStart = address >> K_PAGE_SHIFT;
    U32 pageCount = (len+K_PAGE_SIZE-1)>>K_PAGE_SHIFT;
    
    this->memory->reset(pageStart, pageCount);
    return 0;
}

U32 KProcess::ftruncate64(FD fildes, U64 length) {
    KFileDescriptor* fd = this->getFileDescriptor(fildes);
    FsOpenNode* openNode;

    if (fd==0) {
        return -K_EBADF;
    }
    if (fd->kobject->type!=KTYPE_FILE) {
        return -K_EINVAL;
    }
    std::shared_ptr<KFile> p = std::dynamic_pointer_cast<KFile>(fd->kobject);
    openNode = p->openFile;
    if (openNode->node->isDirectory()) {
        return -K_EISDIR;
    }
    if (!fd->canWrite()) {
        return -K_EINVAL;
    }
    if (!openNode->setLength(length)) {
        return -K_EIO;
    }
    return 0;
}

#define FS_SIZE 107374182400l
#define FS_FREE_SIZE 96636764160l

U32 KProcess::statfs(const std::string& path, U32 address) {
    BoxedPtr<FsNode> node = Fs::getNodeFromLocalPath(this->currentDirectory, path, true);
    if (!node) {
        return -K_ENOENT;
    }
    writed(address, 0xEF53); // f_type (EXT3)
    writed(address + 4, FS_BLOCK_SIZE); // f_bsize
    writed(address + 8, FS_SIZE / FS_BLOCK_SIZE); // f_blocks
    writed(address + 16, FS_FREE_SIZE / FS_BLOCK_SIZE); // f_bfree
    writed(address + 24, FS_FREE_SIZE / FS_BLOCK_SIZE); // f_bavail
    writed(address + 32, 1024 * 1024); // f_files
    writed(address + 40, 1024 * 1024); // f_ffree
    writed(address + 48, 1278601602); // f_fsid
    writed(address + 56, MAX_FILEPATH_LEN); // f_namelen
    writed(address + 60, FS_BLOCK_SIZE); // f_frsize
    writed(address + 64, 4096); // f_flags
    return 0;
}

U32 KProcess::statfs64(const std::string& path, U32 address) {
    BoxedPtr<FsNode> node = Fs::getNodeFromLocalPath(this->currentDirectory, path, true);
    if (!node) {
        return -K_ENOENT;
    }
    writed(address, 0xEF53); // f_type (EXT3)
    writed(address + 4, FS_BLOCK_SIZE); // f_bsize
    writeq(address + 8, FS_SIZE / FS_BLOCK_SIZE); // f_blocks
    writeq(address + 16, FS_FREE_SIZE / FS_BLOCK_SIZE); // f_bfree
    writeq(address + 24, FS_FREE_SIZE / FS_BLOCK_SIZE); // f_bavail
    writeq(address + 32, 1024 * 1024); // f_files
    writeq(address + 40, 1024 * 1024); // f_ffree
    writeq(address + 48, 1278601602); // f_fsid
    writed(address + 56, MAX_FILEPATH_LEN); // f_namelen
    writed(address + 60, FS_BLOCK_SIZE); // f_frsize
    writed(address + 64, 4096); // f_flags
    return 0;
}

U32 KProcess::fstatfs64(FD fildes, U32 address) {
    KFileDescriptor* fd = this->getFileDescriptor(fildes);

    if (!fd) {
        return -K_EBADF;
    }
    writed(address, 0xEF53); // f_type (EXT3)
    writed(address + 4, FS_BLOCK_SIZE); // f_bsize
    writeq(address + 8, FS_SIZE / FS_BLOCK_SIZE); // f_blocks
    writeq(address + 16, FS_FREE_SIZE / FS_BLOCK_SIZE); // f_bfree
    writeq(address + 24, FS_FREE_SIZE / FS_BLOCK_SIZE); // f_bavail
    writeq(address + 32, 1024 * 1024); // f_files
    writeq(address + 40, 1024 * 1024); // f_ffree
    writeq(address + 48, 12719298601114463092ull); // f_fsid
    writed(address + 56, MAX_FILEPATH_LEN); // f_namelen
    writed(address + 60, FS_BLOCK_SIZE); // f_frsize
    writed(address + 64, 4096); // f_flags
    return 0;
}

U32 KProcess::setitimer(U32 which, U32 newValue, U32 oldValue) {

    if (which != 0) { // ITIMER_REAL
        kpanic("setitimer which=%d not supported", which);
    }
    if (oldValue) {
        U32 remaining = this->timer.millies - KSystem::getMilliesSinceStart();

        writed(oldValue, this->timer.resetMillies / 1000);
        writed(oldValue, (this->timer.resetMillies % 1000) * 1000);
        writed(oldValue + 8, remaining / 1000);
        writed(oldValue + 12, (remaining % 1000) * 1000);
    }
    if (newValue) {
        U32 millies = readd(newValue + 8) * 1000 + readd(newValue + 12) / 1000;
        U32 resetMillies = readd(newValue) * 1000 + readd(newValue + 4) / 1000;

        if (millies == 0) {
            if (this->timer.millies!=0) {
                removeTimer(&this->timer);
                this->timer.millies = 0;
            }
        } else {
            this->timer.resetMillies = resetMillies;			
            if (this->timer.millies!=0) {
                this->timer.millies = millies + KSystem::getMilliesSinceStart();
            } else {
                this->timer.millies = millies + KSystem::getMilliesSinceStart();
                addTimer(&this->timer);
            }
        }
    }	
    return 0;
}

#define CSIGNAL         0x000000ff      /* signal mask to be sent at exit */
#define K_CLONE_VM        0x00000100      /* set if VM shared between processes */
#define K_CLONE_FS        0x00000200      /* set if fs info shared between processes */
#define K_CLONE_FILES     0x00000400      /* set if open files shared between processes */
#define K_CLONE_SIGHAND   0x00000800      /* set if signal handlers and blocked signals shared */
#define K_CLONE_PTRACE    0x00002000      /* set if we want to let tracing continue on the child too */
#define K_CLONE_VFORK     0x00004000      /* set if the parent wants the child to wake it up on mm_release */
#define K_CLONE_PARENT    0x00008000      /* set if we want to have the same parent as the cloner */
#define K_CLONE_THREAD    0x00010000      /* Same thread group? */
#define K_CLONE_NEWNS     0x00020000      /* New namespace group? */
#define K_CLONE_SYSVSEM   0x00040000      /* share system V SEM_UNDO semantics */
#define K_CLONE_SETTLS    0x00080000      /* create a new TLS for the child */
#define K_CLONE_PARENT_SETTID     0x00100000      /* set the TID in the parent */
#define K_CLONE_CHILD_CLEARTID    0x00200000      /* clear the TID in the child */
#define K_CLONE_DETACHED          0x00400000      /* Unused, ignored */
#define K_CLONE_UNTRACED          0x00800000      /* set if the tracing process can't force K_CLONE_PTRACE on this clone */
#define K_CLONE_CHILD_SETTID      0x01000000      /* set the TID in the child */
/* 0x02000000 was previously the unused K_CLONE_STOPPED (Start in stopped state)
and is now available for re-use. */
#define K_CLONE_NEWUTS            0x04000000      /* New utsname group? */
#define K_CLONE_NEWIPC            0x08000000      /* New ipcs */
#define K_CLONE_NEWUSER           0x10000000      /* New user namespace */
#define K_CLONE_NEWPID            0x20000000      /* New pid namespace */
#define K_CLONE_NEWNET            0x40000000      /* New network namespace */
#define K_CLONE_IO                0x80000000      /* Clone io context */

U32 KProcess::clone(U32 flags, U32 child_stack, U32 ptid, U32 tls, U32 ctid) {    
    flags &= ~CSIGNAL;
    
    if (!(flags & K_CLONE_THREAD)) { // new thread group (process)
        bool vFork = false;
        if (flags & K_CLONE_VFORK) {
            flags &=~K_CLONE_VFORK;
            vFork = true;
        }
        bool vm = false;
        if (flags & K_CLONE_VM) {
            flags &=~K_CLONE_VM;
            vm = true;
        }
        if (flags & ~(K_CLONE_CHILD_SETTID|K_CLONE_CHILD_CLEARTID|K_CLONE_PARENT_SETTID)) {
            kpanic("KProcess::clone - unhandled flag 0x%X", (U32)(flags & ~(K_CLONE_CHILD_SETTID|K_CLONE_CHILD_CLEARTID|K_CLONE_PARENT_SETTID)));
        }
        std::shared_ptr<KProcess> newProcess = KProcess::create();
        if (vm) {
            newProcess->memory = this->memory;
            newProcess->memory->incRefCount();
        } else {            
            newProcess->memory = new Memory();
            newProcess->memory->clone(this->memory);
        }
        KThread* newThread = newProcess->createThread();

        newProcess->parentId = this->id;        
        
        newProcess->clone(shared_from_this());
        newThread->clone(KThread::currentThread());
        newThread->memory = newProcess->memory;

        // will only create them if they are missing
        //newProcess->initStdio();
        
        if ((flags & K_CLONE_CHILD_SETTID)!=0) {
            if (ctid!=0) {
                ChangeThread c(newThread); // so that writed will go to the new memory space
                 ::writed(ctid, newThread->id);
            }
        }
        if ((flags & K_CLONE_CHILD_CLEARTID)!=0) {
            newThread->clear_child_tid = ctid;
        }
        if ((flags & K_CLONE_PARENT_SETTID)!=0) {
            if (ptid) {                
                writed(ptid, newThread->id);
                ChangeThread c(newThread); // so that writed will go to the new memory space
                writed(ptid, newThread->id);
            }
        }
        if (child_stack!=0) {
            newThread->cpu->reg[4].u32 = child_stack;
        }
        if (vm) {
            newThread->cpu->reg[4].u32+=8;
            newThread->cpu->eip.u32 = newThread->cpu->peek32(0);
        } else {
            newThread->cpu->eip.u32 += 2; // step over clone call in the new thread
        }
        newThread->cpu->reg[0].u32 = 0;
        //runThreadSlice(newThread); // if the new thread runs before the current thread, it will likely call exec which will prevent unnessary copy on write actions when running the current thread first        
        if (vFork) {
#ifndef BOXEDWINE_MULTI_THREADED
            // don't re-enter when we wake up
            KThread::currentThread()->cpu->eip.u32+=2; // don't re-enter
            KThread::currentThread()->cpu->reg[0].u32 = newProcess->id;
#endif
            BOXEDWINE_CONDITION_LOCK(newProcess->exitOrExecCond);
            scheduleThread(newThread);
            BOXEDWINE_CONDITION_WAIT(newProcess->exitOrExecCond);
            BOXEDWINE_CONDITION_UNLOCK(newProcess->exitOrExecCond);
#ifdef BOXEDWINE_MULTI_THREADED
			if (KThread::currentThread()->terminating) {
				return -K_EINTR;
			}
#endif
        } else {
            //klog("starting %d/%d", newThread->process->id, newThread->id);            
            scheduleThread(newThread);
        }
        return newProcess->id;
    } else if ((flags & 0xFFFFFF00) == (K_CLONE_THREAD | K_CLONE_VM | K_CLONE_FS | K_CLONE_FILES | K_CLONE_SIGHAND | K_CLONE_SETTLS | K_CLONE_PARENT_SETTID | K_CLONE_CHILD_CLEARTID | K_CLONE_SYSVSEM)) {
        KThread* newThread = this->createThread();
        struct user_desc desc;

        readMemory((U8*)&desc, tls, sizeof(struct user_desc));

        if (desc.base_addr!=0 && desc.entry_number!=0) {
            struct user_desc* ldt = newThread->getLDT(desc.entry_number);
            *ldt = desc;
            newThread->cpu->setSegment(GS, desc.entry_number << 3);
        }
        newThread->clear_child_tid = ctid;
        writed(ptid, newThread->id);
        newThread->cpu->reg[4].u32 = child_stack;
        newThread->cpu->reg[4].u32+=8;
        newThread->cpu->eip.u32 = newThread->cpu->peek32(0);
        //klog("starting %d/%d", newThread->process->id, newThread->id);
        scheduleThread(newThread);
        return this->id;
    } else {
        kpanic("sys_clone does not implement flags: %X", flags);
        return 0;
    }
    return -K_ENOSYS;
}

void KProcess::killAllThreadsExceptCurrent() {
    std::vector<U32> threadIds;
    {
        BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(this->threadsCondition);
        std::unordered_map<U32, KThread*> tmp = this->threads;        
        for (auto& n : tmp) {
            KThread* thread = n.second;
            if (thread != KThread::currentThread()) {
                threadIds.push_back(thread->id);
            }
        }
    }
    // don't hold threadsCondition while calling terminateOtherThread
    for (auto& n : threadIds) {
        terminateOtherThread(shared_from_this(), n);
    }
}

void KProcess::killAllThreads() {
    std::vector<U32> threadIds;
    {
        BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(this->threadsCondition);
        std::unordered_map<U32, KThread*> tmp = this->threads;
        for (auto& n : tmp) {
            KThread* thread = n.second;
            threadIds.push_back(thread->id);
        }
    }
    // don't hold threadsCondition while calling terminateOtherThread
    for (auto& n : threadIds) {
        terminateOtherThread(shared_from_this(), n);
    }
}

U32 KProcess::exitgroup(U32 code) {
    std::shared_ptr<KProcess> parent = KSystem::getProcess(this->parentId);
    if (parent && parent->sigActions[K_SIGCHLD].handlerAndSigAction!=K_SIG_DFL) {
        if (parent->sigActions[K_SIGCHLD].handlerAndSigAction!=K_SIG_IGN) {
            parent->signalCHLD(K_CLD_EXITED, this->id, this->userId, code);
        }
    }

    killAllThreadsExceptCurrent();

    BOXEDWINE_CONDITION_LOCK(KSystem::processesCond);
    this->terminated = true;
    BOXEDWINE_CONDITION_UNLOCK(KSystem::processesCond);

    KThread::currentThread()->cleanup(); // must happen before we clear memory
    this->threads.clear();
    this->cleanupProcess(); // release RAM, sockets, etc now.  No reason to wait to do that until waitpid is called
    this->exitCode = code;

    BOXEDWINE_CONDITION_LOCK(this->exitOrExecCond);
    BOXEDWINE_CONDITION_SIGNAL_ALL(this->exitOrExecCond);
    BOXEDWINE_CONDITION_UNLOCK(this->exitOrExecCond);

    if (KSystem::getProcessCount()==1) {        
        // no one left to wait on this process, with no processes running main will exit boxedwine
        KSystem::eraseProcess(this->id);
    }  

    BOXEDWINE_CONDITION_LOCK(KSystem::processesCond);
    KSystem::wakeThreadsWaitingOnProcessStateChanged(); // after this the process could be deleted
    BOXEDWINE_CONDITION_UNLOCK(KSystem::processesCond);
     
	terminateCurrentThread(KThread::currentThread());
    return -K_CONTINUE;
}

U32 KProcess::mprotect(U32 address, U32 len, U32 prot) {
    bool read = (prot & K_PROT_READ)!=0;
    bool write = (prot & K_PROT_WRITE)!=0;
    bool exec = (prot & K_PROT_EXEC)!=0;
    U32 pageStart = address >> K_PAGE_SHIFT;
    U32 pageCount = (len+K_PAGE_SIZE-1)>>K_PAGE_SHIFT;
    U32 permissions = 0;
    U32 i;

    if (write)
        permissions|=PAGE_WRITE;
    if (read)
        permissions|=PAGE_READ;
    if (exec)
        permissions|=PAGE_EXEC;

    for (i=pageStart;i<pageStart+pageCount;i++) {
        this->memory->protectPage(i, permissions);
    }
    return 0;
}

U32 KProcess::fchdir(FD fildes) {
    KFileDescriptor* fd = this->getFileDescriptor(fildes);

    if (fd==0) {
        return -K_EBADF;
    }
    if (fd->kobject->type!=KTYPE_FILE) {
        return -K_EINVAL;
    }
    std::shared_ptr<KFile> p = std::dynamic_pointer_cast<KFile>(fd->kobject);

    if (!p->openFile->node->isDirectory()) {		
        return -K_ENOTDIR;
    }
    if (p->openFile->openedPath.length())
        this->currentDirectory = p->openFile->openedPath;
    else
        this->currentDirectory = p->openFile->node->path;
    if (stringHasEnding(this->currentDirectory, "/")) {
        this->currentDirectory = this->currentDirectory.substr(0, this->currentDirectory.length()-1);
    }
    return 0;
}

S64 KProcess::llseek(FD fildes, S64 offset, U32 whence) {
    KFileDescriptor* fd = this->getFileDescriptor(fildes);
    S64 pos;

    if (!fd) {
        return -K_EBADF;
    }
    if (whence==0) { // SEEK_SET
        pos = offset;
    } else if (whence==1) { // SEEK_CUR
        pos = fd->kobject->getPos();
        if (pos<0)
            return pos;
        pos+=offset;
    } else if (whence==2) { // SEEK_END
        pos = fd->kobject->length();
        if (pos<0)
            return pos;
        pos+=offset;
    } else {
        return -K_EINVAL;
    }
    return fd->kobject->seek(pos);
}

U32 writeRecord(U32 dirp, U32 len, U32 count, U32 pos, bool is64, const char* name, U32 id, U32 type) {
    U32 recordLen;

    if (is64) {
        recordLen = 20+(U32)strlen(name);
        recordLen=(recordLen+3) / 4 * 4;
        if (recordLen+len>count) {
            if (len==0)
                return -K_EINVAL;
            return 0;
        }
        writeq(dirp, id);
        writeq(dirp + 8, pos);
        writew(dirp + 16, recordLen);
        writeb(dirp + 18, type);
        writeNativeString(dirp + 19, name);
    } else {
        recordLen = 12+(U32)strlen(name);
        recordLen=(recordLen+3) / 4 * 4;
        if (recordLen+len>count) {
            if (len==0)
                return -K_EINVAL;
            return 0;
        }
        writed(dirp, id);
        writed(dirp + 4, pos);
        writew(dirp + 8, recordLen);
        writeNativeString(dirp + 10, name);
        writeb(dirp + recordLen - 1, type);
    }
    return recordLen;
}

U32 KProcess::getdents(FD fildes, U32 dirp, U32 count, bool is64) {
    KFileDescriptor* fd = this->getFileDescriptor(fildes);

    if (!fd) {
        return -K_EBADF;
    }
    if (fd->kobject->type!=KTYPE_FILE) {
        return -K_ENOTDIR;
    }
    std::shared_ptr<KFile> p = std::dynamic_pointer_cast<KFile>(fd->kobject);
    BoxedPtr<FsNode> node = p->openFile->node;
    FsOpenNode* openNode = p->openFile;

    if (!node->isDirectory()) {
        return -K_ENOTDIR;
    }
    U32 entries = openNode->getDirectoryEntryCount();
    U32 len = 0;

    for (U32 i=(U32)openNode->getFilePointer();i<entries;i++) {
        std::string name;
        BoxedPtr<FsNode> entry = openNode->getDirectoryEntry(i, name);
        U32 recordLen = writeRecord(dirp, len, count, i + 2, is64, name.c_str(), entry->id, entry->getType(true));
        if (recordLen>0) {
            dirp+=recordLen;
            len+=recordLen;
            openNode->seek(i+1);
        } else if (recordLen == 0) {
            return len;
        } else {
            return recordLen;
        }
    }
    return len;
}

U32 KProcess::msync(U32 addr, U32 len, U32 flags) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(mappedFilesMutex);
    for (auto& n : this->mappedFiles) {
        BoxedPtr<MappedFile> m = n.second;
        if (m->address<=addr && addr+len<m->address+m->len) {
            klog("msync not implemented");
            return 0;
        }
    }
    return -K_ENOMEM;
}

U32 KProcess::writev(FD handle, U32 iov, S32 iovcnt) {
    KFileDescriptor* fd = this->getFileDescriptor(handle);

    if (fd==0) {
        return -K_EBADF;
    }
    if (!fd->canWrite()) {
        return -K_EINVAL;
    }
    return fd->kobject->writev(iov, iovcnt);    
}

U32 KProcess::memfd_create(const std::string& name, U32 flags) {
    FsMemNode* node = new FsMemNode(1, 1, name);
    FsMemOpenNode* openNode = new FsMemOpenNode(flags, node);

    node->openNode = openNode;

    std::shared_ptr<KObject> kobject = std::make_shared<KFile>(openNode);

    U32 descriptorFlags = 0;
    if (flags & 1) { // MFD_CLOEXEC
        descriptorFlags = FD_CLOEXEC;
    }
    if (!(flags & 2)) { // MFD_ALLOW_SEALING	
        openNode->addSeals(K_F_SEAL_SEAL);
    }
    return this->allocFileDescriptor(kobject, K_O_RDWR, descriptorFlags, -1, 0)->handle;
}

U32 KProcess::mlock(U32 addr, U32 len) {
    return 0;
}


U32 KProcess::mremap(U32 oldaddress, U32 oldsize, U32 newsize, U32 flags) {
    if (flags > 1) {
        kpanic("mremap not implemented: flags=%X", flags);
    }
    // is page aligned
    if (oldaddress & 0xFFF) {
        return -K_EINVAL;
    }
    if (newsize==0) {
        return -K_EINVAL;
    }
    if (oldsize==0) {
        kpanic("mremap not implemented for oldsize==0");
    }
    U32 oldPageCount = oldsize>>K_PAGE_SHIFT;
    U32 pageFlags = this->memory->getPageFlags(oldaddress >> K_PAGE_SHIFT);

    for (U32 i=0;i<oldPageCount;i++) {
        if (this->memory->getPageFlags((oldaddress >> K_PAGE_SHIFT)+i)!=pageFlags) {
            return -K_EFAULT;
        }
    }
    if (newsize<oldsize) {
        this->unmap(oldaddress+newsize, oldsize-newsize);
        return oldaddress;
    } else {
        U32 result;
        U32 prot=0;        
        U32 f = K_MAP_FIXED;
        if (pageFlags & PAGE_READ) {
            prot|=K_PROT_READ;
        }
        if (pageFlags & PAGE_WRITE) {
            prot|=K_PROT_WRITE;
        }
        if (pageFlags & PAGE_EXEC) {
            prot|=K_PROT_EXEC;
        }
        if (pageFlags & PAGE_SHARED) {
            f|=K_MAP_SHARED;
        } else {
            f|=K_MAP_PRIVATE;
        }
        result = this->mmap(oldaddress+oldsize, newsize-oldsize, prot, f, -1, 0);
        if (result==oldaddress+oldsize) {
            return oldaddress;
        }
       
        if ((flags & 1)!=0) { // MREMAP_MAYMOVE
            kpanic("__NR_mremap not implemented");
            return -K_ENOMEM;
        } else {
            return -K_ENOMEM;
        }
    }
}

U32 KProcess::prctl(U32 option, U32 arg2) {
    if (option == 15) { // PR_SET_NAME
        char tmp[MAX_FILEPATH_LEN];
        this->name = getNativeString(arg2, tmp, sizeof(tmp));
        return 0;
    } else if (option == 38) { // PR_SET_NO_NEW_PRIVS
        return 0;
    } else {
        kwarn("prctl not implemented for option: %d", option);
    }
    return -1;
}

U32 KProcess::sigaction(U32 sig, U32 act, U32 oact, U32 sigsetSize) {
    if (sig == K_SIGKILL || sig == K_SIGSTOP || sig>MAX_SIG_ACTIONS || sig>=MAX_SIG_ACTIONS) {
        return -K_EINVAL;
    }
    if (oact!=0) {
        this->sigActions[sig].writeSigAction(oact, sigsetSize);
    }
    if (act!=0) {
        this->sigActions[sig].readSigAction(act, sigsetSize);
    }
    return 0;
}

U32 KProcess::pread64(FD fildes, U32 address, U32 len, U64 offset) {
    KFileDescriptor* fd = this->getFileDescriptor(fildes);

    if (!fd) {
        return -K_EBADF;
    }
    if (fd->kobject->type==KTYPE_NATIVE_SOCKET || fd->kobject->type==KTYPE_UNIX_SOCKET) {
        return -K_ESPIPE;
    }
    if (fd->kobject->type!=KTYPE_FILE) {
        return -K_EINVAL;
    }
    std::shared_ptr<KFile> p = std::dynamic_pointer_cast<KFile>(fd->kobject);
    FsOpenNode* openNode = p->openFile;

    if (openNode->node->isDirectory()) {
        return -K_EISDIR;
    }
    if (!this->memory->isValidWriteAddress(address, len)) {
        return -K_EFAULT;
    }
    return p->pread(address, (S64)offset, len);
}

U32 KProcess::pwrite64(FD fildes, U32 address, U32 len, U64 offset) {
    KFileDescriptor* fd = this->getFileDescriptor(fildes);

    if (!fd) {
        return -K_EBADF;
    }
    if (fd->kobject->type==KTYPE_NATIVE_SOCKET || fd->kobject->type==KTYPE_UNIX_SOCKET) {
        return -K_ESPIPE;
    }
    if (fd->kobject->type!=KTYPE_FILE) {
        return -K_EINVAL;
    }
    std::shared_ptr<KFile> p = std::dynamic_pointer_cast<KFile>(fd->kobject);
    FsOpenNode* openNode = p->openFile;

    if (openNode->node->isDirectory()) {
        return -K_EISDIR;
    }
    if (!this->memory->isValidReadAddress(address, len)) {
        return -K_EFAULT;
    }
    return p->pwrite(address, (S64)offset, len);
}

U32 KProcess::getcwd(U32 buffer, U32 size) {
    if (size==0) {
        return -K_EINVAL;
    }
    if (!this->memory->isValidWriteAddress(buffer, size)) {
        return -K_EFAULT;
    }
    BoxedPtr<FsNode> node = Fs::getNodeFromLocalPath("", this->currentDirectory, true);
    if (!node || !node->isDirectory()) {
        return -K_ENOENT;
    }
    if (this->currentDirectory.length()+1>size)
        return -K_ERANGE;
    writeNativeString(buffer, this->currentDirectory.c_str());
    return (U32)this->currentDirectory.length()+1;
}

U32 KProcess::stat64(const std::string& path, U32 buffer) {
    bool isLink = false;
    BoxedPtr<FsNode> node = Fs::getNodeFromLocalPath(this->currentDirectory, path, true, &isLink);
    if (!node) {
        return -K_ENOENT;
    }
    U64 len = node->length();
    KSystem::writeStat(node->path, buffer, true, 1, node->id, node->getMode(), node->rdev, len, 4096, (len + 4095) / 4096, node->lastModified(), node->getHardLinkCount());
    return 0;
}

U32 KProcess::lstat64(const std::string& path, U32 buffer) {
    BoxedPtr<FsNode> node = Fs::getNodeFromLocalPath(this->currentDirectory, path, false);
    if (!node) {
        return -K_ENOENT;
    }
 
    U64 len;
    U32 mode;

    if (node->isLink()) {
        len = node->getLink().length();
        mode = K__S_IFLNK | (node->getMode() & 0xFFF);
    } else {
        len = node->length();
        mode = node->getMode();
    }
    KSystem::writeStat(node->path, buffer, true, 1, node->id, mode, node->rdev, len, 4096, (len + 4095) / 4096, node->lastModified(), node->getHardLinkCount());
    return 0;
}

U32 KProcess::fstat64(FD handle, U32 buf) {
    KFileDescriptor* fd = this->getFileDescriptor(handle);
    
    if (!fd) {
        return -K_EBADF;
    }
    return fd->kobject->stat(buf, true);
}

U32 KProcess::mincore(U32 address, U32 length, U32 vec) {
    U32 i;
    U32 pages = (length+K_PAGE_SIZE+1)/K_PAGE_SIZE;
    U32 page = address >> K_PAGE_SHIFT;

    for (i=0;i<pages;i++) {
        if (page+i>=K_NUMBER_OF_PAGES)
            return -K_ENOMEM;
        if (this->memory->isPageAllocated(page+i))
            writeb(vec, 1);
        else
            writeb(vec, 0);
        vec++;
    }
    return 0;
}

U32 KProcess::fcntrl(FD fildes, U32 cmd, U32 arg) {
    KFileDescriptor* fd = this->getFileDescriptor(fildes);

    if (!fd) {
        return -K_EBADF;
    }    
    switch (cmd) {
        case K_F_SETOWN:
            if (this->id != arg) {
                kdebug("F_SETOWN not implemented: %d",fildes);
            }
            return 0;
        case K_F_DUPFD: {
            KFileDescriptor* result = this->allocFileDescriptor(fd->kobject, fd->accessFlags, fd->descriptorFlags, -1, arg);        
            return result->handle;
        }
        case K_F_DUPFD_CLOEXEC: {
            KFileDescriptor* result = this->allocFileDescriptor(fd->kobject, fd->accessFlags, fd->descriptorFlags, -1, arg);
            result->descriptorFlags=FD_CLOEXEC;
            return result->handle;
        }
        case K_F_GETFD:
            return fd->descriptorFlags;
        case K_F_SETFD:
            if (arg) {
                fd->descriptorFlags=FD_CLOEXEC;
            } else {
                fd->descriptorFlags=0;
            }
            return 0;
        // blocking is at the file description level, not the file descriptor level
        case K_F_GETFL:
            return fd->accessFlags | (fd->kobject->isBlocking()?0:K_O_NONBLOCK) | (fd->kobject->isAsync()?K_O_ASYNC:0);
        case K_F_SETFL:
            fd->accessFlags = (fd->accessFlags & K_O_ACCMODE) | (arg & ~K_O_ACCMODE);
            fd->kobject->setBlocking((arg & K_O_NONBLOCK)==0);
            fd->kobject->setAsync((arg & K_O_ASYNC)!=0);
            return 0;
        case K_F_GETLK: 
        case K_F_GETLK64:
            if (fd->kobject->supportsLocks()) {
                KFileLock lock;				
                KFileLock* result;
                lock.readFileLock(KThread::currentThread(), arg, cmd==K_F_GETLK64);
                result = fd->kobject->getLock(&lock);
                if (!result) {
                    writew(arg, K_F_UNLCK);
                } else {
                    result->writeFileLock(KThread::currentThread(), arg, K_F_GETLK64 == cmd);
                }
                return 0;
            } else {
                return -K_EBADF;
            }
        case K_F_SETLK: 
        case K_F_SETLK64:
        case K_F_SETLKW:
        case K_F_SETLKW64:
            if (fd->kobject->supportsLocks()) {
                KFileLock lock;

                lock.readFileLock(KThread::currentThread(), arg, cmd == K_F_SETLK64 || cmd == K_F_SETLKW64);
                lock.l_pid = this->id;
                if ((lock.l_type == K_F_WRLCK && !fd->canWrite()) || (lock.l_type == K_F_RDLCK && !fd->canRead())) {
                    return -K_EBADF;
                }
                return fd->kobject->setLock(&lock, (cmd == K_F_SETLKW) || (cmd == K_F_SETLKW64));
            } else {
                return -K_EBADF;
            }
        case K_F_SETSIG: {
            if (arg != K_SIGIO) {
                kpanic("fcntl F_SETSIG not implemented");
            }
            return 0;
        }
        case K_F_ADD_SEALS: {
            if (fd->kobject->type==KTYPE_FILE) {
                std::shared_ptr<KFile> f = std::dynamic_pointer_cast<KFile>(fd->kobject);
                if (f->openFile->node->type==FsNode::Memory) {
                    FsMemOpenNode* openNode = (FsMemOpenNode*)f->openFile;
                    return openNode->addSeals(arg);
                }
            }
            return -K_EINVAL;
        }
        case K_F_GET_SEALS: {
            if (fd->kobject->type==KTYPE_FILE) {
                std::shared_ptr<KFile> f = std::dynamic_pointer_cast<KFile>(fd->kobject);
                if (f->openFile->node->type==FsNode::Memory) {
                    FsMemOpenNode* openNode = (FsMemOpenNode*)f->openFile;
                    return openNode->getSeals();
                }
            }
            return -K_EINVAL;
        }
        default:
            kwarn("fcntl: unknown command: %d", cmd);
            return -K_EINVAL;
    }
}

U32 KProcess::set_thread_area(U32 info) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(usedTlsMutex);
    struct user_desc desc;    

    readMemory((U8*)&desc, info, sizeof(struct user_desc));
    if (desc.entry_number==(U32)(-1)) {
        U32 i;

        for (i=0;i<TLS_ENTRIES;i++) {
            if (this->usedTLS[i]==0) {
                desc.entry_number=i+TLS_ENTRY_START_INDEX;
                break;
            }
        }
        if (desc.entry_number==(U32)(-1)) {
            kwarn("__NR_set_thread_area ran out of TLS slots");
            return -K_ESRCH;
        }
        writeMemory(info, (U8*)&desc, sizeof(struct user_desc));
    }
    if (desc.base_addr!=0) {
        if (desc.entry_number<TLS_ENTRY_START_INDEX || desc.entry_number>=TLS_ENTRIES+TLS_ENTRY_START_INDEX) {
            return -K_ESRCH;
        }
        this->usedTLS[desc.entry_number-TLS_ENTRY_START_INDEX]=1;

        KThread::currentThread()->setTLS(&desc);
    }
    return 0;
}

U32 KProcess::epollcreate(U32 size, U32 flags) {
    std::shared_ptr<KObject> o = std::make_shared<KEPoll>();
    KFileDescriptor* result = this->allocFileDescriptor(o, K_O_RDWR, flags, -1, 0);
    return result->handle;
}

U32 KProcess::epollctl(FD epfd, U32 op, FD fd, U32 address) {
    KFileDescriptor* epollFD = this->getFileDescriptor(epfd);
    if (fd==epfd || epollFD->kobject->type != KTYPE_EPOLL) {
        return -K_EINVAL;
    }
    std::shared_ptr<KEPoll> p = std::dynamic_pointer_cast<KEPoll>(epollFD->kobject);
    return p->ctl(op, fd, address);
}

U32 KProcess::epollwait(FD epfd, U32 events, U32 maxevents, U32 timeout) {
    KFileDescriptor* epollFD = this->getFileDescriptor(epfd);
    if (!epollFD) {
        return -K_EBADF;
    }
    if (epollFD->kobject->type != KTYPE_EPOLL) {
        return -K_EINVAL;
    }
    std::shared_ptr<KEPoll> p = std::dynamic_pointer_cast<KEPoll>(epollFD->kobject);
    return p->wait(events, maxevents, timeout);
}

U32 KProcess::utimes(const std::string& path, U32 times) {
    BoxedPtr<FsNode> node = Fs::getNodeFromLocalPath(this->currentDirectory, path, true);

    if (!node) {
        return -K_ENOENT;
    } else {        
        U64 lastAccessTime = 0;
        U32 lastAccessTimeNano = 0;
        U64 lastModifiedTime =  0;
        U32 lastModifiedTimeNano = 0;
        if (!times) {
            lastAccessTime = time(NULL);
            lastModifiedTime = time(NULL);
        } else {
            lastAccessTime = readd(times);
            lastAccessTimeNano = readd(times+4)*1000;
            lastModifiedTime = readd(times+8);
            lastModifiedTimeNano = readd(times+12)*1000;
        }
        return node->setTimes(lastAccessTime, lastAccessTimeNano, lastModifiedTime, lastModifiedTimeNano);
    }
}

U32 KProcess::getCurrentDirectoryFromDirFD(FD dirfd, std::string& currentDirectory) {
    U32 result = 0;
    if (dirfd==-100) { // AT_FDCWD
        currentDirectory = this->currentDirectory;
    } else {
        KFileDescriptor* fd = this->getFileDescriptor(dirfd);
        if (!fd) {
            result = -K_EBADF;
        } else if (fd->kobject->type!=KTYPE_FILE){
            result = -K_ENOTDIR;
        } else {
            std::shared_ptr<KFile> f = std::dynamic_pointer_cast<KFile>(fd->kobject);
            currentDirectory = f->openFile->node->path;
        }
    }
    return result;
}

U32 KProcess::openat(FD dirfd, const std::string& path, U32 flags) {
    std::string dir;
    U32 result = 0;
    
    if (path[0]!='/')
        result = getCurrentDirectoryFromDirFD(dirfd, dir);

    if (result)
        return result;
    KFileDescriptor* fd = NULL;
    result = this->openFile(dir, path, flags, &fd);
    if (result || !fd) {
        return result;
    }
    return fd->handle;
}

U32 KProcess::mkdirat(U32 dirfd, const std::string& path, U32 mode) {
    std::string dir;
    U32 result = 0;
    
    if (path[0]!='/')
        result = getCurrentDirectoryFromDirFD(dirfd, dir);

    if (result)
        return result;
    std::string fullPath = Fs::getFullPath(dir, path);
    return this->mkdir(fullPath);
}

U32 KProcess::fstatat64(FD dirfd, const std::string& path, U32 buf, U32 flag) {
    std::string dir;
    U32 result = 0;
    
    if (path[0]!='/')
        result = getCurrentDirectoryFromDirFD(dirfd, dir);

    if (result)
        return result;
    bool isLink = false;

    BoxedPtr<FsNode> node = Fs::getNodeFromLocalPath(dir, path, (flag & 0x100)==0, &isLink);
    if (!node) {
        return -K_ENOENT;
    }

    U64 len;
    U32 mode = node->getMode();
    if (node->isLink() || isLink) {
        mode|=K__S_IFLNK;
    }
    len = node->length();
    KSystem::writeStat(path, buf, true, 1, node->id, mode, node->rdev, len, 4096, (len + 4095) / 4096, node->lastModified(), node->getHardLinkCount());
    return 0;    
}

U32 KProcess::unlinkat(FD dirfd, const std::string& path, U32 flags) {
    std::string dir;
    U32 result = 0;
    
    if (path[0]!='/')
        result = getCurrentDirectoryFromDirFD(dirfd, dir);

    if (result)
        return result;
    BoxedPtr<FsNode> node = Fs::getNodeFromLocalPath(dir, path, false);
    if (!node) {
        return -K_ENOENT;
    }
    if (flags & 0x200) { // unlinkat AT_REMOVEDIR
        if (!node->isDirectory()) {
            return -K_ENOTDIR;
        }
        if (node->removeDir() == 0)
            return 0;
        return -K_ENOTEMPTY;
    } else {
        if (!node->remove()) {
            kwarn("filed to remove file: errno=%d", errno);
            return -K_EBUSY;
        }
        return 0;
    }
}

U32 KProcess::faccessat(U32 dirfd, const std::string& path, U32 mode, U32 flags) {    
    std::string dir;
    U32 result = 0;
    
    if (path[0]!='/')
        result = getCurrentDirectoryFromDirFD(dirfd, dir);

    if (result)
        return result;
    BoxedPtr<FsNode> node = Fs::getNodeFromLocalPath(dir, path, (flags & 0x100)==0);
    if (!node) {
        return -K_ENOENT;
    }  
    return internalAccess(node, mode);
}

#define K_UTIME_NOW 0x3fffffff
#define K_UTIME_OMIT 0x3ffffffe

U32 KProcess::utimesat(FD dirfd, const std::string& path, U32 times, U32 flags) {
    std::string dir;
    U32 result = 0;
    
    if (path[0]!='/')
        result = getCurrentDirectoryFromDirFD(dirfd, dir);

    if (result)
        return result;
    BoxedPtr<FsNode> node = Fs::getNodeFromLocalPath(dir, path, (flags & 0x100)==0);
    if (!node) {
        return -K_ENOENT;
    } 
    U64 lastAccessTime = 0;
    U32 lastAccessTimeNano = 0;
    U64 lastModifiedTime =  0;
    U32 lastModifiedTimeNano = 0;

    if (times) {
        lastAccessTime = readd(times);
        lastAccessTimeNano = readd(times+4);
        lastModifiedTime = readd(times+8);
        lastModifiedTimeNano = readd(times+12);
    }
    if (lastAccessTimeNano != K_UTIME_OMIT) {
        if (lastAccessTimeNano == K_UTIME_NOW) {
            lastAccessTime = time(NULL);
            lastAccessTimeNano = 0;
        }
    }
    if (lastModifiedTimeNano != K_UTIME_OMIT) {
        if (lastModifiedTimeNano == K_UTIME_NOW) {
            lastModifiedTime = (U32)time(NULL);
            lastModifiedTimeNano = 0;
        }
    }
    return node->setTimes(lastAccessTime, lastAccessTimeNano, lastModifiedTime, lastAccessTimeNano);
}

user_desc* KProcess::getLDT(U32 index) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(ldtMutex);
    if (this->ldt.count(index))
        return &this->ldt[index];
    return NULL;
}

BoxedPtr<SHM> KProcess::allocSHM(U32 key, U32 afterIndex) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(privateShmMutex);
    BoxedPtr<SHM> result;

    while (this->privateShm.count(afterIndex)) {
        if (afterIndex > 0x7FFFFFFF)
            kpanic("allocSHM ran out of indexes");
        afterIndex++;
    }
    result = new SHM(afterIndex, key);
    this->privateShm[afterIndex] = result;
    return result;
}

BoxedPtr<SHM> KProcess::getSHM(U32 key) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(privateShmMutex);
    if (this->privateShm.count(key))
        return this->privateShm[key];
    return NULL;
}

void KProcess::attachSHM(U32 address, const BoxedPtr<SHM>& shm) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(attachedShmMutex);
    BoxedPtr<AttachedSHM> attached = new AttachedSHM(shm, address, this->id);
    this->attachedShm[address] = attached;
}

U32 KProcess::shmdt(U32 shmaddr) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(attachedShmMutex);

    if (this->attachedShm.count(shmaddr)) {
        BoxedPtr<AttachedSHM> attached = this->attachedShm[shmaddr];
        if (attached) {
            this->memory->reset(shmaddr >> K_PAGE_SHIFT, (U32)attached->shm->pages.size());
            this->attachedShm.erase(shmaddr);
            return 0;
        }
    }
    return -K_EINVAL;
}

void KProcess::iterateThreads(std::function<bool(KThread*)> callback) {
	BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(threadsCondition);
    for (auto& t : this->threads) {
        if (!callback(t.second)) {
            return;
        }
    }
}

void KProcess::printStack() {
	BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(threadsCondition);
    for (auto& t : this->threads) {
        KThread* thread = t.second;
        CPU* cpu=thread->cpu;
        
        if (thread->waitingCond) {            
            klog("  thread %X WAITING %s", thread->id, thread->waitingCond->name.c_str());
        } else {
            klog("  thread %X RUNNING", thread->id);
            std::string name = this->getModuleName(cpu->seg[CS].address+cpu->eip.u32);

            klog("    0x%08d %s", this->getModuleEip(cpu->seg[CS].address+cpu->eip.u32), name.length()?name.c_str():"Unknown");
        }
        cpu->walkStack(cpu->eip.u32, EBP, 6);
    }
}

U32 KProcess::signal(U32 signal) {
    for (auto& t : this->threads) {
        KThread* thread = t.second;

        if (((U64)1 << (signal-1)) & ~(thread->inSignal?thread->inSigMask:thread->sigMask)) {
            return thread->signal(signal, true);
        }
    }
    // didn't find a thread that could handle it
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(this->pendingSignalsMutex);
    this->pendingSignals |= ((U64)1 << (signal-1));
    this->signalFd(NULL, signal);
    return 0;
}

void KProcess::signalFd(KThread* thread, U32 signal) {
    for (auto& n : this->fds) {
        KFileDescriptor* fd = n.second;
        if (fd->kobject->type == KTYPE_SIGNAL) {
            std::shared_ptr<KSignal> p = std::dynamic_pointer_cast<KSignal>(fd->kobject);
            if ((p->mask & signal) && (!thread || thread->waitingCond == &p->lockCond)) {
                BOXEDWINE_CONDITION_LOCK(p->lockCond);
                p->sigAction = this->sigActions[signal];
                p->signalingPid = this->id;
                p->signalingUid = this->userId;
                BOXEDWINE_CONDITION_SIGNAL(p->lockCond);
                BOXEDWINE_CONDITION_UNLOCK(p->lockCond);
            }
        }
    }
}

void KProcess::printMappedFiles() {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(mappedFilesMutex);
    for (auto& n : this->mappedFiles) {
        const BoxedPtr<MappedFile>& mappedFile = n.second;
        klog("    %.8X - %.8X %s\n", mappedFile->address, mappedFile->address+(int)mappedFile->len, mappedFile->file->openFile->node->path.c_str());
    }
}

#ifdef BOXEDWINE_64BIT_MMU
#include "../emulation/hardmmu/hard_memory.h"
U32 KProcess::allocNative(U32 len) {
    U32 page = this->nextNativeAddress;
    U32 pageCount = (len+K_PAGE_SIZE-1) >> K_PAGE_SHIFT;
    this->memory->allocNativeMemory(page, pageCount, PAGE_READ|PAGE_WRITE);
    this->nextNativeAddress+=pageCount;
    return page << K_PAGE_SHIFT;
}

U32 KProcess::readd(U32 address) {
    return *(U32*)getNativeAddress(memory, address);
}

U16 KProcess::readw(U32 address) {
    return *(U16*)getNativeAddress(memory, address);
}

U8 KProcess::readb(U32 address) {
    return *(U8*)getNativeAddress(memory, address);
}

void KProcess::writed(U32 address, U32 value) {
    *(U32*)getNativeAddress(memory, address) = value;
}

void KProcess::writew(U32 address, U16 value) {
    *(U16*)getNativeAddress(memory, address) = value;
}

void KProcess::writeb(U32 address, U8 value) {
    *(U8*)getNativeAddress(memory, address) = value;
}

void KProcess::memcopyFromNative(U32 address, const void* p, U32 len) {
    memcpy(getNativeAddress(memory, address), p, len);
}

void KProcess::memcopyToNative( U32 address, void* p, U32 len) {
    memcpy(p, getNativeAddress(memory, address), len);
}

#else

U32 KProcess::readd(U32 address) {
    if ((address & 0xFFF) < 0xFFD) {
        int index = address >> 12;
#ifndef UNALIGNED_MEMORY
        if (memory->mmuReadPtr[index])
            return *(U32*)(&memory->mmuReadPtr[index][address & 0xFFF]);
#endif
        return memory->mmu[index]->readd(address);
    } else {
        return readb(address) | (readb(address+1) << 8) | (readb(address+2) << 16) | (readb(address+3) << 24);
    }
}

U16 KProcess::readw(U32 address) {
    if ((address & 0xFFF) < 0xFFF) {
        int index = address >> 12;
#ifndef UNALIGNED_MEMORY
        if (memory->mmuReadPtr[index])
            return *(U16*)(&memory->mmuReadPtr[index][address & 0xFFF]);
#endif
        return memory->mmu[index]->readw(address);
    }
    return readb(address) | (readb(address+1) << 8);
}

U8 KProcess::readb(U32 address) {
    int index = address >> 12;
    if (memory->mmuReadPtr[index])
        return memory->mmuReadPtr[index][address & 0xFFF];
    return memory->mmu[index]->readb(address);
}

void KProcess::writed(U32 address, U32 value) {
    if ((address & 0xFFF) < 0xFFD) {
        int index = address >> 12;
#ifndef UNALIGNED_MEMORY
        if (memory->mmuWritePtr[index])
            *(U32*)(&memory->mmuWritePtr[index][address & 0xFFF]) = value;
        else
#endif
            memory->mmu[index]->writed(address, value);
    } else {
        writeb(address, value);
        writeb(address+1, value >> 8);
        writeb(address+2, value >> 16);
        writeb(address+3, value >> 24);
    }
}

void KProcess::writew(U32 address, U16 value) {
    if ((address & 0xFFF) < 0xFFF) {
        int index = address >> 12;
#ifndef UNALIGNED_MEMORY
        if (memory->mmuWritePtr[index])
            *(U16*)(&memory->mmuWritePtr[index][address & 0xFFF]) = value;
        else
#endif
            memory->mmu[index]->writew(address, value);
    } else {
        writeb(address, (U8)value);
        writeb(address+1, (U8)(value >> 8));
    }
}

void KProcess::writeb(U32 address, U8 value) {
    int index = address >> 12;
    if (memory->mmuWritePtr[index])
        memory->mmuWritePtr[index][address & 0xFFF] = value;
    else
        memory->mmu[index]->writeb(address, value);
}

void KProcess::memcopyFromNative(U32 address, const void* pv, U32 len) {
    U32 i;
    U8* p = (U8*)pv;
    for (i=0;i<len;i++) {
        writeb(address+i, p[i]);
    }
}

void KProcess::memcopyToNative(U32 address, void* pv, U32 len) {
    U8* p = (U8*)pv;
    for (U32 i=0;i<len;i++) {
        p[i] = readb(address+i);
    }
}

#endif
