#include "boxedwine.h"

#include "kstat.h"

FsNode::FsNode(Type type, U32 id, U32 rdev, BString path, BString link, BString nativePath, bool isDirectory, std::shared_ptr<FsNode> parent) :
    path(path),
    nativePath(nativePath),
    name(Fs::getFileNameFromPath(path)),    
    link(link),    
    id(id), 
    rdev(rdev),
    hardLinkCount(1),
    type(type),  
    parent(parent),
    isDir(isDirectory),      
    hasLoadedChildrenFromFileSystem(false),
    locksCS(std::make_shared<BoxedWineCondition>(B("FsNode.lockCS")))
 {   
}

void FsNode::removeOpenNode(FsOpenNode* node) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(this->openNodesMutex);
    node->listNode.remove();
}

bool FsNode::canRead() {
    return (getMode() & K__S_IREAD)!=0;
}

bool FsNode::canWrite() {
    return (getMode() & K__S_IWRITE)!=0;
}

void FsNode::removeNodeFromParent() {
    std::shared_ptr<FsNode> parent = this->getParent().lock();
    if (parent) {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(parent->childrenByNameMutex);
        parent->childrenByName.remove(this->name);
    }
}

void FsNode::loadChildren() {
    if (!this->hasLoadedChildrenFromFileSystem) {
        BOXEDWINE_CRITICAL_SECTION;
        if (this->hasLoadedChildrenFromFileSystem) {
            return;
        }
        this->hasLoadedChildrenFromFileSystem = true;
        if (this->nativePath.length()) {
            std::vector<Platform::ListNodeResult> results;
            Platform::listNodes(nativePath, results);
            for (auto& n : results) {
                BString localPath = this->path;
                BString remotePath = this->nativePath ^ n.name;
                if (!localPath.endsWith("/")) {
                    localPath += "/";
                }
                Fs::remoteNameToLocal(n.name);
                localPath+=n.name;
                if (localPath.endsWith(EXT_MIXED)) {
                    localPath.remove(localPath.length() - 6);
                }
                if (localPath.endsWith(EXT_DOSATTRIB)) {
                    continue;
                }
                if (!localPath.endsWith(EXT_LINK)) {
                    Fs::addFileNode(localPath, B(""), remotePath, n.isDirectory, shared_from_this());
                } else {
                    U8 tmp[MAX_FILEPATH_LEN];
                    U32 result = Fs::readNativeFile(remotePath, tmp, MAX_FILEPATH_LEN-1);
                    tmp[result]=0;
                    if (result==0) {
                        kwarn("Could not read link file from filesystem: %s", localPath.c_str());
                    }
                    localPath = localPath.substr(0, localPath.length()-5);
                    Fs::addFileNode(localPath, BString::copy((const char*)tmp), remotePath, n.isDirectory, shared_from_this());
                }           
            }
        }
    }
}


std::shared_ptr<FsNode> FsNode::getChildByName(BString name) {
    this->loadChildren();
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(this->childrenByNameMutex);
    return this->childrenByName[name];
}

std::shared_ptr<FsNode> FsNode::getChildByNameIgnoreCase(BString name) {
    this->loadChildren();
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(this->childrenByNameMutex);
    for (auto& n : this->childrenByName) {
        if (n.key.compareTo(name, true) == 0) {
            return n.value;
        }
    }
    return nullptr;
}

U32 FsNode::getChildCount() {    
    this->loadChildren();
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(this->childrenByNameMutex);
    return (U32)this->childrenByName.size();
}

void FsNode::addChild(std::shared_ptr<FsNode> node) {
    this->loadChildren();
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(this->childrenByNameMutex);
    this->childrenByName.set(node->name, node);
}

void FsNode::removeChildByName(BString name) {
    this->loadChildren();
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(this->childrenByNameMutex);
    this->childrenByName.remove(name);
}

void FsNode::getAllChildren(std::vector<std::shared_ptr<FsNode> > & results) {
    this->loadChildren();
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(this->childrenByNameMutex);
    for (auto& n : this->childrenByName) {
        results.push_back(n.value);
    }
}

bool FsNode::unlock(KFileLock* lock) {
    BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(this->locksCS);
    KFileLock* found = internalGetLock(lock, false);
    bool result = false;

    while (found) {
        this->locks.erase(std::remove(locks.begin(), locks.end(), *found), locks.end());
        found = internalGetLock(lock, false);
        result = true;
    }
    if (result) {
        BOXEDWINE_CONDITION_SIGNAL(this->locksCS);
    }
    return result;
}

U32 FsNode::addLock(KFileLock* lock) {
    BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(this->locksCS);
    KFileLock* found = internalGetLock(lock, true);
    if (found) {
        return -K_EAGAIN;
    }
    this->locks.push_back(*lock);    
    return 0;
}

U32 FsNode::addLockAndWait(KFileLock* lock, bool otherProcess) {
    BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(this->locksCS);
    KFileLock* found = internalGetLock(lock, otherProcess);

    while (found) {
        BOXEDWINE_CONDITION_WAIT(this->locksCS);
        found = internalGetLock(lock, otherProcess);
    }
    this->locks.push_back(*lock);
    return 0;
}

bool FsNode::hasLock(U32 pid) {
    BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(this->locksCS);

    for (auto& n : this->locks) {
        KFileLock* next = &n;
        if (next->l_pid == pid) {
            return true;
        }
    }
    return false;
}

void FsNode::unlockAll(U32 pid) {
    BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(this->locksCS);
    auto it = std::remove_if(locks.begin(), locks.end(), [pid](auto&& item)
        { return (item.l_pid == pid); });
    locks.erase(it, locks.end());
    BOXEDWINE_CONDITION_SIGNAL(this->locksCS);
}

KFileLock* FsNode::getLock(KFileLock* lock, bool otherProcess) {
    BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(this->locksCS);
    return internalGetLock(lock, otherProcess);
}

KFileLock* FsNode::internalGetLock(KFileLock* lock, bool otherProcess) {    
    U64 l1 = lock->l_start;
    U64 l2 = l1+lock->l_len;
    U32 pid = KThread::currentThread()->process->id;

    if (lock->l_len == 0)
        l2 = 0xFFFFFFFF;
    for( auto& n : this->locks ) {
        KFileLock* next = &n;
        U64 s1 = next->l_start;
        U64 s2 = s1+next->l_len;
        
        if (next->l_len == 0)
            s2 = 0xFFFFFFFF;
        if (((s1>=l1 && s1<=l2) || (s2>=l1 && s2<=l2)) && ((otherProcess && next->l_pid != pid) || (!otherProcess && next->l_pid == pid))) {
            return next;
        }
    }
    return nullptr;
 }

void FsNode::addOpenNode(KListNode<FsOpenNode*>* node) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(this->openNodesMutex);
    this->openNodes.addToBack(node);
}
