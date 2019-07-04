#include "boxedwine.h"

#include "kstat.h"

FsNode::FsNode(Type type, U32 id, U32 rdev, const std::string& path, const std::string& link, bool isDirectory, BoxedPtr<FsNode> parent) : 
    path(path),
    name(Fs::getFileNameFromPath(path)),
    link(link),
    id(id), 
    rdev(rdev),
    hardLinkCount(1),
    type(type),  
    kobject(0),  
    parent(parent),
    isDir(isDirectory),  
    hasLoadedChildrenFromFileSystem(false)
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
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(this->parent->childrenByNameMutex);
    this->parent->childrenByName.erase(this->name);
}

void FsNode::loadChildren() {
    // don't need to protect from threads since this is private
    if (!this->hasLoadedChildrenFromFileSystem) {
        this->hasLoadedChildrenFromFileSystem = true;
        std::string nativePath = Fs::localPathToRemote(this->path);
        std::vector<Platform::ListNodeResult> results;
        Platform::listNodes(nativePath, results);
        for (auto& n : results) {
            std::string localPath = this->path;
            if (!stringHasEnding(localPath, "/"))
                localPath+="/";
            localPath+=n.name;
            Fs::remotePathToLocal(localPath);
            if (!stringHasEnding(localPath, ".link")) {
                Fs::addFileNode(localPath, "", n.isDirectory, this);
            } else {
                U8 tmp[MAX_FILEPATH_LEN];
                U32 result = Fs::readFile(localPath, tmp, MAX_FILEPATH_LEN-1);
                tmp[result]=0;
                if (result==0) {
                    kwarn("Could not read link file from filesystem: %s", localPath.c_str());
                }
                localPath = localPath.substr(0, localPath.length()-5);
                Fs::addFileNode(localPath, (const char*)tmp, n.isDirectory, this);
            }           
        }
    }
}

BoxedPtr<FsNode> FsNode::getChildByName(const std::string& name) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(this->childrenByNameMutex);
    this->loadChildren();
    if (this->childrenByName.count(name))
        return this->childrenByName[name];
    return NULL;
}

U32 FsNode::getChildCount() {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(this->childrenByNameMutex);
    this->loadChildren();
    return (U32)this->childrenByName.size();
}

void FsNode::addChild(BoxedPtr<FsNode> node) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(this->childrenByNameMutex);
    this->loadChildren();
    this->childrenByName[node->name] = node;
}

void FsNode::removeChildByName(const std::string& name) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(this->childrenByNameMutex);
    this->loadChildren();
    this->childrenByName.erase(name);
}

void FsNode::getAllChildren(std::vector<BoxedPtr<FsNode> > & results) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(this->childrenByNameMutex);
    this->loadChildren();
    for (auto& n : this->childrenByName) {
        results.push_back(n.second);
    }
}

void FsNode::addLock(KFileLock* lock) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(this->locksMutex);
    this->locks.push_back(*lock);
}

KFileLock* FsNode::getLock(KFileLock* lock) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(this->locksMutex);
    U64 l1 = lock->l_start;
    U64 l2 = l1+lock->l_len;

    if (lock->l_len == 0)
        l2 = 0xFFFFFFFF;
    for( auto& n : this->locks ) {
        KFileLock* next = &n;
        U64 s1 = next->l_start;
        U64 s2 = s1+next->l_len;
        
        if (next->l_len == 0)
            s2 = 0xFFFFFFFF;
        if ((s1>=l1 && s1<=l2) || (s2>=l1 && s2<=l2)) {
            return next;
        }
    }
    return NULL;
 }

void FsNode::addOpenNode(KListNode<FsOpenNode*>* node) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(this->openNodesMutex);
    this->openNodes.addToBack(node);
}