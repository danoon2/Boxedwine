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
        klog_fmt("Created root directory: %s", rootPath.c_str());
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

static bool startsWithToken(const char* value, int len, int index, const char* token) {
    int tokenLen = (int)strlen(token);
    return index + tokenLen <= len && memcmp(value + index, token, tokenLen) == 0;
}

static int hexValue(char c) {
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    }
    if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    }
    return -1;
}

static void appendEscapedByte(BString& value, U8 c) {
    char buffer[8];
    snprintf(buffer, sizeof(buffer), "(_x%02X_)", c);
    value.append(buffer);
}

static bool isLocalPathSeparator(char c) {
    return c == '/';
}

static bool isAtEndOfLocalPathPart(const char* value, int len, int index) {
    return index + 1 >= len || isLocalPathSeparator(value[index + 1]);
}

void Fs::remoteNameToLocal(BString& path) {
    BString result;
    const char* value = path.c_str();
    int len = path.length();

    for (int i = 0; i < len;) {
        if (Fs::nativePathSeperator.length() && startsWithToken(value, len, i, Fs::nativePathSeperator.c_str())) {
            result.append('/');
            i += Fs::nativePathSeperator.length();
        } else if (startsWithToken(value, len, i, "(_colon_)")) {
            result.append(':');
            i += 9;
        } else if (startsWithToken(value, len, i, "(_question_)")) {
            result.append('?');
            i += 12;
        } else if (startsWithToken(value, len, i, "(_at_)")) {
            result.append('@');
            i += 6;
        } else if (startsWithToken(value, len, i, "(_dot_)")) {
            result.append('.');
            i += 7;
        } else if (startsWithToken(value, len, i, "(_space_)")) {
            result.append(' ');
            i += 9;
        } else if (i + 7 <= len && memcmp(value + i, "(_x", 3) == 0 && value[i + 5] == '_' && value[i + 6] == ')') {
            int hi = hexValue(value[i + 3]);
            int lo = hexValue(value[i + 4]);
            if (hi >= 0 && lo >= 0) {
                result.append((char)((hi << 4) | lo));
                i += 7;
            } else {
                result.append(value[i++]);
            }
        } else {
            result.append(value[i++]);
        }
    }
    path = result;
}

void Fs::localNameToRemote(BString& path) {
    BString result;
    const char* value = path.c_str();
    int len = path.length();

    for (int i = 0; i < len; i++) {
        U8 c = (U8)value[i];

        if (isLocalPathSeparator(value[i])) {
            result.append(Fs::nativePathSeperator);
        } else if (value[i] == ':') {
            result.append("(_colon_)");
        } else if (value[i] == '?') {
            result.append("(_question_)");
        } else if (value[i] == '@') {
            result.append("(_at_)");
        } else if (value[i] == '.' && isAtEndOfLocalPathPart(value, len, i)) {
            result.append("(_dot_)");
        } else if (value[i] == ' ' && isAtEndOfLocalPathPart(value, len, i)) {
            result.append("(_space_)");
        } else if (c >= 0x80) {
            appendEscapedByte(result, c);
        } else {
            result.append(value[i]);
        }
    }
    path = result;
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

std::shared_ptr<FsNode> Fs::getNodeFromLocalPath(const BString& currentDirectory, const BString& path,
    bool followLink, bool* isLink) {
    FsPathLookupOptions options;
    options.followFinalSymlink = followLink;
    options.allowEmptyPath = true;
    FsPathResult result = resolvePath(currentDirectory, path, options);
    if (isLink) {
        *isLink = result.finalComponentWasSymlink;
    }
    return result.node;
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

FsPathResult Fs::resolvePath(const BString& currentDirectory, const BString& path, const FsPathLookupOptions& options) {
    FsPathResult result;
    result.trailingSlash = path.length() > 0 && path.endsWith("/");

    if (!rootNode) {
        result.error = -K_ENOENT;
        return result;
    }
    if (!path.length() && !options.allowEmptyPath) {
        result.error = -K_ENOENT;
        return result;
    }

    std::deque<BString> pending;
    auto appendComponents = [&pending](const BString& value) {
        std::vector<BString> components;
        Fs::splitPath(value, components);
        pending.insert(pending.end(), components.begin(), components.end());
    };

    if (!path.startsWith("/")) {
        appendComponents(currentDirectory);
    }
    appendComponents(path);

    std::shared_ptr<FsNode> node = rootNode;
    std::vector<std::shared_ptr<FsNode>> ancestors;
    ancestors.push_back(rootNode);
    U32 symlinkCount = 0;
    bool followedFinalSymlink = false;

    while (!pending.empty()) {
        BString component = pending.front();
        pending.pop_front();

        if (!component.length()) {
            continue;
        }
        if (!node->isDirectory()) {
            result.error = -K_ENOTDIR;
            return result;
        }
        if (component == ".") {
            continue;
        }
        if (component == "..") {
            if (ancestors.size() > 1) {
                ancestors.pop_back();
            }
            node = ancestors.back();
            continue;
        }

        std::shared_ptr<FsNode> child = node->getChildByName(component);
        if (!child) {
            result.parent = node;
            result.finalName = component;
            result.missingComponents.push_back(component);
            for (const BString& missing : pending) {
                if (missing.length() && missing != ".") {
                    result.missingComponents.push_back(missing);
                }
            }
            result.error = -K_ENOENT;
            return result;
        }

        bool isFinalComponent = true;
        for (const BString& remaining : pending) {
            if (remaining.length()) {
                isFinalComponent = false;
                break;
            }
        }
        if (child->isLink() && (options.followFinalSymlink || !isFinalComponent || result.trailingSlash)) {
            if (isFinalComponent) {
                followedFinalSymlink = true;
            }
            if (++symlinkCount > 40) {
                result.error = -K_ELOOP;
                return result;
            }

            BString target = child->getLink();
            std::vector<BString> targetComponents;
            Fs::splitPath(target, targetComponents);
            if (target.endsWith("/")) {
                targetComponents.push_back(B("."));
            }
            pending.insert(pending.begin(), targetComponents.begin(), targetComponents.end());
            if (target.startsWith("/")) {
                node = rootNode;
                ancestors.clear();
                ancestors.push_back(rootNode);
            }
            continue;
        }

        result.parent = node;
        result.finalName = component;
        result.finalComponentWasSymlink = followedFinalSymlink || child->isLink();
        node = child;
        ancestors.push_back(node);
    }

    if ((result.trailingSlash || options.requireDirectory) && !node->isDirectory()) {
        result.error = -K_ENOTDIR;
        return result;
    }
    result.finalComponentWasSymlink = result.finalComponentWasSymlink || followedFinalSymlink;
    result.node = node;
    return result;
}

std::shared_ptr<FsFileNode> Fs::createFileNode(const BString& path, const BString& link, const BString& nativePath, bool isDirectory, const std::shared_ptr<FsNode>& parent) {
    return std::make_shared<FsFileNode>(Fs::nextNodeId++, 0, path, link, nativePath, isDirectory, false, parent);
}

std::shared_ptr<FsFileNode> Fs::addFileNode(const BString& path, const BString& link, const BString& nativePath, bool isDirectory, const std::shared_ptr<FsNode>& parent) {
    std::shared_ptr<FsFileNode> result = createFileNode(path, link, nativePath, isDirectory, parent);
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
    return result;
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
    FsPathLookupOptions options;
    options.followFinalSymlink = false;
    options.allowEmptyPath = true;
    FsPathResult resolution = Fs::resolvePath(B(""), path, options);
    std::shared_ptr<FsNode> node = resolution.node;
    std::vector<std::shared_ptr<FsNode> > nodes;
    bool notFound = false;

    if (!node) {
        if (resolution.error != -K_ENOENT || !resolution.parent) {
            return resolution.error;
        }
        node = resolution.parent;
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
        BString base = resolution.parent->path;
        for (U32 i=0;i<resolution.missingComponents.size();i++) {
            node = Fs::getNodeFromLocalPath(B(""), base, false);

            if (!base.endsWith("/")) // will be / if root
                base = base + "/";
            base = base + resolution.missingComponents[i];
            BString nativePath = resolution.missingComponents[i];
            Fs::localNameToRemote(nativePath);
            nativePath = node->nativePath.stringByApppendingPath(nativePath);
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
        BString tmp = path.substr(1);
        tmp.split('/', parts);
    } else {
        path.split('/', parts);
    }
}

U32 Fs::readNativeFile(const BString& nativePath, U8* buffer, U32 bufferLen) {
    int f = ::open(nativePath.c_str(), O_RDONLY | O_BINARY);
    if (f>=0) {
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
        return parent->nativePath.stringByApppendingPath(nativeFileName);
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
        tmp = tmp.stringByApppendingPath(part);
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
            U32 result = iterateAllNativeFiles(path.stringByApppendingPath(n.name), true, includeDirs, f);
            if (result) {
                return result;
            }
        }
        if (!includeDirs && n.isDirectory) {
            continue;
        }
        U32 result = f(path.stringByApppendingPath(n.name), n.isDirectory);
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

static BString getXAttrNativePath(const std::shared_ptr<FsNode>& file, const BString& name) {
    if (name == "user.DOSATTRIB") {
        return file->getNativePathForData() + EXT_DOSATTRIB;
    }
    if (name == "user.WINEREPARSE") {
        return file->getNativePathForData() + EXT_WINEREPARSE;
    }
    return BString::empty;
}

U32 Fs::getXAttr(const std::shared_ptr<FsNode>& file, const BString& name, std::vector<U8>& value) {
    BString nativePath = getXAttrNativePath(file, name);
    if (nativePath.isEmpty()) {
        return -K_ENOTSUP;
    }
    if (!Fs::doesNativePathExist(nativePath)) {
        return -K_ENODATA;
    }
    U64 len = Fs::getNativeFileSize(nativePath);
    value.resize((size_t)len);
    if (!len) {
        return 0;
    }
    BReadFile f(nativePath);
    if (!f.isOpen()) {
        return -K_EIO;
    }
    if (f.read(value.data(), len) != len) {
        return -K_EIO;
    }
    return 0;
}

U32 Fs::setXAttr(const std::shared_ptr<FsNode>& file, const BString& name, const U8* value, U32 len) {
    BString nativePath = getXAttrNativePath(file, name);
    if (nativePath.isEmpty()) {
        return -K_ENOTSUP;
    }
    BWriteFile w(nativePath, true);
    if (!w.isOpen()) {
        return -K_EIO;
    }
    if (len && w.write(value, len) != len) {
        return -K_EIO;
    }
    return 0;
}

U32 Fs::removeXAttr(const std::shared_ptr<FsNode>& file, const BString& name) {
    BString nativePath = getXAttrNativePath(file, name);
    if (nativePath.isEmpty()) {
        return -K_ENOTSUP;
    }
    if (Fs::doesNativePathExist(nativePath)) {
        deleteNativeFile(nativePath);
        return 0;
    }
    return -K_ENODATA;
}

U32 Fs::listXAttrNames(const std::shared_ptr<FsNode>& file, std::vector<BString>& names) {
    BString nativePath = file->getNativePathForData();
    if (Fs::doesNativePathExist(nativePath + EXT_DOSATTRIB)) {
        names.push_back(B("user.DOSATTRIB"));
    }
    if (Fs::doesNativePathExist(nativePath + EXT_WINEREPARSE)) {
        names.push_back(B("user.WINEREPARSE"));
    }
    return 0;
}

/*
Read - Only = 0x1
Hidden = 0x2
System = 0x4
Archive = 0x20
*/
BString Fs::getDosAttrib(const std::shared_ptr<FsNode>& file) {
    std::vector<U8> value;
    if (Fs::getXAttr(file, B("user.DOSATTRIB"), value) == 0 && value.size()) {
        return BString::copy((const char*)value.data(), (int)value.size());
    }
    return BString::empty;
}

void Fs::setDosAttrib(const std::shared_ptr<FsNode>& file, const BString& attrib) {
    Fs::setXAttr(file, B("user.DOSATTRIB"), (const U8*)attrib.c_str(), attrib.length());
}

U32 Fs::removeDosAttrib(const std::shared_ptr<FsNode>& file) {
    return Fs::removeXAttr(file, B("user.DOSATTRIB"));
}
