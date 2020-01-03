#include "boxedwine.h"

#include "fsfilenode.h"
#include "fsvirtualnode.h"
#ifdef BOXEDWINE_ZLIB
#include "fszip.h"
#endif

#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>

#include UNISTD
#include MKDIR_INCLUDE

U32 Fs::nextNodeId=1;
BOXEDWINE_MUTEX Fs::nextNodeIdMutex;

BoxedPtr<FsFileNode> Fs::rootNode;
std::string Fs::nativePathSeperator;

bool Fs::initFileSystem(const std::string& rootPath) {
    std::string path;
    if (stringHasEnding(rootPath, "/")) {
        Fs::nativePathSeperator = "/";
        path = rootPath.substr(0, rootPath.length()-1);
    } else if (stringHasEnding(rootPath, "\\")) {
        Fs::nativePathSeperator = "\\";
        path = rootPath.substr(0, rootPath.length()-1);
    } else {
        path = rootPath;
        if (stringContains(rootPath, "\\"))
            Fs::nativePathSeperator = "\\";
        else
            Fs::nativePathSeperator = "/";        
    }

    if (MKDIR(rootPath.c_str())==0) {
        klog("Created root directory: %s", rootPath.c_str());
    }

    BoxedPtr<FsNode> parent(NULL);
    rootNode = new FsFileNode(Fs::nextNodeId++, 0, "/", "", path, true, true, parent);

    BoxedPtr<FsNode> dir = Fs::getNodeFromLocalPath("", "/tmp/del", false, NULL);
    if (dir) {
        std::vector<BoxedPtr<FsNode> > children;
        dir->getAllChildren(children);
        for (U32 i=0;i<children.size();i++) {
            children[i]->remove();
        }
    }
    return true;
}

void Fs::remoteNameToLocal(std::string& path) {
    stringReplaceAll(path, Fs::nativePathSeperator, "/");
    stringReplaceAll(path, "(_colon_)", ":");
}

void Fs::localNameToRemote(std::string& path) {
    stringReplaceAll(path, "/", Fs::nativePathSeperator);
    stringReplaceAll(path, ":", "(_colon_)");
}

BoxedPtr<FsNode> Fs::addVirtualFile(const std::string& path, OpenVirtualNode func, U32 mode, U32 rdev, const BoxedPtr<FsNode>& parent) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(Fs::nextNodeIdMutex);
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

bool cleanPath(std::vector<std::string>& parts) {
    for (U32 i=0;i<parts.size();) {
        if (parts[i].length()==0) { // ignore double slashes
            parts.erase(parts.begin()+i, parts.begin()+i+1);
            continue;
        }
        if (parts[i]==".") {
            i++;
            continue;
        }
        if (parts[i]=="..") {
            if (i==0)
                return false;
            parts.erase(parts.begin()+i-1, parts.begin()+i+1);
            i--;
            continue;
        }
        i++;
    }
    return true;
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

    if (!cleanPath(parts))
        return NULL;
    for (U32 i=0;i<parts.size();) {
        if (parts[i].length()==0) { // ignore double slashes
            i++;
            continue;
        }
        if (parts[i]==".") {
            i++;
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
            }   
            if (!cleanPath(parts))
                return NULL;

            nodes.clear();
            node = Fs::rootNode;
            nodes.push_back(node);
            i=0;
            continue;
        }
        i++;
    }
    return node;
}

BoxedPtr<FsNode> Fs::addFileNode(const std::string& path, const std::string& link, const std::string& nativePath, bool isDirectory, const BoxedPtr<FsNode>& parent) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(Fs::nextNodeIdMutex);
    BoxedPtr<FsFileNode> result = new FsFileNode(Fs::nextNodeId++, 0, path, link, nativePath, isDirectory, false, parent);
    parent->addChild(result);
    return result;
}

BoxedPtr<FsNode> Fs::addRootDirectoryNode(const std::string& path, const std::string& nativePath, const BoxedPtr<FsNode>& parent) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(Fs::nextNodeIdMutex);
    BoxedPtr<FsFileNode> result = new FsFileNode(Fs::nextNodeId++, 0, path, "", nativePath, true, false, parent);
    parent->addChild(result);
    result->isRootPath = true;
    return result;
}

std::string Fs::getParentPath(const std::string& path) {
    return path.substr(0, path.rfind('/'));
}

std::string Fs::getNativeParentPath(const std::string& path) {
    return path.substr(0, path.rfind(Fs::nativePathSeperator));
}

std::string Fs::getFileNameFromPath(const std::string& path) {
    size_t pos = path.rfind('/');
    if (pos==std::string::npos) {
        return path;
    }
    return path.substr(pos+1);
}

std::string Fs::getFileNameFromNativePath(const std::string& path) {
    size_t pos = path.rfind(Fs::nativePathSeperator);
    if (pos==std::string::npos) {
        return path;
    }
    return path.substr(pos+1);
}

bool Fs::doesNativePathExist(const std::string& path) {
    if (::access(path.c_str(), 0)!=-1) {
        return true;
    }
    return false;
}

bool Fs::isNativePathDirectory(const std::string& path) {
    PLATFORM_STAT_STRUCT buf;

#ifdef BOXEDWINE_MSVC
    if (path.length()<3) {
        if (PLATFORM_STAT(std::string(path+"\\").c_str(), &buf)==0) {
            return S_ISDIR(buf.st_mode);
        }
    }
#endif
    if (PLATFORM_STAT(path.c_str(), &buf)==0) {
        return S_ISDIR(buf.st_mode);
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
        if (!Fs::doesNativePathExist(nodePart->nativePath)) {
            U32 result = MKDIR(nodePart->nativePath.c_str());
            if (result) {
                return -translateErr(errno);
            }
        }
    }
    if (notFound) {
        std::string base = lastNode->path;        
        for (U32 i=0;i<missingParts.size();i++) {
            node = Fs::getNodeFromLocalPath("", base, false);

            if (!stringHasEnding(base, "/")) // will be / if root
                base+=std::string("/");
            base+=missingParts[i];
            std::string nativePath = missingParts[i];
            Fs::localNameToRemote(nativePath);
            nativePath = node->nativePath + Fs::nativePathSeperator + nativePath;
            U32 result = MKDIR(nativePath.c_str());
            if (result) {
                return -translateErr(errno);
            }
            BoxedPtr<FsNode> childNode = Fs::addFileNode(base, "", nativePath, true, node);
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

U32 Fs::readNativeFile(const std::string& nativePath, U8* buffer, U32 bufferLen) {
    int f = ::open(nativePath.c_str(), O_RDONLY);
    if (f>0) {
        U32 result = ::read(f, buffer, bufferLen);
        ::close(f);
        return result;
    }
    return 0;
}

std::string Fs::getNativePathFromParentAndLocalFilename(const BoxedPtr<FsNode>& parent, const std::string fileName) {
    std::string nativeFileName = fileName;
    Fs::localNameToRemote(nativeFileName);
    return parent->nativePath + Fs::nativePathSeperator + nativeFileName;
}