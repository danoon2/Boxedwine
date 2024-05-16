#include "boxedwine.h"

#include "fsfilenode.h"
#include "fsvirtualnode.h"
#include "fsdynamiclinknode.h"
#include "bufferaccess.h"

#ifdef BOXEDWINE_ZLIB
#include "fszip.h"
#endif

#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>

#include UNISTD
#include MKDIR_INCLUDE

std::atomic_int Fs::nextNodeId=1;

std::shared_ptr<FsFileNode> Fs::rootNode;
BString Fs::nativePathSeperator;

void Fs::shutDown() {
	rootNode = nullptr;
}
bool Fs::initFileSystem(const BString& rootPath) {
    Fs::nextNodeId = 1;
    BString path;
    Fs::nativePathSeperator = (char)std::filesystem::path::preferred_separator;
    if (rootPath.endsWith("/")) {
        path = rootPath.substr(0, rootPath.length()-1);
    } else if (rootPath.endsWith("\\")) {
        path = rootPath.substr(0, rootPath.length()-1);
    } else {
        path = rootPath;
    }

    if (MKDIR(rootPath.c_str())==0) {
        klog("Created root directory: %s", rootPath.c_str());
    }

    std::shared_ptr<FsNode> parent(nullptr);
    rootNode = std::make_shared<FsFileNode>(Fs::nextNodeId++, 0, B("/"), B(""), path, true, true, parent);

    std::shared_ptr<FsNode> dir = Fs::getNodeFromLocalPath(B(""), B("/tmp/del"), false, nullptr);
    if (dir) {
        std::vector<std::shared_ptr<FsNode> > children;
        dir->getAllChildren(children);
        for (U32 i=0;i<children.size();i++) {
            children[i]->remove();
        }
    }
    std::shared_ptr<FsNode> lock = Fs::getNodeFromLocalPath(B(""), B("/tmp/.X0-lock"), false, nullptr);
    if (lock) {
        lock->remove();
    }
    return true;
}

void Fs::remoteNameToLocal(BString& path) {
    path.replace(Fs::nativePathSeperator, "/");
    path.replace("(_colon_)", ":");
    path.replace("(_at_)", "@");
}

void Fs::localNameToRemote(BString& path) {
    path.replace("/", Fs::nativePathSeperator);
    path.replace(":", "(_colon_)");
    path.replace("@", "(_at_)");
}

BString Fs::localFromNative(const BString& path) {
    BString result = path;
    remoteNameToLocal(result);
    return result;
}

BString Fs::nativeFromLocal(const BString& path) {
    BString result = path;
    localNameToRemote(result);
    return result;
}

std::shared_ptr<FsNode> Fs::addVirtualFile(const BString& path, std::function<FsOpenNode* (const std::shared_ptr<FsNode>& node, U32 flags, U32 data)> func, U32 mode, U32 rdev, const std::shared_ptr<FsNode>& parent, U32 data) {
    std::shared_ptr<FsNode> result = std::make_shared<FsVirtualNode>(Fs::nextNodeId++, rdev, path, func, mode, parent, data);
    parent->addChild(result);
    return result;
}

std::shared_ptr<FsNode> Fs::addVirtualFile(const BString& path, U32 mode, U32 rdev, const std::shared_ptr<FsNode>& parent, const BString& value) {
    std::shared_ptr<FsNode> result = std::make_shared<FsVirtualNode>(Fs::nextNodeId++, rdev, path, [value](const std::shared_ptr<FsNode>& node, U32 flags, U32 data) {
        return new BufferAccess(node, flags, value);
        }, mode, parent, 0);
    parent->addChild(result);
    return result;
}

std::shared_ptr<FsNode> Fs::addDynamicLinkFile(const BString& path, U32 rdev, const std::shared_ptr<FsNode>& parent, bool isDirectory, const BString& link) {
    std::shared_ptr<FsNode> result = std::make_shared<FsDynamicLinkNode>(Fs::nextNodeId++, rdev, path, parent, isDirectory, [link]() {
        return link;
        });
    parent->addChild(result);
    return result;
}

std::shared_ptr<FsNode> Fs::addDynamicLinkFile(const BString& path, U32 rdev, const std::shared_ptr<FsNode>& parent, bool isDirectory, std::function<BString(void)> fnGetLink) {
    std::shared_ptr<FsNode> result = std::make_shared<FsDynamicLinkNode>(Fs::nextNodeId++, rdev, path, parent, isDirectory, fnGetLink);
    parent->addChild(result);
    return result;
}

std::shared_ptr<FsNode> Fs::getNodeFromLocalPath(const BString& currentDirectory, const BString& path, bool followLink, bool* isLink) {
    return Fs::getNodeFromLocalPath(currentDirectory, path, nullptr, nullptr, followLink, isLink);
}

BString Fs::getFullPath(const BString& currentDirectory, const BString& path) {
    BString fullpath;

    if (path.startsWith("/"))
        fullpath = path;
    else
        fullpath = currentDirectory+"/"+path;
    if (fullpath.endsWith("/."))
        fullpath = fullpath.substr(0, fullpath.length()-2);
    fullpath.replace("//", "/");
    return fullpath;
}

bool cleanPath(std::vector<BString>& parts) {
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

std::shared_ptr<FsNode> Fs::getNodeFromLocalPath(const BString& currentDirectory, const BString& path, std::shared_ptr<FsNode>* lastNode, std::vector<BString>* missingParts, bool followLink, bool* isLink) {
    BString fullpath = Fs::getFullPath(currentDirectory, path);

    if (fullpath.length()==0 || fullpath=="/")
        return Fs::rootNode;
    std::vector<BString> parts;
    Fs::splitPath(fullpath, parts);
    std::shared_ptr<FsNode> node = Fs::rootNode;
    if (!node) {
        return nullptr;
    }
    std::vector<std::shared_ptr<FsNode> > nodes;

    nodes.push_back(node);

    if (!cleanPath(parts))
        return nullptr;
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
            if (missingParts) {
                for (; i < parts.size(); i++) {
                    missingParts->push_back(parts[i]);
                }
            }
            if (lastNode) {
                *lastNode = nodes.back();
            }
            return nullptr;
        }
        nodes.push_back(node);
        if (node->isLink() && (followLink || i<parts.size()-1)) {
            if (i==parts.size()-1 && isLink) {
                *isLink = true;
            }

            std::vector<BString> linkParts;
            Fs::splitPath(node->getLink(), linkParts);
            parts.erase(parts.begin()+i, parts.begin()+i+1);
            parts.insert(parts.begin()+i, linkParts.begin(), linkParts.end());
            if (node->getLink().startsWith("/")) {
                parts.erase(parts.begin(), parts.begin()+i);
            }   
            if (!cleanPath(parts))
                return nullptr;

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

std::shared_ptr<FsFileNode> Fs::addFileNode(const BString& path, const BString& link, const BString& nativePath, bool isDirectory, const std::shared_ptr<FsNode>& parent) {
    std::shared_ptr<FsFileNode> result = std::make_shared<FsFileNode>(Fs::nextNodeId++, 0, path, link, nativePath, isDirectory, false, parent);
    parent->addChild(result);
    return result;
}

std::shared_ptr<FsNode> Fs::addRootDirectoryNode(const BString& path, const BString& nativePath, const std::shared_ptr<FsNode>& parent) {
    std::shared_ptr<FsFileNode> result = std::make_shared<FsFileNode>(Fs::nextNodeId++, 0, path, B(""), nativePath, true, false, parent);
    parent->addChild(result);
    result->isRootPath = true;
    return result;
}

BString Fs::getParentPath(const BString& path) {
    return path.substr(0, path.lastIndexOf('/', path.length()-2));
}

BString Fs::getNativeParentPath(const BString& path) {
    int pos = path.lastIndexOf(Fs::nativePathSeperator);
    if (pos != -1) {
        return path.substr(0, pos);
    }
    pos = path.lastIndexOf('/');
    if (pos != -1) {
        return path.substr(0, pos);
    }
    return path;
}

BString Fs::getFileNameFromPath(const BString& path) {
    int pos = path.lastIndexOf('/');
    if (pos==-1) {
        return path;
    }
    return path.substr(pos+1);
}

BString Fs::getFileNameFromNativePath(const BString& path) {
    int pos = path.lastIndexOf(Fs::nativePathSeperator);
    if (pos==-1) {
        return path;
    }
    return path.substr(pos+1);
}

bool Fs::doesNativePathExist(const BString& path) {
    if (::access(path.c_str(), 0)!=-1) {
        return true;
    }
    return false;
}

bool Fs::isNativeDirectoryEmpty(const BString& path) {
    bool result = true;
    iterateAllNativeFiles(path, false, true, [&result](BString fileName, bool isDir)->U32 {
        result = false;
        return 1; // don't need to continue;
    });
    return false;
}

U64 Fs::getNativeFileSize(const BString& path) {
    PLATFORM_STAT_STRUCT buf;
    if (PLATFORM_STAT(path.c_str(), &buf)==0) {
        return buf.st_size;
    }
    return 0;
}

U64 Fs::getNativeDirectorySize(const BString& path, bool recursive) {
    U64 result = 0;
    iterateAllNativeFiles(path, recursive, true, [&result, path](BString fileName, bool isDir)->U32 {
        if (!isDir) {
            result += Fs::getNativeFileSize(fileName);
        }
        return 0;
    });
    return result;
}

bool Fs::isNativePathDirectory(const BString& path) {
    PLATFORM_STAT_STRUCT buf = {};

#ifdef BOXEDWINE_MSVC
    if (path.length()<3) {
        BString msvc = path + "\\";
        if (PLATFORM_STAT(msvc.c_str(), &buf)==0) {
            return S_ISDIR(buf.st_mode);
        }
    }
#endif
    if (PLATFORM_STAT(path.c_str(), &buf)==0) {
        return S_ISDIR(buf.st_mode);
    }
    return false;
}

U32 Fs::makeLocalDirs(const BString& path) {
    std::shared_ptr<FsNode> lastNode;
    std::vector<BString> missingParts;
    std::shared_ptr<FsNode> node = Fs::getNodeFromLocalPath(B(""), path, &lastNode, &missingParts, false);    
    std::vector<std::shared_ptr<FsNode> > nodes;
    bool notFound = false;

    if (!node) {
        node = lastNode;
        notFound = true;
    }
    while (node) {
        nodes.push_back(node);
        node = node->getParent().lock();
    }
    std::reverse(nodes.begin(), nodes.end());
    for (U32 i=0;i<nodes.size();i++) {
        std::shared_ptr<FsNode> nodePart = nodes[i];
        if (!Fs::doesNativePathExist(nodePart->nativePath)) {
            U32 result = MKDIR(nodePart->nativePath.c_str());
            if (result) {
                return -translateErr(errno);
            }
        }
    }
    if (notFound) {
        BString base = lastNode->path;        
        for (U32 i=0;i<missingParts.size();i++) {
            node = Fs::getNodeFromLocalPath(B(""), base, false);

            if (!base.endsWith("/")) // will be / if root
                base = base + "/";
            base = base + missingParts[i];
            BString nativePath = missingParts[i];
            Fs::localNameToRemote(nativePath);
            nativePath = node->nativePath ^ nativePath;
            U32 result = MKDIR(nativePath.c_str());
            if (result) {
                return -translateErr(errno);
            }
            std::shared_ptr<FsNode> childNode = Fs::addFileNode(base, B(""), nativePath, true, node);
        }
    }
    return 0;
}

void Fs::splitPath(const BString& path, std::vector<BString>& parts) {
    if (path.startsWith("/")) {
        path.substr(1).split('/', parts);
    } else {
        path.split('/', parts);
    }
}

U32 Fs::readNativeFile(const BString& nativePath, U8* buffer, U32 bufferLen) {
    int f = ::open(nativePath.c_str(), O_RDONLY | O_BINARY);
    if (f>0) {
        U32 result = (U32)::read(f, buffer, bufferLen);
        ::close(f);
        return result;
    }
    return 0;
}

BString Fs::getNativePathFromParentAndLocalFilename(const std::shared_ptr<FsNode>& parent, const BString& fileName) {
    BString nativeFileName = fileName;
    Fs::localNameToRemote(nativeFileName);
    if (parent) {
        return parent->nativePath ^ nativeFileName;
    }
    return nativeFileName;
}

bool Fs::makeNativeDirs(const BString& path) {
    std::vector<BString> parts;

    BString tmp;
    if (path.startsWith(Fs::nativePathSeperator)) {
        path.substr(1).split(Fs::nativePathSeperator, parts);
        tmp = Fs::nativePathSeperator + parts[0];
    } else {
        path.split(Fs::nativePathSeperator, parts);
        tmp = parts[0];
    }
    parts.erase(parts.begin(), parts.begin()+1);

    for (auto& part : parts) {
        tmp = tmp ^ part;
        if (!Fs::doesNativePathExist(tmp)) {
            U32 result = MKDIR(tmp.c_str());
            if (result!=0) {
                return false;
            }
        }
    }
    return true;
}

U32 Fs::deleteNativeFile(const BString& path) {
    return unlink(path.c_str());
}

U32 Fs::deleteNativeDirAndAllFilesInDir(const BString& path) {
    std::error_code e; // will prevent it from throwing an error
    return (U32)std::filesystem::remove_all(path.c_str(), e);
}

U32 Fs::iterateAllNativeFiles(const BString& path, bool recursive, bool includeDirs, std::function<U32(BString, bool isDir)> f) {
    std::vector<Platform::ListNodeResult> results;
    Platform::listNodes(path, results);
    for (auto& n : results) {
        if (recursive && n.isDirectory) {
            U32 result = iterateAllNativeFiles(path ^ n.name, true, includeDirs, f);
            if (result) {
                return result;
            }
        }
        if (!includeDirs && n.isDirectory) {
            continue;
        }
        U32 result = f(path ^ n.name, n.isDirectory);
        if (result) {
            return result;
        }
    }
    return 0;
}

std::vector<BString> Fs::getFilesInNativeDirectoryWhereFileMatches(const BString& dirPath, const BString& startsWith, const BString& endsWith, bool ignoreCase) {
    std::vector<BString> results;
    Fs::iterateAllNativeFiles(dirPath, false, false, [&results, startsWith, endsWith, ignoreCase](BString filePath, bool isDir) {
        BString name = Fs::getFileNameFromNativePath(filePath);
        if ((startsWith.isEmpty() || name.startsWith(startsWith, ignoreCase)) && (endsWith.isEmpty() || name.endsWith(endsWith, ignoreCase))) {
            results.push_back(filePath);
        }
        return 0;
    });
    return results;
}

BString Fs::trimTrailingSlash(const BString& s) {
    if (s.endsWith('/') || s.endsWith('\\')) {
        return s.substr(0, s.length() - 1);
    }
    return s;
}

/*
Read - Only = 0x1
Hidden = 0x2
System = 0x4
Archive = 0x20
*/
BString Fs::getDosAttrib(const std::shared_ptr<FsNode>& file) {
    U8 buffer[1024] = { 0 };
    if (Fs::readNativeFile(file->nativePath + EXT_DOSATTRIB, buffer, 1024)) {
        return BString::copy((const char*)buffer);
    }
    return BString::empty;
}

void Fs::setDosAttrib(const std::shared_ptr<FsNode>& file, const BString& attrib) {
    BWriteFile w(file->nativePath + EXT_DOSATTRIB, true);
    if (w.isOpen()) {
        w.write(attrib);
    }
}

U32 Fs::removeDosAttrib(const std::shared_ptr<FsNode>& file) {
    BString nativePath = file->nativePath + EXT_DOSATTRIB;
    if (Fs::doesNativePathExist(nativePath)) {
        deleteNativeFile(nativePath);
        return 0;
    } else {
        return -K_ENODATA; // ENOATTR
    }
}