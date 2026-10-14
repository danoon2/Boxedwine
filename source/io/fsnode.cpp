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

#include "boxedwine.h"

#include "fsfilenode.h"
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
    fileIdentity(std::make_shared<FsFileIdentity>()),
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

namespace {

// loadChildren() and the host folder refresh share this lock, so a directory is
// never synced while another thread is still loading it for the first time.
[[maybe_unused]] BOXEDWINE_MUTEX& loadChildrenMutex() {
    static BOXEDWINE_MUTEX mutex;
    return mutex;
}

enum class NativeEntryKind { Skip, Plain, Link, HardLink };

// Maps a native directory entry to the local path loadChildren() gives it.
// n.name is converted to its local form.
NativeEntryKind getNativeEntryLocalPath(const BString& parentPath, const BString& parentNativePath, Platform::ListNodeResult& n, BString& localPath, BString& remotePath) {
    localPath = parentPath;
    remotePath = parentNativePath.stringByApppendingPath(n.name);
    if (!localPath.endsWith("/")) {
        localPath += "/";
    }
    Fs::remoteNameToLocal(n.name);
    localPath+=n.name;
    if (localPath.endsWith(EXT_MIXED)) {
        localPath.remove(localPath.length() - 6);
    }
    if (localPath.endsWith(EXT_DOSATTRIB) || localPath.endsWith(EXT_WINEREPARSE) || localPath.endsWith(EXT_HARDLINK_BACKING)) {
        return NativeEntryKind::Skip;
    }
    if (localPath.endsWith(EXT_HARDLINK)) {
        localPath = localPath.substr(0, localPath.length() - BString(EXT_HARDLINK, true).length());
        return NativeEntryKind::HardLink;
    }
    if (localPath.endsWith(EXT_LINK)) {
        localPath = localPath.substr(0, localPath.length()-5);
        return NativeEntryKind::Link;
    }
    return NativeEntryKind::Plain;
}

void addChildFromNativeEntry(const std::shared_ptr<FsNode>& parent, NativeEntryKind kind, const Platform::ListNodeResult& n, const BString& localPath, const BString& remotePath) {
    if (kind == NativeEntryKind::HardLink) {
        std::shared_ptr<FsHardLinkState> state = FsFileNode::readHardLinkMetadata(remotePath);
        if (!state) {
            kwarn_fmt("Could not read hard link file from filesystem: %s", localPath.c_str());
            return;
        }
        BString visibleNativePath = remotePath.substr(0, remotePath.length() - BString(EXT_HARDLINK, true).length());
        std::shared_ptr<FsFileNode> node = Fs::addFileNode(localPath, B(""), visibleNativePath, false, parent);
        state->linkCount++;
        node->setHardLinkState(state);
    } else if (kind == NativeEntryKind::Link) {
        U8 tmp[MAX_FILEPATH_LEN];
        U32 result = Fs::readNativeFile(remotePath, tmp, MAX_FILEPATH_LEN-1);
        tmp[result]=0;
        if (result==0) {
            kwarn_fmt("Could not read link file from filesystem: %s", localPath.c_str());
            return;
        }
        Fs::addFileNode(localPath, BString::copy((const char*)tmp), remotePath, n.isDirectory, parent);
    } else if (kind == NativeEntryKind::Plain) {
        Fs::addFileNode(localPath, B(""), remotePath, n.isDirectory, parent);
    }
}

}

void FsNode::loadChildren() {
    if (!this->hasLoadedChildrenFromFileSystem) {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(loadChildrenMutex());
        if (this->hasLoadedChildrenFromFileSystem) {
            return;
        }
        this->hasLoadedChildrenFromFileSystem = true;
        if (this->nativePath.length()) {
            std::vector<Platform::ListNodeResult> results;
            Platform::listNodes(nativePath, results);
            std::shared_ptr<FsNode> self = shared_from_this();
            for (auto& n : results) {
                BString localPath;
                BString remotePath;
                NativeEntryKind kind = getNativeEntryLocalPath(this->path, this->nativePath, n, localPath, remotePath);
                addChildFromNativeEntry(self, kind, n, localPath, remotePath);
            }
        }
    }
}


std::shared_ptr<FsNode> FsNode::getChildByName(BString name) {
#ifdef BOXEDWINE_MULTI_THREADED
    std::unique_lock<std::recursive_mutex> visibilityLock;
    BOXEDWINE_MUTEX* visibilityMutex =
        childrenVisibilityMutex.load(std::memory_order_acquire);
    if (visibilityMutex) {
        visibilityLock = std::unique_lock<std::recursive_mutex>(*visibilityMutex);
    }
#endif
    this->loadChildren();
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(this->childrenByNameMutex);
    return this->childrenByName[name];
}

std::shared_ptr<FsNode> FsNode::getChildByNameIgnoreCase(BString name) {
#ifdef BOXEDWINE_MULTI_THREADED
    std::unique_lock<std::recursive_mutex> visibilityLock;
    BOXEDWINE_MUTEX* visibilityMutex =
        childrenVisibilityMutex.load(std::memory_order_acquire);
    if (visibilityMutex) {
        visibilityLock = std::unique_lock<std::recursive_mutex>(*visibilityMutex);
    }
#endif
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
#ifdef BOXEDWINE_MULTI_THREADED
    std::unique_lock<std::recursive_mutex> visibilityLock;
    BOXEDWINE_MUTEX* visibilityMutex =
        childrenVisibilityMutex.load(std::memory_order_acquire);
    if (visibilityMutex) {
        visibilityLock = std::unique_lock<std::recursive_mutex>(*visibilityMutex);
    }
#endif
    this->loadChildren();
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(this->childrenByNameMutex);
    return (U32)this->childrenByName.size();
}

void FsNode::addChild(std::shared_ptr<FsNode> node) {
#ifdef BOXEDWINE_MULTI_THREADED
    std::unique_lock<std::recursive_mutex> visibilityLock;
    BOXEDWINE_MUTEX* visibilityMutex =
        childrenVisibilityMutex.load(std::memory_order_acquire);
    if (visibilityMutex) {
        visibilityLock = std::unique_lock<std::recursive_mutex>(*visibilityMutex);
    }
#endif
    this->loadChildren();
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(this->childrenByNameMutex);
    this->childrenByName.set(node->name, node);
}

void FsNode::removeChildByName(BString name) {
#ifdef BOXEDWINE_MULTI_THREADED
    std::unique_lock<std::recursive_mutex> visibilityLock;
    BOXEDWINE_MUTEX* visibilityMutex =
        childrenVisibilityMutex.load(std::memory_order_acquire);
    if (visibilityMutex) {
        visibilityLock = std::unique_lock<std::recursive_mutex>(*visibilityMutex);
    }
#endif
    this->loadChildren();
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(this->childrenByNameMutex);
    this->childrenByName.remove(name);
}

void FsNode::getAllChildren(std::vector<std::shared_ptr<FsNode> > & results) {
#ifdef BOXEDWINE_MULTI_THREADED
    std::unique_lock<std::recursive_mutex> visibilityLock;
    BOXEDWINE_MUTEX* visibilityMutex =
        childrenVisibilityMutex.load(std::memory_order_acquire);
    if (visibilityMutex) {
        visibilityLock = std::unique_lock<std::recursive_mutex>(*visibilityMutex);
    }
#endif
    this->loadChildren();
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(this->childrenByNameMutex);
    for (auto& n : this->childrenByName) {
        results.push_back(n.value);
    }
}

void FsNode::reserveChildren(std::size_t capacity) {
#ifdef BOXEDWINE_MULTI_THREADED
    std::unique_lock<std::recursive_mutex> visibilityLock;
    BOXEDWINE_MUTEX* visibilityMutex =
        childrenVisibilityMutex.load(std::memory_order_acquire);
    if (visibilityMutex) {
        visibilityLock = std::unique_lock<std::recursive_mutex>(*visibilityMutex);
    }
#endif
    this->loadChildren();
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(this->childrenByNameMutex);
    this->childrenByName.reserve(capacity);
}

void FsNode::setChildrenVisibilityMutex(BOXEDWINE_MUTEX* mutex) {
    if (!trySetChildrenVisibilityMutex(mutex)) {
        kpanic("attempted to replace an FsNode child visibility mutex");
    }
}

bool FsNode::trySetChildrenVisibilityMutex(BOXEDWINE_MUTEX* mutex) {
#ifdef BOXEDWINE_MULTI_THREADED
    if (!mutex) {
        return childrenVisibilityMutex.load(std::memory_order_acquire) == nullptr;
    }
    BOXEDWINE_MUTEX* expected = nullptr;
    if (childrenVisibilityMutex.compare_exchange_strong(expected, mutex,
            std::memory_order_release, std::memory_order_acquire)) {
        return true;
    }
    return expected == mutex;
#else
    (void)mutex;
    return true;
#endif
}

#ifdef __TEST
bool FsNode::trySetChildrenVisibilityMutexForTest(BOXEDWINE_MUTEX* mutex) {
    return trySetChildrenVisibilityMutex(mutex);
}

BOXEDWINE_MUTEX* FsNode::getChildrenVisibilityMutexForTest() const {
#ifdef BOXEDWINE_MULTI_THREADED
    return childrenVisibilityMutex.load(std::memory_order_acquire);
#else
    return nullptr;
#endif
}
#endif

std::shared_ptr<FsNode> FsNode::getLoadedChildByNativePath(const BString& childNativePath) {
#ifdef BOXEDWINE_MULTI_THREADED
    std::unique_lock<std::recursive_mutex> visibilityLock;
    BOXEDWINE_MUTEX* visibilityMutex =
        childrenVisibilityMutex.load(std::memory_order_acquire);
    if (visibilityMutex) {
        visibilityLock = std::unique_lock<std::recursive_mutex>(*visibilityMutex);
    }
#endif
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(loadChildrenMutex());
    if (!this->hasLoadedChildrenFromFileSystem) {
        return nullptr;
    }
    std::vector<std::shared_ptr<FsNode> > children;
    this->getAllChildren(children);
    for (auto& child : children) {
        if (child->nativePath == childNativePath) {
            return child;
        }
    }
    return nullptr;
}

void FsNode::syncLoadedChildrenWithFileSystem() {
#ifdef BOXEDWINE_MULTI_THREADED
    std::unique_lock<std::recursive_mutex> visibilityLock;
    BOXEDWINE_MUTEX* visibilityMutex =
        childrenVisibilityMutex.load(std::memory_order_acquire);
    if (visibilityMutex) {
        visibilityLock = std::unique_lock<std::recursive_mutex>(*visibilityMutex);
    }
#endif
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(loadChildrenMutex());
    if (!this->hasLoadedChildrenFromFileSystem || !this->nativePath.length()) {
        return;
    }
    std::vector<Platform::ListNodeResult> results;
    Platform::listNodes(this->nativePath, results);
    std::shared_ptr<FsNode> self = shared_from_this();
    BHashTable<BString, bool> listedNames;
    for (auto& n : results) {
        BString localPath;
        BString remotePath;
        NativeEntryKind kind = getNativeEntryLocalPath(this->path, this->nativePath, n, localPath, remotePath);
        if (kind == NativeEntryKind::Skip) {
            continue;
        }
        BString childName = Fs::getFileNameFromPath(localPath);
        listedNames.set(childName, true);
        std::shared_ptr<FsNode> existing = this->getChildByName(childName);
        if (existing) {
            // Keep the existing node so open files keep their identity, unless
            // a file was replaced by a directory or the other way round.
            if (existing->isDirectory() == n.isDirectory || !existing->nativePath.length()) {
                continue;
            }
            this->removeChildByName(childName);
        }
        addChildFromNativeEntry(self, kind, n, localPath, remotePath);
    }
    std::vector<std::shared_ptr<FsNode> > children;
    this->getAllChildren(children);
    for (auto& child : children) {
        // Only native entries of this directory can disappear from its listing;
        // virtual nodes and mounts backed by other directories stay.
        if (!child->nativePath.length() || Fs::getNativeParentPath(child->nativePath) != this->nativePath) {
            continue;
        }
        if (!listedNames.contains(child->name)) {
            this->removeChildByName(child->name);
        }
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
