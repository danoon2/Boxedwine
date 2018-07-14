#include "boxedwine.h"

#include "fsfilenode.h"
#include "fsvirtualnode.h"
#include "fszip.h"

#include <stdio.h>
#include <fcntl.h>

#include UNISTD
#include MKDIR_INCLUDE

U32 Fs::nextNodeId=1;
BoxedPtr<FsNode> Fs::rootNode;
std::string Fs::nativePathSeperator;
std::string Fs::rootFileSystem;

bool Fs::initFileSystem(const std::string& rootPath, const std::string& zipPath) {
    Fs::rootFileSystem = rootPath;
    
    if (stringHasEnding(rootPath, "/")) {
        Fs::nativePathSeperator = "/";
        Fs::rootFileSystem = Fs::rootFileSystem.substr(0, Fs::rootFileSystem.length()-1);
    } else if (stringHasEnding(rootPath, "\\")) {
        Fs::nativePathSeperator = "\\";
        Fs::rootFileSystem = Fs::rootFileSystem.substr(0, Fs::rootFileSystem.length()-1);
    } else {
        if (stringContains(rootPath, "\\"))
            Fs::nativePathSeperator = "\\";
        else
            Fs::nativePathSeperator = "/";        
    }

    if (MKDIR(rootPath.c_str())==0) {
        klog("Created root directory: %s", rootPath.c_str());
    }

    BoxedPtr<FsNode> parent(NULL);
    rootNode = new FsFileNode(Fs::nextNodeId++, 0, "/", "", true, parent);
#ifdef BOXEDWINE_ZLIB
    return FsZip::init(zipPath);
#endif
    return true;
}

void Fs::remotePathToLocal(std::string& path) {
    if (stringStartsWith(path, Fs::rootFileSystem)) {
        path.erase(0, Fs::rootFileSystem.length());
    }
    stringReplaceAll(path, Fs::nativePathSeperator, "/");
    stringReplaceAll(path, "(_colon_)", ":");
}

std::string Fs::localPathToRemote(const std::string& path) {
    if (path == "/")
        return Fs::rootFileSystem;
    std::string result = path;
    stringReplaceAll(result, "/", Fs::nativePathSeperator);
    stringReplaceAll(result, ":", "(_colon_)");
    return Fs::rootFileSystem+result;
}

BoxedPtr<FsNode> Fs::addVirtualFile(const std::string& path, OpenVirtualNode func, U32 mode, U32 rdev, const BoxedPtr<FsNode>& parent) {
    BoxedPtr<FsNode> result = new FsVirtualNode(Fs::nextNodeId++, rdev, path, func, mode, parent);
    parent->addChild(result);
    return result;
}

BoxedPtr<FsNode> Fs::getNodeFromLocalPath(const std::string& currentDirectory, const std::string& path, bool followLink, bool* isLink) {
    BoxedPtr<FsNode> lastNode;
    std::vector<std::string> missingParts;
    return Fs::getNodeFromLocalPath(currentDirectory, path, lastNode, missingParts, followLink, isLink);
}

std::string Fs::getFullPath(const std::string& currentDirectory, const std::string& path) {
    std::string fullpath;

    if (stringStartsWith(path, "/"))
        fullpath = path;
    else
        fullpath = currentDirectory+"/"+path;
    if (stringHasEnding(fullpath, "/."))
        fullpath = fullpath.substr(0, fullpath.length()-2);
    return fullpath;
}

BoxedPtr<FsNode> Fs::getNodeFromLocalPath(const std::string& currentDirectory, const std::string& path, BoxedPtr<FsNode>& lastNode, std::vector<std::string>& missingParts, bool followLink, bool* isLink) {
    std::string fullpath = Fs::getFullPath(currentDirectory, path);

    if (fullpath.length()==0 || fullpath=="/")
        return Fs::rootNode;
    std::vector<std::string> parts;
    Fs::splitPath(fullpath, parts);
    BoxedPtr<FsNode> node = Fs::rootNode;
    std::vector<BoxedPtr<FsNode> > nodes;

    nodes.push_back(node);
    for (U32 i=0;i<parts.size();) {
        if (parts[i].length()==0) { // ignore double slashes
            i++;
            continue;
        }
        if (parts[i]==".") {
            i++;
            continue;
        }
        if (parts[i]=="..") {
            if (i==0)
                return NULL;
            parts.erase(parts.begin()+i-1, parts.begin()+i+1);
            nodes.pop_back();
            node = nodes.back();
            i--;
            continue;
        }
        node = node->getChildByName(parts[i]);
        if (!node) {
            for (;i<parts.size();i++) {
                missingParts.push_back(parts[i]);
            }
            lastNode = nodes.back();
            return NULL;
        }
        nodes.push_back(node);
        if (node->isLink() && (followLink || i<parts.size()-1)) {
            if (i==parts.size()-1 && isLink) {
                *isLink = true;
            }

            std::vector<std::string> linkParts;
            Fs::splitPath(node->link, linkParts);
            parts.erase(parts.begin()+i, parts.begin()+i+1);
            parts.insert(parts.begin()+i, linkParts.begin(), linkParts.end());
            if (stringStartsWith(node->link, "/")) {
                parts.erase(parts.begin(), parts.begin()+i);
                i=0;     
                node = Fs::rootNode;
                nodes.clear();
                nodes.push_back(node);
            } else {
                nodes.pop_back();
                node = nodes.back(); // do it again, it might be another link
            }            
            continue;
        }
        i++;
    }
    return node;
}

BoxedPtr<FsNode> Fs::addFileNode(const std::string& path, const std::string& link, bool isDirectory, const BoxedPtr<FsNode>& parent) {
    BoxedPtr<FsFileNode> result = new FsFileNode(Fs::nextNodeId++, 0, path, link, isDirectory, parent);
    parent->addChild(result);
    return result;
}

std::string Fs::getParentPath(const std::string& path) {
    return path.substr(0, path.rfind('/'));
}

std::string Fs::getNativeParentPath(const std::string& path) {
    return path.substr(0, path.rfind(Fs::nativePathSeperator));
}

std::string Fs::getFileNameFromPath(const std::string& path) {
    return path.substr(path.rfind('/')+1);
}

bool Fs::doesNativePathExist(const std::string& path) {
    if (::access(path.c_str(), 0)!=-1) {
        return true;
    }
    return false;
}

U32 Fs::makeLocalDirs(const std::string& path) {
    BoxedPtr<FsNode> lastNode;
    std::vector<std::string> missingParts;
    BoxedPtr<FsNode> node = Fs::getNodeFromLocalPath("", path, lastNode, missingParts, false);    
    std::vector<BoxedPtr<FsNode> > nodes;
    bool notFound = false;

    if (!node) {
        node = lastNode;
        notFound = true;
    }
    while (node) {
        nodes.push_back(node);
        node = node->getParent();
    }
    std::reverse(nodes.begin(), nodes.end());
    for (U32 i=0;i<nodes.size();i++) {
        BoxedPtr<FsNode> nodePart = nodes[i];
        std::string nativePath = Fs::localPathToRemote(nodePart->path);
        if (!Fs::doesNativePathExist(nativePath)) {
            U32 result = MKDIR(nativePath.c_str());
            if (result)
                return result;
        }
    }
    if (notFound) {
        std::string base = lastNode->path;        
        for (U32 i=0;i<missingParts.size();i++) {
            node = Fs::getNodeFromLocalPath("", base, false);

            if (!stringHasEnding(base, "/")) // will be / if root
                base+=std::string("/");
            base+=missingParts[i];
            std::string nativePath = Fs::localPathToRemote(base);
            U32 result = MKDIR(nativePath.c_str());
            if (result)
                return result;
            BoxedPtr<FsNode> childNode = Fs::addFileNode(base, "", true, node);
        }
    }
    return 0;
}

void Fs::splitPath(const std::string& path, std::vector<std::string>& parts) {
    if (stringStartsWith(path, "/"))
        stringSplit(parts,path.substr(1),'/');
    else
        stringSplit(parts,path,'/');
}

U32 Fs::readFile(const std::string& path, U8* buffer, U32 bufferLen) {
    std::string nativePath = Fs::localPathToRemote(path);
    int f = ::open(nativePath.c_str(), O_RDONLY);
    if (f>0) {
        U32 result = ::read(f, buffer, bufferLen);
        ::close(f);
        return result;
    }
    return 0;
}
