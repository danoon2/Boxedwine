#include "boxedwine.h"

#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>

#include UNISTD
#include UTIME
#include MKDIR_INCLUDE

#include "fsfilenode.h"
#include "fsdiropennode.h"
#include "fsfileopennode.h"
#include "kstat.h"
#include "fszipnode.h"
#include "knativethread.h"

std::set<BString> FsFileNode::nonExecFileFullPaths;

FsFileNode::FsFileNode(U32 id, U32 rdev, BString path, BString link, BString nativePath, bool isDirectory, bool isRootPath, std::shared_ptr<FsNode> parent) : FsNode(Type::File, id, rdev, path, link, nativePath, isDirectory, parent), isRootPath(isRootPath) {
}

BString FsFileNode::getNativeTmpPath() {
    BString localPath;
    BString nativePath;
    getTmpPath(nativePath, localPath);
    return nativePath;
}

BString FsFileNode::getLocalTmpPath() {
    BString localPath;
    BString nativePath;
    getTmpPath(nativePath, localPath);
    return localPath;
}

static U32 nextTmpId;

void FsFileNode::getTmpPath(BString& nativePath, BString& localPath) {
    BOXEDWINE_CRITICAL_SECTION;
    Fs::makeLocalDirs(B("/tmp/del"));
    std::shared_ptr<FsNode> delDir = Fs::getNodeFromLocalPath(B(""), B("/tmp/del"), true);

    for (;nextTmpId<100000000;nextTmpId++) {
        BString name = "del"+BString::valueOf(nextTmpId)+".tmp";
        Fs::localNameToRemote(name);
        nativePath = delDir->nativePath ^ name;
        if (!Fs::doesNativePathExist(nativePath)) {
            localPath = "/tmp/del/"+name;
            break;
        }
    }
}

bool FsFileNode::remove() {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(this->openNodesMutex);
    bool result = false;
    bool exists = Fs::doesNativePathExist(this->nativePath);

#ifdef BOXEDWINE_ZLIB
    if (zipNode) {
        std::shared_ptr<FsZip> fsZip = zipNode->fsZip.lock();
        if (fsZip) {
            fsZip->remove(this->path);
        }
        zipNode = nullptr;
        result = true;
    }
#endif
    if (exists) {
        BString dosAttrib = nativePath + EXT_DOSATTRIB;
        unlink(dosAttrib.c_str());
        result = unlink(nativePath.c_str()) == 0;        
    }
    // if the file failed to be deleted and it exists then its because someone else has it open, 
    // so we need to close all references, move the file then re-open the file for those handles
    // that still have it open.  By moving the file it will appear that it was deleted, but handles
    // that have it open can still use it, just like Linux does.
    if (!result && exists && this->openNodes.size()) {
        S64* tmpPos = new S64[this->openNodes.size()];
        U32 i=0;

        this->openNodes.for_each([&tmpPos,&i](KListNode<FsOpenNode*>* n) {
            FsOpenNode* openNode = n->data;
            if (openNode->isOpen()) {
                tmpPos[i++] = openNode->getFilePointer();
                openNode->close();
            }
        });
        
        Fs::makeLocalDirs(B("/tmp/del"));
        std::shared_ptr<FsNode> delDir = Fs::getNodeFromLocalPath(B(""), B("/tmp/del"), true);

        BString newNativePath = FsFileNode::getNativeTmpPath();

        if (::rename(nativePath.c_str(), newNativePath.c_str())!=0) {
            klog("could not rename %s", nativePath.c_str());
        }

        this->nativePath = newNativePath;

        i=0;
        this->openNodes.for_each([&tmpPos,&i](KListNode<FsOpenNode*>* n) {
            FsOpenNode* openNode = n->data;
            openNode->reopen();
            openNode->seek(tmpPos[i++]);
        });
        result = true;
    }
    if (result) {
        this->removeNodeFromParent();
    }
    return result;
}

U64 FsFileNode::lastModified() {
    PLATFORM_STAT_STRUCT buf;

    if (PLATFORM_STAT(this->nativePath.c_str(), &buf)==0) {
        if (buf.st_mtime == 0) {
            // I've seen this happen with the Age of Empires installer on Raspberry Pi 5
            return ((U64)buf.st_ctime) * 1000l;
        }
        return ((U64)buf.st_mtime)*1000l;
    }
#ifdef BOXEDWINE_ZLIB
    if (this->zipNode)
        return this->zipNode->lastModified();
#endif
    return 0;
}

U64 FsFileNode::length() {    
    if (this->isDirectory())
        return 4096;

    PLATFORM_STAT_STRUCT buf;
    if (PLATFORM_STAT(this->nativePath.c_str(), &buf)==0) {
        return buf.st_size;
    }
#ifdef BOXEDWINE_ZLIB
    if (this->zipNode)
        return this->zipNode->length();
#endif
    return 0;
}

BString FsFileNode::getLink() {
#if defined(BOXEDWINE_ZLIB) && !defined(__EMSCRIPTEN__)
    ensurePathIsLocal(false);
#endif
    return this->link; 
}

void FsFileNode::ensurePathIsLocal(bool prepareForWrite) {
#ifdef BOXEDWINE_ZLIB
    BOXEDWINE_CRITICAL_SECTION;
    if (this->zipNode && !Fs::doesNativePathExist(this->nativePath)) {
        if (this->isDirectory()) {
            Fs::makeLocalDirs(this->path);
        } else {
            BString parentPath = Fs::getParentPath(this->path);
            Fs::makeLocalDirs(parentPath);
            this->zipNode->moveToFileSystem(shared_from_this());
        }
    } else if (prepareForWrite) {
        std::shared_ptr<FsNode> parent = this->getParent().lock();
        if (parent && parent->type == Type::File) {
            BString parentPath = Fs::getParentPath(this->path);
            Fs::makeLocalDirs(parentPath);
        }
    }
#endif
}

FsOpenNode* FsFileNode::open(U32 flags) {
    U32 openFlags = O_BINARY;
            
    if (this->isDirectory()) {
        std::shared_ptr<FsNode> n = Fs::getNodeFromLocalPath(B(""), this->path, true);
        if (!n) {
            return nullptr;
        }
        return new FsDirOpenNode(n, flags);
    }
    if ((flags & K_O_ACCMODE)==K_O_RDONLY) {
        openFlags|=O_RDONLY;
        // make the file local so that it will load faster (no inflate and weird seeking logic)
#if defined(BOXEDWINE_ZLIB) && !defined(__EMSCRIPTEN__)
        if (this->zipNode) {
            ensurePathIsLocal(false);
        }
#endif
    } else {
        if ((flags & K_O_ACCMODE)==K_O_WRONLY) {
            openFlags|=O_WRONLY;        
        } else {
            openFlags|=O_RDWR;            
        }
        BString parentPath = Fs::getNativeParentPath(this->nativePath);
        if (!Fs::doesNativePathExist(parentPath)) {
            ensurePathIsLocal(true);
        }
#ifdef BOXEDWINE_ZLIB
        else if (this->zipNode) {
            ensurePathIsLocal(false);
        }
#endif
    }
    if (flags & K_O_CREAT) {
        openFlags|=O_CREAT;
    }
    if (flags & K_O_EXCL) {
        openFlags|=O_EXCL;
    }
    if (flags & K_O_TRUNC) {
        openFlags|=O_TRUNC;
    }
    if (flags & K_O_APPEND) {
        openFlags|=O_APPEND;
    }
    U32 f = ::open(this->nativePath.c_str(), openFlags, 0666);	
    if (!f || f==0xFFFFFFFF) {
#ifdef BOXEDWINE_ZLIB
        if (this->zipNode && (flags & K_O_ACCMODE)==K_O_RDONLY)
            return this->zipNode->open(shared_from_this(), flags);
#endif
        return nullptr;
    }
    return new FsFileOpenNode(std::dynamic_pointer_cast<FsFileNode>(shared_from_this()), flags, f);
}

U32 FsFileNode::getType(bool checkForLink) {	
    if (this->isDirectory())
        return 4; // DT_DIR
    if (checkForLink && this->isLink()) 
        return 10; // DT_LNK
    return 8; // DT_REG
}

U32 FsFileNode::getMode() {
    U32 result = K__S_IREAD | (FsFileNode::getType(false) << 12);
    if (this->path == "/etc/sudoers") {
        return result | K__S_IRGRP;
    }
    if (!FsFileNode::nonExecFileFullPaths.count(this->path)) {
        result |= K__S_IEXEC;
        if (this->path.startsWith("/usr/local/bin") || this->path.startsWith("/usr/bin") || this->path.startsWith("/bin") || this->path.startsWith("/opt/wine/bin")) {
            result |= K__S_IXGRP | K__S_IXOTH;
        }
    }
    if (KThread::currentThread()->process->userId == 0 || this->path.startsWith("/tmp") || this->path.startsWith("/var") || this->path.startsWith("/home")) {
        result |= K__S_IWRITE;
        // wine server needs to be private, but winetricks check "-w" in the script on /tmp which needs these 2
        if (!this->path.startsWith("/tmp/.wine")) {
            result |= K__S_IWGRP | K__S_IWOTH;
        }
    }
    return result;
}

S32 translateErr(U32 e) {
    switch (e) {
    case EPERM: return K_EPERM;
    case ENOENT: return K_ENOENT;
    case ESRCH: return K_ESRCH;
    case EINTR: return K_EINTR;
    case EIO: return K_EIO;
    case ENXIO: return K_ENXIO;
    //case E2BIG: return K_E2BIG;
    case ENOEXEC: return K_ENOEXEC;
    case EBADF: return K_EBADF;
    case ECHILD: return K_ECHILD;
    case EAGAIN: return K_EAGAIN;
    case ENOMEM: return K_ENOMEM;
    case EACCES: return K_EACCES;
    case EFAULT: return K_EFAULT;
    case EBUSY: return K_EBUSY;
    case EEXIST: return K_EEXIST;
    case EXDEV: return K_EXDEV;
    case ENODEV: return K_ENODEV;
    case ENOTDIR: return K_ENOTDIR;
    case EISDIR: return K_EISDIR;
    case ENFILE: return K_ENFILE;
    case EMFILE: return K_EMFILE;
    case ENOTTY: return K_ENOTTY;
    case EFBIG: return K_EFBIG;
    case ENOSPC: return K_ENOSPC;
    case ESPIPE: return K_ESPIPE;
    case EROFS: return K_EROFS;
    //case EMLINK: return K_EMLINK;
    case EPIPE: return K_EPIPE;
    //case EDOM: return K_EDOM;
    //case EDEADLK: return K_EDEADLK;
    case ENAMETOOLONG: return K_ENAMETOOLONG;
    case ENOLCK: return K_ENOLCK;
    case ENOSYS: return K_ENOSYS;
    case ENOTEMPTY: return K_ENOTEMPTY;
    default: return K_EIO;
    }
}

U32 FsFileNode::rename(BString path) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(this->openNodesMutex);
    U32 result=0;
    std::vector<S64> tmpPos;

    this->ensurePathIsLocal(false);
    if (this->openNodes.size()) {
        int i=0;
        for (U32 x=0;x<this->openNodes.size();x++) {
            tmpPos.push_back(-1);
        }
        this->openNodes.for_each([&tmpPos,&i](KListNode<FsOpenNode*>* n) {
            FsOpenNode* openNode = n->data;
            if (openNode->isOpen()) {
                tmpPos[i] = openNode->getFilePointer();
                openNode->close();
            }
            i++;
        });
    }

    std::shared_ptr<FsNode> parent = Fs::getNodeFromLocalPath(B(""), Fs::getParentPath(path), true);
    if (!parent) {
        return -K_ENOENT;
    }

    BString fileName = Fs::getFileNameFromPath(path);
    Fs::localNameToRemote(fileName);
    BString nativePath = parent->nativePath ^ fileName;
    BString originalPath;

    if (this->isLink()) {
        nativePath = nativePath + EXT_LINK;
    }

    for (U32 i=0;i<3;i++) {
        if (Fs::doesNativePathExist(nativePath)) {
            std::shared_ptr<FsNode> existingNode = Fs::getNodeFromLocalPath(B(""), path, false);
            if (!existingNode) {
                // we must be on windows, nativePath and path are case insensitive the same, but are different case
                //            
                originalPath = path;
            } else {
                if (existingNode->isDirectory()!=this->isDirectory()) {
                    if (existingNode->isDirectory())
                        return -K_EISDIR;
                    return -K_ENOTDIR;
                }
                if (existingNode->isDirectory()) {
                    result = existingNode->removeDir();
                    if (result!=0)
                        return result;
                } else {
                    existingNode->remove();
                }
            }
        }
        if (originalPath.length()) {     
            nativePath = nativePath+EXT_MIXED;
        }    
        result = ::rename(this->nativePath.c_str(), nativePath.c_str());
        if (result==0) {
            BString dosAttrib = this->nativePath + EXT_DOSATTRIB;
            if (Fs::doesNativePathExist(dosAttrib)) {
                BString dosAttribDst = nativePath + EXT_DOSATTRIB;
                ::rename(dosAttrib.c_str(), dosAttribDst.c_str());
            }
            this->removeNodeFromParent();
            this->path = path;
            this->nativePath = nativePath;
            this->name = Fs::getFileNameFromPath(path);
            BString parentPath = Fs::getParentPath(path);
            std::shared_ptr<FsNode> parentNode = Fs::getNodeFromLocalPath(B(""), parentPath, false);
            parentNode->addChild(shared_from_this());
            this->parent = parentNode;            
        } else {
            result = -translateErr(errno);
            KNativeThread::sleep(100); // not sure why on Windows doesNativePathExist fails sometimes, debian stretch /usr/bin/dpkg -i /var/local/xbitmaps_1.1.1-2_all.deb seems to cause this in a reproducible way
            continue;
        }
        break;
    }
    int i=0;
    this->openNodes.for_each([&tmpPos,&i](KListNode<FsOpenNode*>* n) {
        if (i<(int)tmpPos.size() && tmpPos.at(i)!=-1) {
            FsOpenNode* openNode = n->data;
            if (openNode) {
                openNode->reopen();
                openNode->seek(tmpPos.at(i));
            }
        }
        i++;
    });
    return result;
}

U32 FsFileNode::removeDir() {
    if (!this->isDirectory() || this->isLink())
        return -K_ENOTDIR;
    if (this->getChildCount()) {
        return -K_ENOTEMPTY;
    }    
    if (Fs::doesNativePathExist(this->nativePath) && ::rmdir(this->nativePath.c_str()) < 0) {
        return -translateErr(errno);
    }
    std::shared_ptr<FsNode> parent = this->getParent().lock();
    if (parent) {
        parent->removeChildByName(this->name);
    }
    return 0;
}

U32 FsFileNode::setTimes(U64 lastAccessTime, U32 lastAccessTimeNano, U64 lastModifiedTime, U32 lastModifiedTimeNano) {
    struct utimbuf settime = {0, 0};

    this->ensurePathIsLocal(false);
    if (lastAccessTime) {
        settime.actime = lastAccessTime;
    }
    if (lastModifiedTime) {
        settime.modtime = lastModifiedTime;
    }       
    utime(this->nativePath.c_str(),&settime);
    return 0; // no error checking, we don't care if this fails
}
