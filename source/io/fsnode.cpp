#include "boxedwine.h"

#include "kstat.h"

FsNode::FsNode(Type type, U32 id, U32 rdev, const std::string& path, const std::string& link, bool isDirectory, BoxedPtr<FsNode> parent) : 
    type(type), 
    id(id), 
    rdev(rdev), 
    path(path), 
    kobject(0), 
    link(link), 
    isDir(isDirectory), 
    parent(parent), 
    name(Fs::getFileNameFromPath(path)), 
    hasLoadedChildrenFromFileSystem(false),
    hardLinkCount(1) {   
}

void FsNode::removeOpenNode(FsOpenNode* node) {
    node->listNode.remove();
}

bool FsNode::canRead() {
    return (getMode() & K__S_IREAD)!=0;
}

bool FsNode::canWrite() {
    return (getMode() & K__S_IWRITE)!=0;
}

void FsNode::removeNodeFromParent() {
    this->parent->childrenByName.erase(this->name);
}

void FsNode::loadChildren() {
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
    this->loadChildren();
    if (this->childrenByName.count(name))
        return this->childrenByName[name];
    return NULL;
}

U32 FsNode::getChildCount() {
    this->loadChildren();
    return this->childrenByName.size();
}

void FsNode::addChild(BoxedPtr<FsNode> node) {
    this->loadChildren();
    this->childrenByName[node->name] = node;
}

 void FsNode::removeChildByName(const std::string& name) {
    this->loadChildren();
    this->childrenByName.erase(name);
 }

 void FsNode::getAllChildren(std::vector<BoxedPtr<FsNode> > & results) {
    this->loadChildren();
    for (auto& n : this->childrenByName) {
        results.push_back(n.second);
    }
 }
