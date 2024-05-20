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
    process->processNode = KSystem::addProcess(process->id, process);
    if (process->processNode) {
        std::weak_ptr<KProcess> weak_process = process;
        Fs::addDynamicLinkFile(process->processNode->path + "/exe", k_mdev(0, 0), process->processNode, false, [weak_process]() {
            std::shared_ptr<KProcess> p = weak_process.lock();
            if (p) {
                return p->exe;
            }
            return BString::empty;
            });
        Fs::addVirtualFile(process->processNode->path + "/loginuid", K__S_IREAD, k_mdev(0, 0), process->processNode, B("1"));
        process->fdNode = Fs::addFileNode(process->processNode->path + "/fd", B(""), B(""), true, process->processNode);
        process->taskNode = Fs::addFileNode(process->processNode->path + B("/task"), B(""), B(""), true, process->processNode);
    }
    process->timer.process = process; // can't use shared_from_this in constructor    
    return process;
}

KProcess::KProcess(U32 id) : id(id), exitOrExecCond(std::make_shared<BoxedWineCondition>(B("KProcess::exitOrExecCond"))), threadRemovedCondition(std::make_shared<BoxedWineCondition>(B("KProcess::threadRemovedCondition"))) {
    for (int i=0;i<LDT_ENTRIES;i++) {
        this->ldt[i].seg_not_present = 1;
        this->ldt[i].read_exec_only = 1;
    }

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

    this->hasSetSeg[GS] = true;
    this->hasSetSeg[FS] = true;    
}

void KProcess::onExec(KThread* thread) {
    BHashTable<U32, KFileDescriptor*> fdsToClose = this->fds; // make a copy since we can't remove from it while iterating
    for( const auto& n : fdsToClose ) {
        KFileDescriptor* fd = n.value;
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
        if (thread != n.value) {
            toDelete.push_back(n.value);
        }
    }
    for (auto& otherThread : toDelete) {
        terminateOtherThread(shared_from_this(), otherThread->id);
    }
    this->threads.clear();
    this->threads.set(thread->id, thread);

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
    returnToLoopAddress = nullptr;
    reTranslateChunkAddress = nullptr;
    syncToHostAddress = nullptr;
    syncFromHostAddress = nullptr;
    doSingleOpAddress = nullptr;
    jmpAndTranslateIfNecessary = nullptr;
#ifdef BOXEDWINE_POSIX
    runSignalAddress = nullptr;
#endif
#endif
}

KProcess::~KProcess() {
    killAllThreads(KThread::currentThread());
    this->cleanupProcess();
    if (memory) {
        delete memory;
    }
}

void KProcess::cleanupProcess() {    
    removeTimer(&this->timer);

    BHashTable<U32, KFileDescriptor*> fdsToClose = this->fds; // make a copy since we can't remove from it while iterating
    for( const auto& n : fdsToClose ) {
        KFileDescriptor* fd = n.value;
        fd->refCount = 1; // make sure it is really closed
        fd->close();
    }
    this->attachedShm.clear();
    this->privateShm.clear();
    this->mappedFiles.clear();
    // will be handled when thread exits, we don't want to delete current memory associated with execution
#ifndef BOXEDWINE_BINARY_TRANSLATOR    
    if (memory) {
        memory->cleanup();
    }
#endif
}

KThread* KProcess::createThread() {	
    KThread* thread = new KThread(KSystem::getNextThreadId(), shared_from_this());

    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(threadsMutex);
    this->threads.set(thread->id, thread);
    return thread;
}

void KProcess::removeThread(KThread* thread) {
	BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(threadRemovedCondition);
	BOXEDWINE_CONDITION_SIGNAL(threadRemovedCondition);

    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(threadsMutex);
    this->threads.remove(thread->id);
}

KThread* KProcess::getThreadById(U32 tid) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(threadsMutex);
    return this->threads[tid];
}

U32 KProcess::getThreadCount() {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(threadsMutex);
    return (U32)this->threads.size();
}

void KProcess::deleteThread(KThread* thread) {
    thread->cleanup();        
    if (this->threads.size() == 1 && this->threads.get(thread->id)==thread) {
        delete this->memory; // this might call KThread::currentThread, so don't delete thread before this
        this->memory = nullptr;
    }
    delete thread;
    // don't call into getProcess while holding threadsCondition
    if (!this->terminated && this->getThreadCount() == 0) {
        std::shared_ptr<KProcess> parent = KSystem::getProcess(this->parentId);
        if (!parent || parent->getThreadCount() == 0) {
            KSystem::eraseProcess(this->id);
        }
    }
}

FsOpenNode* openCommandLine(const std::shared_ptr<FsNode>& node, U32 flags, U32 data) {
    return new BufferAccess(node, flags, KThread::currentThread()->process->commandLine);
}

void KProcess::setupCommandlineNode() {
    Fs::addVirtualFile(this->processNode->path +"/cmdline", openCommandLine, K__S_IREAD, 0, this->processNode);
}

BString KProcess::getAbsoluteExePath() { 
    BString path = Fs::getNodeFromLocalPath(B(""), this->exe, true)->path;
    return Fs::getParentPath(path);
}
void KProcess::clone(const std::shared_ptr<KProcess>& from) {
    this->parentId = from->id;;
    this->groupId = from->groupId;
    this->userId = from->userId;
    this->effectiveUserId = from->effectiveUserId;
    this->effectiveGroupId = from->effectiveGroupId;
    this->currentDirectory = from->currentDirectory;
    this->brkEnd = from->brkEnd;
    for (auto& n : from->fds) {
        KFileDescriptor* fromFd = n.value;
        KFileDescriptor* fd = this->allocFileDescriptor(fromFd->kobject, fromFd->accessFlags, fromFd->descriptorFlags, n.key, 0);
        fd->refCount = fromFd->refCount;
        this->fds.set(n.key, fd);     
        KObject* kobject = fd->kobject.get();
        Fs::addDynamicLinkFile(fdNode->path + "/" + BString::valueOf(n.key), k_mdev(0, 0), fdNode, false, [kobject] {
            return kobject->selfFd();
            });
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
        std::shared_ptr<AttachedSHM> attached = std::make_shared<AttachedSHM>(n.value->shm, n.key, this->id);
        this->attachedShm.set(n.key, attached);
    }

    for (U32 i=0;i<LDT_ENTRIES;i++) {
        this->ldt[i] = from->ldt[i];
    }

    this->loaderBaseAddress = from->loaderBaseAddress;
    this->phdr = from->phdr;
    this->phnum = from->phnum;
    this->entry = from->entry;

    for (U32 i=0;i<6;i++) {
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
    thread->memory->strcpy(ESP, s);
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
    std::shared_ptr<KProcess> process = cpu->thread->process;

    cpu->push32(rand());
    cpu->push32(rand());
    cpu->push32(rand());
    cpu->push32(rand());
    U32 randomAddress = ESP;
    cpu->push32(0);
    cpu->push32(0);
    writeStackString(thread, cpu, "i686");
    U32 platform = ESP;

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
    for (int i=envc-1;i>=0;i--) {
        cpu->push32(e[i]);
    }
    cpu->push32(0);
    for (int i=argc-1;i>=0;i--) {
        cpu->push32(a[i]);
    }
    cpu->push32(argc);
}

static void setupThreadStack(KThread* thread, CPU* cpu, BString programName, const std::vector<BString>& args, const std::vector<BString>& env) {
    U32 a[MAX_ARG_COUNT] = {};
    U32 e[MAX_ARG_COUNT] = {};

    cpu->push32(0);
    cpu->push32(0);
    cpu->push32(0);	
    writeStackString(thread, cpu, programName.c_str());
    if (args.size()>MAX_ARG_COUNT)
        kpanic("Too many args: %d is max", MAX_ARG_COUNT);
    if (env.size()>MAX_ARG_COUNT)
        kpanic("Too many env: %d is max", MAX_ARG_COUNT);
    //klog("env");
    for (size_t i=0;i<env.size();i++) {
        writeStackString(thread, cpu, env[i].c_str());
        if (strncmp(env[i].c_str(), "PATH=", 5)==0) {
            env[i].substr(5).split(':', cpu->thread->process->path);
        }
        //klog("    %s", env[i]);
        e[i]=ESP;
    }
    for (S32 i=(S32)args.size()-1;i>=0;i--) {
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
    if (handle<0) {
        handle = this->getNextFileDescriptorHandle(afterHandle);
    }
    KFileDescriptor* result = new KFileDescriptor(shared_from_this(), kobject, accessFlags, descriptorFlags, handle);

    KFileDescriptor* old = this->getFileDescriptor(handle);
    if (old) {
        old->close();
    }
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(fdsMutex);
    this->fds.set(handle, result);
    Fs::addDynamicLinkFile(fdNode->path+"/"+BString::valueOf(handle), k_mdev(0, 0), fdNode, false, [kobject] {
        return kobject->selfFd();
        });
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

U32 KProcess::openFileDescriptor(BString currentDirectory, BString localPath, U32 accessFlags, U32 descriptorFlags, S32 handle, U32 afterHandle, KFileDescriptor** result) {
    std::shared_ptr<FsNode> node;
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
        BString fullPath = Fs::getFullPath(currentDirectory, localPath);
        BString parentPath = Fs::getParentPath(fullPath);
        BString fileName = Fs::getFileNameFromPath(fullPath);
        std::shared_ptr<FsNode> parent = Fs::getNodeFromLocalPath(B(""), parentPath, true);
        if (!parent) {
            return -K_ENOENT;
        }       
        BString nativePath = Fs::getNativePathFromParentAndLocalFilename(parent, fileName);
        std::shared_ptr<FsNode> mixedSibling = parent->getChildByNameIgnoreCase(fileName);
        if (mixedSibling) {
            nativePath = nativePath + EXT_MIXED;
            if (Fs::doesNativePathExist(nativePath)) {
                kwarn("KProcess::openFileDescriptor mixed file already exists");
            }
        }
        node = Fs::addFileNode(parent->path+"/"+fileName, B(""), nativePath, false, parent);
        Fs::makeLocalDirs(parent->path);
    }
    std::shared_ptr<KObject> nodeObject = node->kobject.lock();
    if (nodeObject) {
        kobject = nodeObject;
    } else {
        FsOpenNode* openNode = node->open(accessFlags);
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

U32 KProcess::openFile(BString currentDirectory, BString localPath, U32 accessFlags, KFileDescriptor** result) {
    return this->openFileDescriptor( currentDirectory, localPath, accessFlags, (accessFlags & K_O_CLOEXEC)?FD_CLOEXEC:0, -1, 0, result);
}

void KProcess::initStdio() {
    if (!this->getFileDescriptor(0)) {
        this->openFileDescriptor(B(""), B("/dev/tty0"), K_O_RDONLY, 0, 0, 0, nullptr);
    }
    if (!this->getFileDescriptor(1)) {
        this->openFileDescriptor(B(""), B("/dev/tty0"), K_O_WRONLY, 0, 1, 0, nullptr);
    }
    if (!this->getFileDescriptor(2)) {
        this->openFileDescriptor(B(""), B("/dev/tty0"), K_O_WRONLY, 0, 2, 0, nullptr);
    }
}

KThread* KProcess::startProcess(BString currentDirectory, const std::vector<BString>& argValues, const std::vector<BString>& envValues, int userId, int groupId, int effectiveUserId, int effectiveGroupId) {
    std::shared_ptr<FsNode> node = Fs::getNodeFromLocalPath(currentDirectory, argValues[0], true);

    if (!node) {
        kwarn("Could not find %s", argValues[0].c_str());
        return nullptr;
    }

    BString interpreter;
    BString loader;
    std::vector<BString> interpreterArgs;
    std::vector<BString> args;
    std::vector<BString> env;
    this->memory = KMemory::create(this);
    KThread* thread = this->createThread();
    BString name;

    KThread::setCurrentThread(thread);
    thread->setupStack();
    this->parentId = 1;
    this->userId = userId;
    this->effectiveUserId = effectiveUserId;
    this->groupId = groupId;	
    this->effectiveGroupId = effectiveGroupId;
    this->setupCommandlineNode();
    this->exe = argValues[0];
    this->name = Fs::getFileNameFromPath(this->exe);
    
    for (U32 i=0;i<argValues.size();i++) {
        if (i>0)
            this->commandLine+=" ";
        this->commandLine+=argValues[i];
    }
    
    this->initStdio();
    FsOpenNode* openNode=ElfLoader::inspectNode(currentDirectory, node, loader, interpreter, interpreterArgs);
    if (!openNode) {
        return nullptr;
    }    
    if (ElfLoader::loadProgram(thread, openNode, &thread->cpu->eip.u32)) {
        // :TODO: why will it crash in strchr libc if I remove this
        //syscall_mmap64(thread, ADDRESS_PROCESS_LOADER << PAGE_SHIFT, 4096, K_PROT_READ | K_PROT_WRITE, K_MAP_ANONYMOUS|K_MAP_PRIVATE, -1, 0);
        
        if (loader.length())
            args.push_back(loader);
        if (interpreter.length()) {
            exe = interpreter;
            args.push_back(interpreter);
        }
        args.push_back(BString(Fs::getFullPath(currentDirectory, argValues[0])));
        for (U32 i=1;i<argValues.size();i++) {
            args.push_back(argValues[i]);
        }
        for (U32 i=0;i<envValues.size();i++) {
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

U32 KProcess::exit(KThread* thread, U32 code) {
    this->exitCode = code;
    if (this->getThreadCount()==1)
        return this->exitgroup(thread, code);    

    thread->cpu->yield = true;
    thread->cleanup(); // will unschedule the thread
	terminateCurrentThread(thread);

    return 0;
}

KFileDescriptor* KProcess::getFileDescriptor(FD handle) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(fdsMutex);
    return this->fds[handle];
}

void KProcess::clearFdHandle(FD handle) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(fdsMutex);
    this->fds.remove(handle);
    fdNode->removeChildByName(BString::valueOf(handle));
}

bool KProcess::isStopped() {
    return false;
}

bool KProcess::isTerminated() {
    return this->terminated;
}

BString KProcess::getModuleName(U32 eip) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(mappedFilesMutex);
    for (auto& n : this->mappedFiles) {
        std::shared_ptr<MappedFile> mappedFile = n.value;
        if (eip >= mappedFile->address && eip < mappedFile->address + mappedFile->len) {
            return mappedFile->file->openFile->node->name;
        }
    }
    return B("Unknown");
}

U32 KProcess::getModuleEip(U32 eip) {    
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(mappedFilesMutex);
    for (auto& n : this->mappedFiles) {
        std::shared_ptr<MappedFile> mappedFile = n.value;
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

std::shared_ptr<FsNode> KProcess::findInPath(BString path) {
    std::shared_ptr<FsNode> node = Fs::getNodeFromLocalPath(this->currentDirectory, path, true);

    if (!node && !path.startsWith('/')) {
        for( const auto& n : this->path ) {
            node = Fs::getNodeFromLocalPath(n, path, true);
            if (node)
                break;
        }
    }
    return node;
}

U32 KProcess::execve(KThread* thread, BString path, std::vector<BString>& args, const std::vector<BString>& envs) {
    std::shared_ptr<FsNode> node;
    FsOpenNode* openNode = nullptr;
    BString interpreter;
    std::vector<BString> interpreterArgs;
    BString loader;
    BString name;
    std::vector<BString> cmdLine;

    this->systemProcess = false;
    for (auto& s : args) {
        if (s.endsWith("wineboot.exe", true)) {
            this->systemProcess = true;
        } else if (s.endsWith("winemenubuilder.exe", true)) {
            this->systemProcess = true;
        } else if (s.endsWith("services.exe", true)) {
            this->systemProcess = true;
        } else if (s.endsWith("plugplay.exe", true)) {
            this->systemProcess = true;
        } else if (s.endsWith("winedevice.exe", true)) {
            this->systemProcess = true;
        } else if (s.endsWith("explorer.exe", true)) {
            this->systemProcess = true;
        } else if (s.endsWith("wineserver", true)) {
            Platform::setCurrentThreadPriorityHigh();
            this->systemProcess = true;
        }
    }
    node = this->findInPath(path);
    if (!node) {
        return -K_ENOENT;
    }
    openNode = ElfLoader::inspectNode(this->currentDirectory, node, loader, interpreter, interpreterArgs);
    if (!openNode) {
        return -K_ENOEXEC;
    }
#ifdef BOXEDWINE_MULTI_THREADED
    if (KSystem::cpuAffinityCountForApp) {
        Platform::setCpuAffinityForThread(thread, this->isSystemProcess()?0:KSystem::cpuAffinityCountForApp);
    }
#endif    
    if (interpreter.length()) {
        args.insert(args.begin(), interpreterArgs.begin(), interpreterArgs.end());
        args.insert(args.begin(), interpreter);
        this->exe = interpreter;
    } else {        
        if (path != "/proc/self/exe" && path != "/proc/" + BString::valueOf(id) + "/exe") {
            // :TODO: why does this need to be changed, seems like a bug
            args[0] = BString(Fs::getFullPath(currentDirectory, path)); // if path is a link, we should use the link not the actual path
            this->exe = path;
        } else {
            // :TODO: why does this need to be changed, seems like a bug
            args[0] = BString(Fs::getFullPath(currentDirectory, this->exe));
        }
    }
    if (loader.length()) {
        args.insert(args.begin(), loader);
    }    
    
    this->name = Fs::getFileNameFromPath(node->path);
    
    this->commandLine = BString::join("\0", args);
            
    this->path.clear();
    for (U32 i=0;i<envs.size();i++) {
        if (strncmp(envs[i].c_str(), "PATH=", 5)==0) {
            envs[i].substr(5).split(':', this->path);
        }
    }

    // reset memory must come after we grab the args and env
    this->memory->execvReset(cloneVM);
    cloneVM = false;

    thread->reset();
    this->onExec(thread);

    // not sure why x64 doesn't catch setting the CS segment this in time
    if (this->name.contains("winevdm.exe", true) || vectorContainsIgnoreCase(args, B("winevdm.exe"))) {
        for (int i=0;i<6;i++) {
            this->hasSetSeg[i] = true;
            this->hasSetStackMask = true;
        }
#ifdef BOXEDWINE_BINARY_TRANSLATOR
        this->emulateFPU = true;
#endif
    }

    if (!ElfLoader::loadProgram(thread, openNode, &thread->cpu->eip.u32)) {
        // :TODO: maybe alloc a new memory object and keep the old one until we know we are loaded
        kpanic("program failed to load, but memory was already reset");
    }	
    // must come after loadProgram because of process->phdr
    setupThreadStack(thread, thread->cpu, this->name, args, envs);
    openNode->close();
    delete openNode;

    BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(this->exitOrExecCond);
    BOXEDWINE_CONDITION_SIGNAL_ALL(this->exitOrExecCond);

    //klog("%d/%d exec %s (cwd=%s)", KThread::currentThread()->id, this->id, this->commandLine.c_str(), this->currentDirectory.c_str());

    thread->cpu->restart();
    
    return -K_CONTINUE;
}

void KProcess::signalProcess(U32 signal) {	
    U64 signalMask = ((U64)1 << (signal-1));
    {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(this->pendingSignalsMutex);
        this->pendingSignals |= signalMask;
    }

#ifdef BOXEDWINE_MULTI_THREADED
    // give each thread a chance to run a signal, some or all of them might have the signal masked off.  
    // In that case when the user unmasks the signal with sigprocmask it will be caught then
    iterateThreads([](KThread* thread) {
        if (thread->waitingCond) {
            BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(thread->waitingCondSync);
            if (thread->waitingCond) {
                thread->runSignals();
            }
        }
        return true;
    });
#else
    for (auto& t : this->threads) {
        KThread* thread = t.value;
        thread->runSignals();
    }
#endif
    {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(this->pendingSignalsMutex);
        if (this->pendingSignals & signalMask) {
            this->signalFd(nullptr, signal);
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
    signal(K_SIGALRM);
}

U32 KProcess::dup(U32 fildes) {
    KFileDescriptor* fd = this->getFileDescriptor(fildes);

    if (!fd) {
        return -K_EBADF;
    }
    return this->allocFileDescriptor(fd->kobject, fd->accessFlags, 0, -1, 0)->handle; // do not copy file descriptor flags
}

U32 KProcess::rmdir(BString path) {
    std::shared_ptr<FsNode> node = Fs::getNodeFromLocalPath(this->currentDirectory, path, false);

    if (!node)
        return -K_ENOENT;
    return node->removeDir();
}

U32 KProcess::mkdir(BString path) {
    std::shared_ptr<FsNode> node = Fs::getNodeFromLocalPath(this->currentDirectory, path, false);   
    if (node) {
        return -K_EEXIST;
    }
    BString fullpath = Fs::getFullPath(this->currentDirectory, path);
    BString parentPath = Fs::getParentPath(fullpath);
    node = Fs::getNodeFromLocalPath(B(""), parentPath, false); 
    if (!node) {
        return -K_ENOENT;
    }
    return Fs::makeLocalDirs(fullpath);
}

U32 KProcess::rename(BString from, BString to) {
    std::shared_ptr<FsNode> node = Fs::getNodeFromLocalPath(this->currentDirectory, from, false);    
    if (!node)
        return -K_ENOENT;
    BString fullPath = Fs::getFullPath(this->currentDirectory, to);
    return node->rename(fullPath);
}

U32 KProcess::renameat(FD olddirfd, BString from, FD newdirfd, BString to) {
    BString currentDirectory;
    U32 result = this->getCurrentDirectoryFromDirFD(olddirfd, currentDirectory);
    if (result)
        return result;

    std::shared_ptr<FsNode> node = Fs::getNodeFromLocalPath(currentDirectory, from, false);    
    if (!node)
        return -K_ENOENT;
    result = this->getCurrentDirectoryFromDirFD(newdirfd, currentDirectory);
    if (result)
        return result;
    BString fullPath = Fs::getFullPath(currentDirectory, to);
    return node->rename(fullPath);
}

static S32 internalAccess(std::shared_ptr<FsNode> node, U32 flags) {
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

U32 KProcess::access(BString path, U32 mode) {
    std::shared_ptr<FsNode> node = Fs::getNodeFromLocalPath(this->currentDirectory, path, true);
    return internalAccess(node, mode); 
}

U32 KProcess::lseek(FD fildes, S32 offset, U32 whence) {
    KFileDescriptor* fd = this->getFileDescriptor(fildes);
    S64 pos = 0;

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

U32 KProcess::chmod(BString path, U32 mode) {
    std::shared_ptr<FsNode> node = Fs::getNodeFromLocalPath(this->currentDirectory, path, true);
    if (!node)
        return -K_ENOENT;
    return 0;
}

U32 KProcess::chdir(BString path) {
    std::shared_ptr<FsNode> node = Fs::getNodeFromLocalPath(this->currentDirectory, path, true);
    if (!node)
        return -K_ENOENT;
    if (!node->isDirectory())
        return -K_ENOTDIR;
    if (path.startsWith('/')) {
        this->currentDirectory = path;
    } else {
        this->currentDirectory = this->currentDirectory+"/"+path;
    }
    if (this->currentDirectory.endsWith('/')) {
        this->currentDirectory = this->currentDirectory.substr(0, this->currentDirectory.length()-1);
    }
    return 0;
}

U32 KProcess::unlinkFile(BString path) {
    std::shared_ptr<FsNode> node = Fs::getNodeFromLocalPath(this->currentDirectory, path, false);
    if (!node) {
        return -K_ENOENT;
    }
    if (!node->remove()) {
        kwarn("failed to remove file: errno=%d", errno);
        return -K_EBUSY;
    }
    return 0;
}

U32 KProcess::link(BString from, BString to) {
    std::shared_ptr<FsNode> fromNode = Fs::getNodeFromLocalPath(this->currentDirectory, from, false);
    std::shared_ptr<FsNode> toNode = Fs::getNodeFromLocalPath(this->currentDirectory, to, false);

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

    std::shared_ptr<FsNode> toParentNode = Fs::getNodeFromLocalPath(this->currentDirectory, Fs::getParentPath(to), false);
    if (!toParentNode)
        return -K_ENOENT;

    toNode = Fs::addFileNode(to, B(""), Fs::getNativePathFromParentAndLocalFilename(toParentNode, Fs::getFileNameFromPath(to)), false, toParentNode);
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

U32 KProcess::open(BString path, U32 flags) {
    KFileDescriptor* fd = nullptr;
        
    U32 result = this->openFile(this->currentDirectory, path, flags, &fd);
    if (result || !fd) {
        return result;
    }
    return fd->handle;
}

U32 KProcess::read(KThread* thread, FD fildes, U32 bufferAddress, U32 bufferLen) {
    KFileDescriptor* fd = this->getFileDescriptor(fildes);
    if (!fd) {
        return -K_EBADF;
    }
    if (!fd->canRead()) {
        return -K_EINVAL;
    }
    return fd->kobject->read(thread, bufferAddress, bufferLen);
}

U32 KProcess::sendFile(U32 outFd, U32 inFd, U32 offset, U32 count) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(fdsMutex);
    KFileDescriptor* fdOut = getFileDescriptor(outFd);
    KFileDescriptor* fdIn = getFileDescriptor(inFd);

    if (!fdOut || !fdIn) {
        return -K_EBADFD;
    }
    U64 pos;

    if (offset) {
        pos = fdIn->kobject->getPos();
        fdIn->kobject->seek(memory->readq(offset));
    } 
    
    U8 buffer[1024];
    while (count) {
        U32 todo = count;
        if (todo > 1024) {
            todo = 1024;
        }
        U32 read = (S32)fdIn->kobject->readNative(buffer, todo);
        if ((S32)read > 0) {
            fdOut->kobject->writeNative(buffer, read);
            count -= read;
        } else if (read == 0) {
            break;
        } else {
            return -K_EIO;
        }
    }
    if (offset) {
        memory->writeq(offset, fdIn->kobject->getPos());
        fdIn->kobject->seek(pos);
    }
    return 0;
}

U32 KProcess::write(KThread* thread, FD fildes, U32 bufferAddress, U32 bufferLen) {
    KFileDescriptor* fd = this->getFileDescriptor(fildes);
    if (!fd) {
        return -K_EBADF;
    }
    if (!fd->canWrite()) {
        return -K_EINVAL;
    }
    return fd->kobject->write(thread, bufferAddress, bufferLen);
}

U32 KProcess::brk(KThread* thread, U32 address) {    
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
            if (memory->mmap(thread, aligned, len - alreadyAllocated, K_PROT_READ | K_PROT_WRITE | K_PROT_EXEC, K_MAP_PRIVATE|K_MAP_ANONYMOUS|K_MAP_FIXED, -1, 0)==aligned) {
                this->brkEnd+=len;
            }				
        }
    }
    return this->brkEnd; // intentional, should return the new end, even though the libc brk returns the old value
}

U32 KProcess::ioctl(KThread* thread, FD fildes, U32 request) {
    KFileDescriptor* fd = this->getFileDescriptor(fildes);

    if (fd == nullptr) {
        return -K_EBADF;
    }
    return fd->kobject->ioctl(thread, request);
}

U32 KProcess::umask(U32 umask) {	
    U32 old = this->umaskValue;
    this->umaskValue = umask;
    return old;
}

U32 KProcess::dup2(FD fildes, FD fildes2) {
    KFileDescriptor* fd = this->getFileDescriptor(fildes);

    if (!fd || fildes2<0) {
        return -K_EBADF;
    }
    if (fildes == fildes2) {
        return fildes;
    }
    KFileDescriptor* fd2 = this->getFileDescriptor(fildes2);
    if (fd2) {
        if (fd2->refCount>1) {
            kpanic("Not sure what to do on a dup2 where the refcount is %d", fd2->refCount);
        }
        fd2->close();
    } 
    this->allocFileDescriptor(fd->kobject, fd->accessFlags, 0, fildes2, 0); // do not copy file descriptor flags
    return fildes2;
}

U32 KProcess::getrusuage(KThread* thread, U32 who, U32 usage) {    
    U32 userSeconds = 0;
    U32 userMicroSeconds = 0;
    U32 kernelSeconds = 0;
    U32 kernelMicroSeconds = 0;

    if (who==0) { // RUSAGE_SELF
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(threadsMutex);
        for (auto& t : this->threads ) {
            KThread* otherThread = t.value;
            userSeconds += (U32)(otherThread->userTime / 1000000l);
            userMicroSeconds += (U32)(otherThread->userTime % 1000000l);
            kernelSeconds += (U32)(otherThread->kernelTime / 1000000l);
            kernelMicroSeconds += (U32)(otherThread->kernelTime % 1000000l);
        }
        
    } else if ((S32)who < 0) { // RUSAGE_CHILDREN
        klog("getrusuage: RUSAGE_CHILDREN not implemented");
    } else { // RUSAGE_THREAD
        userSeconds = (U32)(thread->userTime / 1000000l);
        userMicroSeconds = (U32)(thread->userTime % 1000000l);
        kernelSeconds = (U32)(thread->kernelTime / 1000000l);
        kernelMicroSeconds = (U32)(thread->kernelTime % 1000000l);
    }

    // user time
    memory->writed(usage, userSeconds);
    memory->writed(usage + 4, userMicroSeconds);
    // system time
    memory->writed(usage + 8, kernelSeconds);
    memory->writed(usage + 12, kernelMicroSeconds);
    memory->memset(usage+16, 0, 56);
    return 0;
}

U32 symlinkInDirectory(BString currentDirectory, BString target, BString linkpath) {
    std::shared_ptr<FsNode> node;

    node = Fs::getNodeFromLocalPath(currentDirectory, linkpath, false);
    if (node) {
        return -K_EEXIST;
    }
    BString fullPath = Fs::getFullPath(currentDirectory, linkpath);
    BString parentPath = Fs::getParentPath(fullPath);
    std::shared_ptr<FsNode> parentNode = Fs::getNodeFromLocalPath(B(""), parentPath, true);

    if (!parentNode) {
        return -K_ENOENT;
    }
    node = Fs::addFileNode(fullPath, target, Fs::getNativePathFromParentAndLocalFilename(parentNode, Fs::getFileNameFromPath(linkpath))+EXT_LINK, false, parentNode);

    if (!node->canWrite()) {
        node->removeNodeFromParent();
        return -K_EACCES;
    }
    FsOpenNode* openNode = node->open( K_O_WRONLY|K_O_CREAT);
    if (!openNode) {
        node->removeNodeFromParent();
        return -K_EIO;
    }
    openNode->writeNative((U8*)target.c_str(), (U32)target.length());
    openNode->close();
    return 0;
}

U32 KProcess::symlinkat(BString target, FD dirfd, BString linkpath) {
    BString currentDirectory;
    U32 result = this->getCurrentDirectoryFromDirFD(dirfd, currentDirectory);
    if (result)
        return result;
    return symlinkInDirectory(currentDirectory, target, linkpath);
}

U32 KProcess::symlink(BString target, BString linkpath) {
    return symlinkInDirectory(this->currentDirectory, target, linkpath);
}

U32 KProcess::readlinkInDirectory(BString currentDirectory, BString path, U32 buffer, U32 bufSize) {
    // :TODO: move these to the virtual filesystem
    std::shared_ptr<FsNode> node = Fs::getNodeFromLocalPath(currentDirectory, path, false);
    if (!node || !node->isLink())
        return -K_EINVAL;
    U32 len = (U32)node->getLink().length();
    if (len>bufSize)
        len = bufSize;
    memory->memcpy(buffer, node->getLink().c_str(), len);
    return len; 
}

U32 KProcess::readlinkat(FD dirfd, BString path, U32 buf, U32 bufsiz) {
    BString currentDirectory;
    U32 result = this->getCurrentDirectoryFromDirFD(dirfd, currentDirectory);
    if (result)
        return result;
    return readlinkInDirectory(currentDirectory, path, buf, bufsiz);
}

U32 KProcess::readlink(BString path, U32 buffer, U32 bufSize) {
    return readlinkInDirectory(this->currentDirectory, path, buffer, bufSize);
}

U32 KProcess::ftruncate64(FD fildes, U64 length) {
    KFileDescriptor* fd = this->getFileDescriptor(fildes);

    if (fd==nullptr) {
        return -K_EBADF;
    }
    if (fd->kobject->type!=KTYPE_FILE) {
        return -K_EINVAL;
    }
    std::shared_ptr<KFile> p = std::dynamic_pointer_cast<KFile>(fd->kobject);
    FsOpenNode* openNode = p->openFile;
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

U32 KProcess::statfs(BString path, U32 address) {
    std::shared_ptr<FsNode> node = Fs::getNodeFromLocalPath(this->currentDirectory, path, true);
    if (!node) {
        return -K_ENOENT;
    }
    memory->writed(address, 0xEF53); // f_type (EXT3)
    memory->writed(address + 4, FS_BLOCK_SIZE); // f_bsize
    memory->writed(address + 8, FS_SIZE / FS_BLOCK_SIZE); // f_blocks
    memory->writed(address + 16, FS_FREE_SIZE / FS_BLOCK_SIZE); // f_bfree
    memory->writed(address + 24, FS_FREE_SIZE / FS_BLOCK_SIZE); // f_bavail
    memory->writed(address + 32, 1024 * 1024); // f_files
    memory->writed(address + 40, 1024 * 1024); // f_ffree
    memory->writed(address + 48, 1278601602); // f_fsid
    memory->writed(address + 56, MAX_FILEPATH_LEN); // f_namelen
    memory->writed(address + 60, FS_BLOCK_SIZE); // f_frsize
    memory->writed(address + 64, 4096); // f_flags
    return 0;
}

U32 KProcess::statfs64(BString path, U32 address) {
    std::shared_ptr<FsNode> node = Fs::getNodeFromLocalPath(this->currentDirectory, path, true);
    if (!node) {
        return -K_ENOENT;
    }
    memory->writed(address, 0xEF53); // f_type (EXT3)
    memory->writed(address + 4, FS_BLOCK_SIZE); // f_bsize
    memory->writeq(address + 8, FS_SIZE / FS_BLOCK_SIZE); // f_blocks
    memory->writeq(address + 16, FS_FREE_SIZE / FS_BLOCK_SIZE); // f_bfree
    memory->writeq(address + 24, FS_FREE_SIZE / FS_BLOCK_SIZE); // f_bavail
    memory->writeq(address + 32, 1024 * 1024); // f_files
    memory->writeq(address + 40, 1024 * 1024); // f_ffree
    memory->writeq(address + 48, 1278601602); // f_fsid
    memory->writed(address + 56, MAX_FILEPATH_LEN); // f_namelen
    memory->writed(address + 60, FS_BLOCK_SIZE); // f_frsize
    memory->writed(address + 64, 4096); // f_flags
    return 0;
}

U32 KProcess::fstatfs64(FD fildes, U32 address) {
    KFileDescriptor* fd = this->getFileDescriptor(fildes);

    if (!fd) {
        return -K_EBADF;
    }
    memory->writed(address, 0xEF53); // f_type (EXT3)
    memory->writed(address + 4, FS_BLOCK_SIZE); // f_bsize
    memory->writeq(address + 8, FS_SIZE / FS_BLOCK_SIZE); // f_blocks
    memory->writeq(address + 16, FS_FREE_SIZE / FS_BLOCK_SIZE); // f_bfree
    memory->writeq(address + 24, FS_FREE_SIZE / FS_BLOCK_SIZE); // f_bavail
    memory->writeq(address + 32, 1024 * 1024); // f_files
    memory->writeq(address + 40, 1024 * 1024); // f_ffree
    memory->writeq(address + 48, 12719298601114463092ull); // f_fsid
    memory->writed(address + 56, MAX_FILEPATH_LEN); // f_namelen
    memory->writed(address + 60, FS_BLOCK_SIZE); // f_frsize
    memory->writed(address + 64, 4096); // f_flags
    return 0;
}

U32 KProcess::setitimer(U32 which, U32 newValue, U32 oldValue) {

    if (which != 0) { // ITIMER_REAL
        kpanic("setitimer which=%d not supported", which);
    }
    if (oldValue) {
        U32 remaining = this->timer.millies - KSystem::getMilliesSinceStart();

        memory->writed(oldValue, this->timer.resetMillies / 1000);
        memory->writed(oldValue, (this->timer.resetMillies % 1000) * 1000);
        memory->writed(oldValue + 8, remaining / 1000);
        memory->writed(oldValue + 12, (remaining % 1000) * 1000);
    }
    if (newValue) {
        U32 millies = memory->readd(newValue + 8) * 1000 + memory->readd(newValue + 12) / 1000;
        U32 resetMillies = memory->readd(newValue) * 1000 + memory->readd(newValue + 4) / 1000;

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

/**
 * struct clone_args - arguments for the clone3 syscall
 * @flags:        Flags for the new process as listed above.
 *                All flags are valid except for CSIGNAL and
 *                CLONE_DETACHED.
 * @pidfd:        If CLONE_PIDFD is set, a pidfd will be
 *                returned in this argument.
 * @child_tid:    If CLONE_CHILD_SETTID is set, the TID of the
 *                child process will be returned in the child's
 *                memory.
 * @parent_tid:   If CLONE_PARENT_SETTID is set, the TID of
 *                the child process will be returned in the
 *                parent's memory.
 * @exit_signal:  The exit_signal the parent process will be
 *                sent when the child exits.
 * @stack:        Specify the location of the stack for the
 *                child process.
 *                Note, @stack is expected to point to the
 *                lowest address. The stack direction will be
 *                determined by the kernel and set up
 *                appropriately based on @stack_size.
 * @stack_size:   The size of the stack for the child process.
 * @tls:          If CLONE_SETTLS is set, the tls descriptor
 *                is set to tls.
 * @set_tid:      Pointer to an array of type *pid_t. The size
 *                of the array is defined using @set_tid_size.
 *                This array is used to select PIDs/TIDs for
 *                newly created processes. The first element in
 *                this defines the PID in the most nested PID
 *                namespace. Each additional element in the array
 *                defines the PID in the parent PID namespace of
 *                the original PID namespace. If the array has
 *                less entries than the number of currently
 *                nested PID namespaces only the PIDs in the
 *                corresponding namespaces are set.
 * @set_tid_size: This defines the size of the array referenced
 *                in @set_tid. This cannot be larger than the
 *                kernel's limit of nested PID namespaces.
 * @cgroup:       If CLONE_INTO_CGROUP is specified set this to
 *                a file descriptor for the cgroup.
 *
 * The structure is versioned by size and thus extensible.
 * New struct members must go at the end of the struct and
 * must be properly 64bit aligned.
 
struct clone_args {
    __aligned_u64 flags;
    __aligned_u64 pidfd;
    __aligned_u64 child_tid;
    __aligned_u64 parent_tid;
    __aligned_u64 exit_signal;
    __aligned_u64 stack;
    __aligned_u64 stack_size;
    __aligned_u64 tls;
    __aligned_u64 set_tid;
    __aligned_u64 set_tid_size;
    __aligned_u64 cgroup;
};
*/
// :TODO: leaving in for now, but currently clone3 syscall returns ENOSYS and glibc will fall back to the normal clone, 
// I'm not sure why this doesn't work, seems like eip might be in a different place
U32 KProcess::clone3(KThread* thread, U32 args, U32 size) {
    U64 flags = memory->readq(args);
    U64 pidfd = memory->readq(args+8);
    U64 child_tid = memory->readq(args + 16);
    U64 parent_tid = memory->readq(args + 24);
    U64 exit_signal = memory->readq(args + 32);
    U64 stack = memory->readq(args + 40);
    U64 stack_size = memory->readq(args + 48);
    U64 tls = memory->readq(args + 56);
    U64 set_tid = memory->readq(args + 64);
    U64 set_tid_size = memory->readq(args + 72);
    U64 cgroup = memory->readq(args + 80);
    return clone(thread, (U32)flags, (U32)(stack + stack_size), (U32)parent_tid, (U32)tls, (U32)child_tid);
}

U32 KProcess::clone(KThread* thread, U32 flags, U32 child_stack, U32 ptid, U32 tls, U32 ctid) {    
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
        newProcess->memory = KMemory::create(newProcess.get());
        newProcess->memory->clone(this->memory, vm);

        KThread* newThread = newProcess->createThread();

        newProcess->parentId = this->id;        
        newProcess->cloneVM = vm;
        newProcess->clone(shared_from_this());
        newThread->clone(thread);

        // will only create them if they are missing
        //newProcess->initStdio();
        
        if ((flags & K_CLONE_CHILD_SETTID)!=0) {
            if (ctid!=0) {
                ChangeThread c(newThread); // so that writed will go to the new memory space
                newThread->memory->writed(ctid, newThread->id);
            }
        }
        if ((flags & K_CLONE_CHILD_CLEARTID)!=0) {
            newThread->clear_child_tid = ctid;
        }
        if ((flags & K_CLONE_PARENT_SETTID)!=0) {
            if (ptid) {                
                memory->writed(ptid, newThread->id);
                ChangeThread c(newThread); // so that writed will go to the new memory space
                newThread->memory->writed(ptid, newThread->id);
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
            thread->cpu->eip.u32+=2; // don't re-enter
            thread->cpu->reg[0].u32 = newProcess->id;
#endif
            {
                BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(newProcess->exitOrExecCond);
                scheduleThread(newThread);
                BOXEDWINE_CONDITION_WAIT(newProcess->exitOrExecCond);
            }
#ifdef BOXEDWINE_MULTI_THREADED
			if (thread->terminating) {
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

        memory->memcpy(&desc, tls, sizeof(struct user_desc));

        if (desc.base_addr!=0 && desc.entry_number!=0) {
            struct user_desc* ldt = newThread->getLDT(desc.entry_number);
            *ldt = desc;
            newThread->cpu->setSegment(GS, desc.entry_number << 3);
        }
        newThread->clear_child_tid = ctid;
        memory->writed(ptid, newThread->id);
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

void KProcess::killAllThreads(KThread* exceptThisThread) {
    iterateThreadIds([this, exceptThisThread](U32 id) {
        if (!exceptThisThread || exceptThisThread->id != id) {
            terminateOtherThread(shared_from_this(), id);
        }
        return true;
        });
}

U32 KProcess::exitgroup(KThread* thread, U32 code) {
    std::shared_ptr<KProcess> parent = KSystem::getProcess(this->parentId);
    if (parent && parent->sigActions[K_SIGCHLD].handlerAndSigAction!=K_SIG_DFL) {
        if (parent->sigActions[K_SIGCHLD].handlerAndSigAction!=K_SIG_IGN) {
            parent->signalCHLD(K_CLD_EXITED, this->id, this->userId, code);
        }
    }

    killAllThreads(thread);

    {
        BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(KSystem::processesCond);
        this->terminated = true;
    }

    thread->cleanup(); // must happen before we clear memory
    this->threads.clear();
    if (cloneVM) {
        // make sure the shared memory is unhooked from parent
        this->memory->execvReset(cloneVM);
    }
    this->cleanupProcess(); // release RAM, sockets, etc now.  No reason to wait to do that until waitpid is called
    this->exitCode = code;

    {
        BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(this->exitOrExecCond);
        BOXEDWINE_CONDITION_SIGNAL_ALL(this->exitOrExecCond);
    }
    if (KSystem::getProcessCount()==1) {        
        // no one left to wait on this process, with no processes running main will exit boxedwine
        KSystem::eraseProcess(this->id);
    }  

    {
        BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(KSystem::processesCond);
        KSystem::wakeThreadsWaitingOnProcessStateChanged(); // after this the process could be deleted
    }
     
	terminateCurrentThread(thread);
    return -K_CONTINUE;
}

U32 KProcess::fchdir(FD fildes) {
    KFileDescriptor* fd = this->getFileDescriptor(fildes);

    if (fd==nullptr) {
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
    if (this->currentDirectory.endsWith("/")) {
        this->currentDirectory = this->currentDirectory.substr(0, this->currentDirectory.length()-1);
    }
    return 0;
}

S64 KProcess::llseek(FD fildes, S64 offset, U32 whence) {
    KFileDescriptor* fd = this->getFileDescriptor(fildes);
    S64 pos = 0;

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

U32 writeRecord(KMemory* memory, U32 dirp, U32 len, U32 count, U32 pos, bool is64, const char* name, U32 id, U32 type) {
    U32 recordLen = 0;

    if (is64) {
        recordLen = 20+(U32)strlen(name);
        recordLen=(recordLen+3) / 4 * 4;
        if (recordLen+len>count) {
            if (len==0)
                return -K_EINVAL;
            return 0;
        }
        memory->writeq(dirp, id);
        memory->writeq(dirp + 8, pos);
        memory->writew(dirp + 16, recordLen);
        memory->writeb(dirp + 18, type);
        memory->strcpy(dirp + 19, name);
    } else {
        recordLen = 12+(U32)strlen(name);
        recordLen=(recordLen+3) / 4 * 4;
        if (recordLen+len>count) {
            if (len==0)
                return -K_EINVAL;
            return 0;
        }
        memory->writed(dirp, id);
        memory->writed(dirp + 4, pos);
        memory->writew(dirp + 8, recordLen);
        memory->strcpy(dirp + 10, name);
        memory->writeb(dirp + recordLen - 1, type);
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
    std::shared_ptr<FsNode> node = p->openFile->node;
    FsOpenNode* openNode = p->openFile;

    if (!node->isDirectory()) {
        return -K_ENOTDIR;
    }
    U32 entries = openNode->getDirectoryEntryCount();
    U32 len = 0;

    for (U32 i=(U32)openNode->getFilePointer();i<entries;i++) {
        BString name;
        std::shared_ptr<FsNode> entry = openNode->getDirectoryEntry(i, name);
        U32 recordLen = writeRecord(memory, dirp, len, count, i + 2, is64, name.c_str(), entry->id, entry->getType(true));
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

U32 KProcess::msync(KThread* thread, U32 addr, U32 len, U32 flags) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(mappedFilesMutex);
    for (auto& n : this->mappedFiles) {
        std::shared_ptr<MappedFile> m = n.value;
        if (m->address<=addr && addr+len<m->address+m->len) {
            return m->file->pwrite(thread, addr, addr - m->address + m->offset, len);
        }
    }
    return -K_ENOMEM;
}

U32 KProcess::writev(KThread* thread, FD handle, U32 iov, S32 iovcnt) {
    KFileDescriptor* fd = this->getFileDescriptor(handle);

    if (fd==nullptr) {
        return -K_EBADF;
    }
    if (!fd->canWrite()) {
        return -K_EINVAL;
    }
    return fd->kobject->writev(thread, iov, iovcnt);    
}

U32 KProcess::memfd_create(BString name, U32 flags) {
    std::shared_ptr<FsMemNode> node = std::make_shared<FsMemNode>(1, 1, name);
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

U32 KProcess::prctl(U32 option, U32 arg2) {
    if (option == 15) { // PR_SET_NAME
        this->name = memory->readString(arg2);
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
        this->sigActions[sig].writeSigAction(memory, oact, sigsetSize);
    }
    if (act!=0) {
        this->sigActions[sig].readSigAction(memory, act, sigsetSize);
    }
    return 0;
}

U32 KProcess::pread64(KThread* thread, FD fildes, U32 address, U32 len, U64 offset) {
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
    if (!this->memory->canWrite(address, len)) {
        return -K_EFAULT;
    }
    return p->pread(thread, address, (S64)offset, len);
}

U32 KProcess::pwrite64(KThread* thread, FD fildes, U32 address, U32 len, U64 offset) {
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
    if (!this->memory->canRead(address, len)) {
        return -K_EFAULT;
    }
    return p->pwrite(thread, address, (S64)offset, len);
}

U32 KProcess::getcwd(U32 buffer, U32 size) {
    if (size==0) {
        return -K_EINVAL;
    }
    if (!this->memory->canWrite(buffer, size)) {
        return -K_EFAULT;
    }
    std::shared_ptr<FsNode> node = Fs::getNodeFromLocalPath(B(""), this->currentDirectory, true);
    if (!node || !node->isDirectory()) {
        return -K_ENOENT;
    }
    if ((U32)this->currentDirectory.length()+1>size)
        return -K_ERANGE;
    memory->strcpy(buffer, this->currentDirectory.c_str());
    return (U32)this->currentDirectory.length()+1;
}

U32 KProcess::stat64(BString path, U32 buffer) {
    bool isLink = false;
    std::shared_ptr<FsNode> node = Fs::getNodeFromLocalPath(this->currentDirectory, path, true, &isLink);
    if (!node) {
        return -K_ENOENT;
    }
    U64 len = node->length();
    KSystem::writeStat(this, node->path, buffer, true, 1, node->id, node->getMode(), node->rdev, len, 4096, (len + 4095) / 4096, node->lastModified(), node->getHardLinkCount());
    return 0;
}

U32 KProcess::lstat64(BString path, U32 buffer) {
    std::shared_ptr<FsNode> node = Fs::getNodeFromLocalPath(this->currentDirectory, path, false);
    if (!node) {
        return -K_ENOENT;
    }
 
    U64 len = 0;
    U32 mode = 0;

    if (node->isLink()) {
        len = node->getLink().length();
        mode = K__S_IFLNK | (node->getMode() & 0xFFF);
    } else {
        len = node->length();
        mode = node->getMode();
    }
    KSystem::writeStat(this, node->path, buffer, true, 1, node->id, mode, node->rdev, len, 4096, (len + 4095) / 4096, node->lastModified(), node->getHardLinkCount());
    return 0;
}

U32 KProcess::fstat64(FD handle, U32 buf) {
    KFileDescriptor* fd = this->getFileDescriptor(handle);
    
    if (!fd) {
        return -K_EBADF;
    }
    return fd->kobject->stat(this, buf, true);
}

U32 KProcess::mincore(U32 address, U32 length, U32 vec) {
    U32 pages = (length+K_PAGE_SIZE+1)/K_PAGE_SIZE;
    U32 page = address >> K_PAGE_SHIFT;

    for (U32 i=0;i<pages;i++) {
        if (page+i>=K_NUMBER_OF_PAGES)
            return -K_ENOMEM;
        if (this->memory->isPageAllocated(page+i))
            memory->writeb(vec, 1);
        else
            memory->writeb(vec, 0);
        vec++;
    }
    return 0;
}

U32 KProcess::fcntrl(KThread* thread, FD fildes, U32 cmd, U32 arg) {
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
                lock.readFileLock(thread, arg, cmd==K_F_GETLK64);
                result = fd->kobject->getLock(&lock);
                if (!result) {
                    memory->writew(arg, K_F_UNLCK);
                } else {
                    result->writeFileLock(thread, arg, K_F_GETLK64 == cmd);
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

                lock.readFileLock(thread, arg, cmd == K_F_SETLK64 || cmd == K_F_SETLKW64);
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
                if (f->openFile->node->type==FsNode::Type::Memory) {
                    FsMemOpenNode* openNode = (FsMemOpenNode*)f->openFile;
                    return openNode->addSeals(arg);
                }
            }
            return -K_EINVAL;
        }
        case K_F_GET_SEALS: {
            if (fd->kobject->type==KTYPE_FILE) {
                std::shared_ptr<KFile> f = std::dynamic_pointer_cast<KFile>(fd->kobject);
                if (f->openFile->node->type==FsNode::Type::Memory) {
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

U32 KProcess::set_thread_area(KThread* thread, U32 info) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(usedTlsMutex);
    struct user_desc desc;    

    memory->memcpy((U8*)&desc, info, sizeof(struct user_desc));
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
        memory->memcpy(info, (U8*)&desc, sizeof(struct user_desc));
    }
    if (desc.base_addr!=0) {
        if (desc.entry_number<TLS_ENTRY_START_INDEX || desc.entry_number>=TLS_ENTRIES+TLS_ENTRY_START_INDEX) {
            return -K_ESRCH;
        }
        this->usedTLS[desc.entry_number-TLS_ENTRY_START_INDEX]=1;

        thread->setTLS(&desc);
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
    return p->ctl(memory, op, fd, address);
}

U32 KProcess::epollwait(KThread* thread, FD epfd, U32 events, U32 maxevents, U32 timeout) {
    KFileDescriptor* epollFD = this->getFileDescriptor(epfd);
    if (!epollFD) {
        return -K_EBADF;
    }
    if (epollFD->kobject->type != KTYPE_EPOLL) {
        return -K_EINVAL;
    }
    std::shared_ptr<KEPoll> p = std::dynamic_pointer_cast<KEPoll>(epollFD->kobject);
    return p->wait(thread, events, maxevents, timeout);
}

U32 KProcess::utimes(BString path, U32 times) {
    std::shared_ptr<FsNode> node = Fs::getNodeFromLocalPath(this->currentDirectory, path, true);

    if (!node) {
        return -K_ENOENT;
    } else {        
        U64 lastAccessTime = 0;
        U32 lastAccessTimeNano = 0;
        U64 lastModifiedTime =  0;
        U32 lastModifiedTimeNano = 0;
        if (!times) {
            lastAccessTime = time(nullptr);
            lastModifiedTime = time(nullptr);
        } else {
            lastAccessTime = memory->readd(times);
            lastAccessTimeNano = memory->readd(times+4)*1000;
            lastModifiedTime = memory->readd(times+8);
            lastModifiedTimeNano = memory->readd(times+12)*1000;
        }
        return node->setTimes(lastAccessTime, lastAccessTimeNano, lastModifiedTime, lastModifiedTimeNano);
    }
}

U32 KProcess::getCurrentDirectoryFromDirFD(FD dirfd, BString& currentDirectory) {
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

U32 KProcess::openat(FD dirfd, BString path, U32 flags) {
    BString dir;
    U32 result = 0;
    
    if (path.charAt(0)!='/')
        result = getCurrentDirectoryFromDirFD(dirfd, dir);

    if (result)
        return result;
    KFileDescriptor* fd = nullptr;
    result = this->openFile(dir, path, flags, &fd);
    if (result || !fd) {
        return result;
    }
    return fd->handle;
}

U32 KProcess::mkdirat(U32 dirfd, BString path, U32 mode) {
    BString dir;
    U32 result = 0;
    
    if (path.charAt(0) != '/')
        result = getCurrentDirectoryFromDirFD(dirfd, dir);

    if (result)
        return result;
    BString fullPath = Fs::getFullPath(dir, path);
    return this->mkdir(fullPath);
}

#define K_AT_SYMLINK_FOLLOW	0x400   // Follow symbolic links. 
#define K_AT_NO_AUTOMOUNT		0x800	// Suppress terminal automount traversal 
#define K_AT_EMPTY_PATH		0x1000	
/*
 
struct statx_timestamp {
        __s64        tv_sec;
        __u32        tv_nsec;
        __s32        __reserved;
};

struct statx {
    __u32 stx_mask;        // Mask of bits indicating
                           // filled fields 
    __u32 stx_blksize;     // Block size for filesystem I/O 
    __u64 stx_attributes;  // Extra file attribute indicators 
    __u32 stx_nlink;       // Number of hard links 
    __u32 stx_uid;         // User ID of owner 
    __u32 stx_gid;         // Group ID of owner 
    __u16 stx_mode;        // File type and mode 
    __u64 stx_ino;         // Inode number 
    __u64 stx_size;        // Total size in bytes 
    __u64 stx_blocks;      // Number of 512B blocks allocated 
    __u64 stx_attributes_mask;
    // Mask to show what's supported
    // in stx_attributes 

       // The following fields are file timestamps
    struct statx_timestamp stx_atime;  // Last access
    struct statx_timestamp stx_btime;  // Creation
    struct statx_timestamp stx_ctime;  // Last status change
    struct statx_timestamp stx_mtime;  // Last modification

    // If this file represents a device, then the next two
    // fields contain the ID of the device
    __u32 stx_rdev_major;  // Major ID
    __u32 stx_rdev_minor;  // Minor ID

    // The next two fields contain the ID of the device
    // containing the filesystem where the file resides 
    __u32 stx_dev_major;   // Major ID 
    __u32 stx_dev_minor;   // Minor ID 

    __u64 stx_mnt_id;      // Mount ID 

    // Direct I/O alignment restrictions 
    __u32 stx_dio_mem_align;
    __u32 stx_dio_offset_align;
};

#define STATX_TYPE		0x00000001U	// Want/got stx_mode & S_IFMT
#define STATX_MODE		0x00000002U	// Want/got stx_mode & ~S_IFMT
#define STATX_NLINK		0x00000004U	// Want/got stx_nlink
#define STATX_UID		0x00000008U	// Want/got stx_uid
#define STATX_GID		0x00000010U	// Want/got stx_gid
#define STATX_ATIME		0x00000020U	// Want/got stx_atime
#define STATX_MTIME		0x00000040U	// Want/got stx_mtime
#define STATX_CTIME		0x00000080U	// Want/got stx_ctime
#define STATX_INO		0x00000100U	// Want/got stx_ino
#define STATX_SIZE		0x00000200U	// Want/got stx_size
#define STATX_BLOCKS		0x00000400U	// Want/got stx_blocks
#define STATX_BASIC_STATS	0x000007ffU	// The stuff in the normal stat struct 
#define STATX_BTIME		0x00000800U	// Want/got stx_btime 
#define STATX_MNT_ID		0x00001000U	// Got stx_mnt_id 
#define STATX_DIOALIGN		0x00002000U	// Want/got direct I/O alignment info 

*/
U32 KProcess::statx(FD dirfd, BString path, U32 flags, U32 mask, U32 buf) {
    BString dir;
    U32 result = 0;

    if (path.endsWith("/") || (flags & K_AT_EMPTY_PATH)) {
        result = getCurrentDirectoryFromDirFD(dirfd, dir);
        if (result == 0 && (flags & K_AT_EMPTY_PATH)) {
            path = dir;
            dir = BString::empty;
        }
    }
    if (result)
        return result;
    bool isLink = false;

    std::shared_ptr<FsNode> node = Fs::getNodeFromLocalPath(dir, path, (flags & 0x100) == 0, &isLink);
    if (!node) {
        return -K_ENOENT;
    }

    U64 len = node->length();
    U32 mode = node->getMode();
    if (node->isLink() || isLink) {
        mode |= K__S_IFLNK;
    }
    U64 t = node->lastModified();
    U64 seconds = t / 1000;
    U32 n = (U32)(t % 1000) * 1000000;

    // 0x00
    memory->writed(buf, 0xfff); buf += 4; // stx_mask
    memory->writed(buf, 512); buf += 4; // stx_blksize
    memory->writeq(buf, 0); buf += 8; // stx_attributes
    // 0x10
    memory->writed(buf, node->getHardLinkCount()); buf += 4; // stx_nlink
    memory->writed(buf, userId); buf += 4; // stx_uid
    memory->writed(buf, groupId); buf += 4; // stx_gid
    memory->writew(buf, mode); buf += 2; // stx_mode
    memory->writew(buf, 0); buf += 2; // 
    // 0x20
    memory->writeq(buf, node->id); buf += 8; // stx_ino
    memory->writeq(buf, len); buf += 8; // stx_size
    memory->writeq(buf, (len+511)/512); buf += 8; // stx_blocks
    memory->writeq(buf, 0); buf += 8; // stx_attributes_mask
    // 0x40
    memory->writeq(buf, seconds); buf += 8; // stx_atime
    memory->writed(buf, n); buf += 4;
    memory->writed(buf, 0); buf += 4;
    memory->writeq(buf, seconds); buf += 8; // stx_btime
    memory->writed(buf, n); buf += 4;
    memory->writed(buf, 0); buf += 4;
    memory->writeq(buf, seconds); buf += 8; // stx_ctime
    memory->writed(buf, n); buf += 4;
    memory->writed(buf, 0); buf += 4;
    memory->writeq(buf, seconds); buf += 8; // stx_mtime
    memory->writed(buf, n); buf += 4;
    memory->writed(buf, 0); buf += 4;
    // 0x80
    memory->writed(buf, node->rdev); buf += 4; // stx_rdev_major
    memory->writed(buf, 0); buf += 4; // stx_rdev_minor
    memory->writed(buf, 1); buf += 4; // stx_dev_major
    memory->writed(buf, 0); buf += 4; // stx_dev_minor
    // 0x90
    memory->writeq(buf, 1); buf += 8; // stx_mnt_id
    memory->writed(buf, 0); buf += 4; // stx_dio_mem_align
    memory->writed(buf, 0);           // stx_dio_offset_align
    return 0;
}

U32 KProcess::fstatat64(FD dirfd, BString path, U32 buf, U32 flag) {
    BString dir;
    U32 result = 0;
    
    if (path.charAt(0) != '/')
        result = getCurrentDirectoryFromDirFD(dirfd, dir);

    if (result)
        return result;
    bool isLink = false;

    std::shared_ptr<FsNode> node = Fs::getNodeFromLocalPath(dir, path, (flag & 0x100)==0, &isLink);
    if (!node) {
        return -K_ENOENT;
    }

    U64 len = node->length();
    U32 mode = node->getMode();
    if (node->isLink() || isLink) {
        mode|=K__S_IFLNK;
    }
    
    KSystem::writeStat(this, path, buf, true, 1, node->id, mode, node->rdev, len, 4096, (len + 4095) / 4096, node->lastModified(), node->getHardLinkCount());
    return 0;    
}

U32 KProcess::unlinkat(FD dirfd, BString path, U32 flags) {
    BString dir;
    U32 result = 0;
    
    if (path.charAt(0) != '/')
        result = getCurrentDirectoryFromDirFD(dirfd, dir);

    if (result)
        return result;
    std::shared_ptr<FsNode> node = Fs::getNodeFromLocalPath(dir, path, false);
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

U32 KProcess::faccessat(U32 dirfd, BString path, U32 mode, U32 flags) {    
    BString dir;
    U32 result = 0;
    
    if (path.charAt(0) != '/')
        result = getCurrentDirectoryFromDirFD(dirfd, dir);

    if (result)
        return result;
    std::shared_ptr<FsNode> node = Fs::getNodeFromLocalPath(dir, path, (flags & 0x100)==0);
    if (!node) {
        return -K_ENOENT;
    }  
    return internalAccess(node, mode);
}

#define K_UTIME_NOW 0x3fffffff
#define K_UTIME_OMIT 0x3ffffffe

U32 KProcess::utimesat(FD dirfd, BString path, U32 times, U32 flags, bool time64) {
    BString dir;
    U32 result = 0;
    
    if (path.charAt(0) != '/')
        result = getCurrentDirectoryFromDirFD(dirfd, dir);

    if (result)
        return result;
    std::shared_ptr<FsNode> node = Fs::getNodeFromLocalPath(dir, path, (flags & 0x100)==0);
    if (!node) {
        return -K_ENOENT;
    } 
    U64 lastAccessTime = 0;
    U32 lastAccessTimeNano = 0;
    U64 lastModifiedTime =  0;
    U32 lastModifiedTimeNano = 0;

    if (times) {
        if (time64) {
            lastAccessTime = memory->readd(times);
            lastAccessTimeNano = memory->readd(times + 8);
            lastModifiedTime = memory->readd(times + 12);
            lastModifiedTimeNano = memory->readd(times + 20);
        } else {
            lastAccessTime = memory->readd(times);
            lastAccessTimeNano = memory->readd(times + 4);
            lastModifiedTime = memory->readd(times + 8);
            lastModifiedTimeNano = memory->readd(times + 12);
        }
    }
    if (lastAccessTimeNano != K_UTIME_OMIT) {
        if (lastAccessTimeNano == K_UTIME_NOW) {
            lastAccessTime = time(nullptr);
            lastAccessTimeNano = 0;
        }
    }
    if (lastModifiedTimeNano != K_UTIME_OMIT) {
        if (lastModifiedTimeNano == K_UTIME_NOW) {
            lastModifiedTime = (U32)time(nullptr);
            lastModifiedTimeNano = 0;
        }
    }
    return node->setTimes(lastAccessTime, lastAccessTimeNano, lastModifiedTime, lastModifiedTimeNano);
}

U32 KProcess::utimesat64(FD dirfd, BString path, U32 times, U32 flags) {
    BString dir;
    U32 result = 0;

    if (path.charAt(0) != '/')
        result = getCurrentDirectoryFromDirFD(dirfd, dir);

    if (result)
        return result;
    std::shared_ptr<FsNode> node = Fs::getNodeFromLocalPath(dir, path, (flags & 0x100) == 0);
    if (!node) {
        return -K_ENOENT;
    }
    U64 lastAccessTime = 0;
    U32 lastAccessTimeNano = 0;
    U64 lastModifiedTime = 0;
    U32 lastModifiedTimeNano = 0;

    if (times) {
        lastAccessTime = memory->readq(times);
        lastAccessTimeNano = memory->readd(times + 8);
        lastModifiedTime = memory->readq(times + 12);
        lastModifiedTimeNano = memory->readd(times + 20);
    }
    if (lastAccessTimeNano != K_UTIME_OMIT) {
        if (lastAccessTimeNano == K_UTIME_NOW) {
            lastAccessTime = time(nullptr);
            lastAccessTimeNano = 0;
        }
    }
    if (lastModifiedTimeNano != K_UTIME_OMIT) {
        if (lastModifiedTimeNano == K_UTIME_NOW) {
            lastModifiedTime = (U32)time(nullptr);
            lastModifiedTimeNano = 0;
        }
    }
    return node->setTimes(lastAccessTime, lastAccessTimeNano, lastModifiedTime, lastModifiedTimeNano);
}

U32 KProcess::timerfd_create(U32 clockid, U32 flags) {
    std::shared_ptr<KTimer> o = std::make_shared<KTimer>();
    KFileDescriptor* fd = allocFileDescriptor(o, K_O_RDONLY, 0, -1, 0);

    if (flags & K_O_CLOEXEC) {
        fd->descriptorFlags |= FD_CLOEXEC;
    }
    if (flags & K_O_NONBLOCK) {
        fd->accessFlags |= K_O_NONBLOCK;
        o->setBlocking(false);
    }
    return fd->handle;
}

U32 KProcess::timerfd_settime(U32 fildes, U32 flags, U32 newValue, U32 oldValue) {
    KFileDescriptor* fd = getFileDescriptor(fildes);
    if (!fd) {
        return -K_EBADF;
    }
    if (fd->kobject->type != KTYPE_SIGNAL) {
        return -K_EINVAL;
    }
    if (!newValue) {
        return -K_EFAULT;
    }
    std::shared_ptr<KTimer> timer = std::dynamic_pointer_cast<KTimer>(fd->kobject);
    if (oldValue) {
        U64 microNextTimer = timer->getMicroNextTimer();
        U64 microInterval = timer->getMicroInterval();

        memory->writed(oldValue, (U32)(microInterval / 1000000));
        memory->writed(oldValue + 4, (U32)((microInterval % 1000000) * 1000));

        if (!microNextTimer) {
            memory->writed(oldValue + 8, 0);
            memory->writed(oldValue + 12, 0);
        } else {
            U64 currentTime = KSystem::getSystemTimeAsMicroSeconds();
            S64 diff = (S64)microNextTimer - (S64)currentTime;
            if (diff < 0) {
                memory->writed(oldValue + 8, 0);
                memory->writed(oldValue + 12, 0);
            } else {
                memory->writed(oldValue + 8, (U32)(diff / 1000000));
                memory->writed(oldValue + 12, (U32)((diff % 1000000) * 1000));
            }
        }
    }
    U64 interval = memory->readd(newValue) * 1000000 + memory->readd(newValue+4) / 1000;
    U64 next = memory->readd(newValue+8) * 1000000 + memory->readd(newValue + 12) / 1000;
    // TFD_TIMER_ABSTIME
    if ((flags & 1) == 0) {
        next += KSystem::getSystemTimeAsMicroSeconds();
    }
    timer->setTimes(interval, next);
    return 0;
}

U32 KProcess::timerfd_gettime(U32 fildes, U32 value) {
    KFileDescriptor* fd = getFileDescriptor(fildes);
    if (!fd) {
        return -K_EBADF;
    }
    if (fd->kobject->type != KTYPE_SIGNAL) {
        return -K_EINVAL;
    }
    if (!value) {
        return -K_EFAULT;
    }
    std::shared_ptr<KTimer> timer = std::dynamic_pointer_cast<KTimer>(fd->kobject);
    U64 microNextTimer = timer->getMicroNextTimer();
    U64 microInterval = timer->getMicroInterval();

    memory->writed(value, (U32)(microInterval / 1000000));
    memory->writed(value + 4, (U32)((microInterval % 1000000) * 1000));

    if (!microNextTimer) {
        memory->writed(value + 8, 0);
        memory->writed(value + 12, 0);
    } else {
        U64 currentTime = KSystem::getSystemTimeAsMicroSeconds();
        S64 diff = (S64)microNextTimer - (S64)currentTime;
        if (diff < 0) {
            memory->writed(value + 8, 0);
            memory->writed(value + 12, 0);
        } else {
            memory->writed(value + 8, (U32)(diff / 1000000));
            memory->writed(value + 12, (diff % 1000000) * 1000);
        }
    }
    return 0;
}

user_desc* KProcess::getLDT(U32 index) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(ldtMutex);
    if (index < LDT_ENTRIES)
        return &this->ldt[index];
    return nullptr;
}

std::shared_ptr<SHM> KProcess::allocSHM(U32 key, U32 afterIndex) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(privateShmMutex);
    std::shared_ptr<SHM> result;

    while (this->privateShm.contains(afterIndex)) {
        if (afterIndex > 0x7FFFFFFF)
            kpanic("allocSHM ran out of indexes");
        afterIndex++;
    }
    result = std::make_shared<SHM>(afterIndex, key);
    this->privateShm.set(afterIndex, result);
    return result;
}

std::shared_ptr<SHM> KProcess::getSHM(U32 key) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(privateShmMutex);
    return this->privateShm[key];
}

void KProcess::attachSHM(U32 address, const std::shared_ptr<SHM>& shm) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(attachedShmMutex);
    std::shared_ptr<AttachedSHM> attached = std::make_shared<AttachedSHM>(shm, address, this->id);
    this->attachedShm.set(address, attached);
}

U32 KProcess::shmdt(U32 shmaddr) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(attachedShmMutex);

    std::shared_ptr<AttachedSHM> attached = this->attachedShm[shmaddr];
    if (attached) {
        memory->unmap(shmaddr, (U32)attached->shm->pages.size());
        this->attachedShm.remove(shmaddr);
        return 0;
    }
    return -K_EINVAL;
}

void KProcess::iterateThreadIds(std::function<bool(U32)> callback) {
    std::vector<U32> threadIds;

    {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(threadsMutex);

        for (auto& t : this->threads) {
            threadIds.push_back(t.key);
        }
    }
    for (auto& id : threadIds) {
        if (!callback(id)) {
            return;
        }
    }
}

void KProcess::iterateThreads(std::function<bool(KThread*)> callback) {
    std::vector<U32> threadIds;

    {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(threadsMutex);

        for (auto& t : this->threads) {
            threadIds.push_back(t.key);
        }
    }
    for (auto& id : threadIds) {
        KThread* thread = getThreadById(id);
        if (thread && !callback(thread)) {
            return;
        }
    }
}

void KProcess::printStack() {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(threadsMutex);
    for (auto& t : this->threads) {
        KThread* thread = t.value;
        CPU* cpu=thread->cpu;
        
        if (thread->waitingCond) {            
            klog("  thread %X WAITING %s", thread->id, thread->waitingCond->name.c_str());
        } else {
            klog("  thread %X RUNNING", thread->id);
            BString name = this->getModuleName(cpu->seg[CS].address+cpu->eip.u32);

            klog("    0x%08d %s", this->getModuleEip(cpu->seg[CS].address+cpu->eip.u32), name.length()?name.c_str():"Unknown");
        }
        cpu->walkStack(cpu->eip.u32, EBP, 6);
    }
}

U32 KProcess::signal(U32 signal) {
    if (signal == K_SIGKILL) {
        KThread* currentThread = KThread::currentThread();
        if (currentThread->process->id != id) {
            currentThread = nullptr;
        }
        this->killAllThreads(currentThread);
        if (currentThread) {
            terminateCurrentThread(currentThread);
        }
        return 0;
    }
    {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(threadsMutex);
        for (auto& t : this->threads) {
            KThread* thread = t.value;
            BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(thread->sigWaitCond);
            if (thread->sigWaitMask & signal) {
                thread->foundWaitSignal = signal;
                BOXEDWINE_CONDITION_SIGNAL(thread->sigWaitCond);
                return 0;
            }
        }
        for (auto& t : this->threads) {
            KThread* thread = t.value;

            if (((U64)1 << (signal - 1)) & ~(thread->inSignal ? thread->inSigMask : thread->sigMask)) {
                return thread->signal(signal, false);
            }
        }
    }
    // didn't find a thread that could handle it
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(this->pendingSignalsMutex);
    this->pendingSignals |= ((U64)1 << (signal-1));
    this->signalFd(nullptr, signal);
    return 0;
}

void KProcess::signalFd(KThread* thread, U32 signal) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(fdsMutex);
    for (auto& n : this->fds) {
        KFileDescriptor* fd = n.value;
        if (fd->kobject->type == KTYPE_SIGNAL) {
            std::shared_ptr<KSignal> p = std::dynamic_pointer_cast<KSignal>(fd->kobject);
            if ((p->mask & signal) && (!thread || thread->waitingCond == p->lockCond)) {
                BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(p->lockCond);
                p->sigAction = this->sigActions[signal];
                p->signalingPid = this->id;
                p->signalingUid = this->userId;
                BOXEDWINE_CONDITION_SIGNAL(p->lockCond);
            }
        }
    }
}

void KProcess::printMappedFiles() {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(mappedFilesMutex);
    for (auto& n : this->mappedFiles) {
        const std::shared_ptr<MappedFile>& mappedFile = n.value;
        klog("    %.8X - %.8X (offset=%x) %s\n", mappedFile->address, mappedFile->address+(int)mappedFile->len, (U32)mappedFile->offset, mappedFile->file->openFile->node->path.c_str());
    }
}