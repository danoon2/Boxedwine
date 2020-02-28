#include "boxedwine.h"

#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <SDL.h>

#include UNISTD
#include UTIME
#include MKDIR_INCLUDE

#include "fsfilenode.h"
#include "fsdiropennode.h"
#include "fsfileopennode.h"
#include "kstat.h"
#include "fszipnode.h"

std::set<std::string> FsFileNode::nonExecFileFullPaths;

FsFileNode::FsFileNode(U32 id, U32 rdev, const std::string& path, const std::string& link, const std::string& nativePath, bool isDirectory, bool isRootPath, BoxedPtr<FsNode> parent) : FsNode(File, id, rdev, path, link, nativePath, isDirectory, parent), isRootPath(isRootPath) {
}

std::string FsFileNode::getNativeTmpPath() {
    std::string localPath;
    std::string nativePath;
    getTmpPath(nativePath, localPath);
    return nativePath;
}

std::string FsFileNode::getLocalTmpPath() {
    std::string localPath;
    std::string nativePath;
    getTmpPath(nativePath, localPath);
    return localPath;
}

static U32 nextTmpId;

void FsFileNode::getTmpPath(std::string& nativePath, std::string& localPath) {
    BOXEDWINE_CRITICAL_SECTION;
    Fs::makeLocalDirs("/tmp/del");
    BoxedPtr<FsNode> delDir = Fs::getNodeFromLocalPath("", "/tmp/del", true);

    for (;nextTmpId<100000000;nextTmpId++) {
        std::string name = "del"+std::to_string(nextTmpId)+".tmp";
        Fs::localNameToRemote(name);
        nativePath = delDir->nativePath+Fs::nativePathSeperator+name;
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

    if (exists)
        result = unlink(nativePath.c_str())==0;
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
        
        Fs::makeLocalDirs("/tmp/del");
        BoxedPtr<FsNode> delDir = Fs::getNodeFromLocalPath("", "/tmp/del", true);

        std::string newNativePath = FsFileNode::getNativeTmpPath();

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

void FsFileNode::ensurePathIsLocal() {
#ifdef BOXEDWINE_ZLIB
    BOXEDWINE_CRITICAL_SECTION;
    if (this->zipNode && !Fs::doesNativePathExist(this->nativePath)) {
        if (this->isDirectory()) {
            Fs::makeLocalDirs(this->path);
        } else {
            std::string parentPath = Fs::getParentPath(this->path);
            Fs::makeLocalDirs(parentPath.c_str());
            this->zipNode->moveToFileSystem();
        }
    } else if (this->parent->type==File) {
        std::string parentPath = Fs::getParentPath(this->path);
        Fs::makeLocalDirs(parentPath);
    }
#endif
}

FsOpenNode* FsFileNode::open(U32 flags) {
    U32 openFlags = O_BINARY;
    U32 f;
            
    if (this->isDirectory()) {
        return new FsDirOpenNode(Fs::getNodeFromLocalPath("", this->path, true), flags);
    }
    if ((flags & K_O_ACCMODE)==K_O_RDONLY) {
        openFlags|=O_RDONLY;
    } else {
        if ((flags & K_O_ACCMODE)==K_O_WRONLY) {
            openFlags|=O_WRONLY;        
        } else {
            openFlags|=O_RDWR;            
        }
        std::string parentPath = Fs::getNativeParentPath(this->nativePath);
        if (!Fs::doesNativePathExist(parentPath)) {
            ensurePathIsLocal();
        }
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
    f = ::open(this->nativePath.c_str(), openFlags, 0666);	
    if (!f || f==0xFFFFFFFF) {
#ifdef BOXEDWINE_ZLIB
        if (this->zipNode && (flags & K_O_ACCMODE)==K_O_RDONLY)
            return this->zipNode->open(flags);
#endif
        return 0;
    }
    return new FsFileOpenNode(this, flags, f);
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
    if (!FsFileNode::nonExecFileFullPaths.count(this->path)) {
        result|=K__S_IEXEC;
    }
    if (KThread::currentThread()->process->userId == 0 ||  stringStartsWith(this->path, "/tmp") ||  stringStartsWith(this->path, "/var") ||  stringStartsWith(this->path, "/home")) {
        result|=K__S_IWRITE;
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

U32 FsFileNode::rename(const std::string& path) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(this->openNodesMutex);
    U32 result;
    S64* tmpPos = NULL;

    this->ensurePathIsLocal();
    if (this->openNodes.size()) {
        int i=0;
        tmpPos = new S64[this->openNodes.size()];
        for (U32 x=0;x<this->openNodes.size();x++) {
            tmpPos[x]=-1;
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

    BoxedPtr<FsNode> parent = Fs::getNodeFromLocalPath("", Fs::getParentPath(path), true);
    if (!parent) {
        return -K_ENOENT;
    }

    std::string fileName = Fs::getFileNameFromPath(path);
    Fs::localNameToRemote(fileName);
    std::string nativePath = parent->nativePath+Fs::nativePathSeperator+fileName;
    std::string originalPath;

    if (this->isLink())
        nativePath+=".link";

    for (U32 i=0;i<3;i++) {
        if (Fs::doesNativePathExist(nativePath)) {
            BoxedPtr<FsNode> existingNode = Fs::getNodeFromLocalPath("", path, false);
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
            nativePath = nativePath+".mixed";
        }    
        result = ::rename(this->nativePath.c_str(), nativePath.c_str());
        if (result==0) {
            this->removeNodeFromParent();
            this->path = path;
            this->nativePath = nativePath;
            this->name = Fs::getFileNameFromPath(path);
            std::string parentPath = Fs::getParentPath(path);
            BoxedPtr<FsNode> parentNode = Fs::getNodeFromLocalPath("", parentPath, false);
            parentNode->addChild(this);
            this->parent = parentNode;
        } else {
            result = -translateErr(errno);
            SDL_Delay(100); // not sure why on Windows doesNativePathExist fails sometimes, debian stretch /usr/bin/dpkg -i /var/local/xbitmaps_1.1.1-2_all.deb seems to cause this in a reproducible way
            continue;
        }
        break;
    }
    int i=0;
    this->openNodes.for_each([&tmpPos,&i](KListNode<FsOpenNode*>* n) {
        if (tmpPos[i]!=-1) {
            FsOpenNode* openNode = n->data;
            openNode->reopen();
            openNode->seek(tmpPos[i]);
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
    this->getParent()->removeChildByName(this->name);
    return 0;
}

U32 FsFileNode::setTimes(U64 lastAccessTime, U32 lastAccessTimeNano, U64 lastModifiedTime, U32 lastModifiedTimeNano) {
    struct utimbuf settime = {0, 0};

    this->ensurePathIsLocal();
    if (lastAccessTime) {
        settime.actime = lastAccessTime;
    }
    if (lastModifiedTime) {
        settime.modtime = lastModifiedTime;
    }       
    utime(this->nativePath.c_str(),&settime);
    return 0; // no error checking, we don't care if this fails
}
