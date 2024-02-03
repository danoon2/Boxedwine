#include "boxedwine.h"

#include "fsvirtualnode.h"
#include "kstat.h"

bool FsVirtualNode::remove() {
    return false;
}

U64 FsVirtualNode::lastModified() {
    return 0;
}

U64 FsVirtualNode::length() {
    return 0;
}

U32 FsVirtualNode::getMode() {
    return this->mode;
}

FsOpenNode* FsVirtualNode::open(U32 flags) {
    if ((flags & K_O_ACCMODE)==K_O_RDONLY) {
        if (!this->canRead())
            return nullptr;
    } else if ((flags & K_O_ACCMODE)==K_O_WRONLY) {
        if (!this->canWrite())
            return nullptr;
    } else {
        if (!this->canWrite())
            return nullptr;
        if (!this->canRead())
            return nullptr;
    }
    if (flags & K_O_CREAT) {
        //return 0;
    }
    if (flags & K_O_EXCL) {
        kdebug("What about exclusive virtual files");
    }
    if (flags & K_O_TRUNC) {
        kdebug("What about truncating a virtual file");
    }
    if (flags & K_O_APPEND) {
        kdebug("What about appending a virtual file");
    }
    std::shared_ptr<FsNode> node = Fs::getNodeFromLocalPath(B(""), this->path, true);
    return this->openFunc(node, flags, this->openData);
}

U32 FsVirtualNode::getType(bool checkForLink) {
    if (this->isDirectory()) {
        return 4; // DT_DIR
    }
    if (this->mode & K__S_IFCHR) {
        return 2; // DT_CHR
    }
    if (this->mode & K_S_IFSOCK) {
        return 12; // DT_SOCK
    }
    return 8; // DT_REG
}

U32 FsVirtualNode::rename(BString path) {
    return -K_EIO;
}

U32 FsVirtualNode::removeDir() {
    kpanic("FsVirtualNode::removeDir not implemented");
    return 0;
}

U32 FsVirtualNode::setTimes(U64 lastAccessTime, U32 lastAccessTimeNano, U64 lastModifiedTime, U32 lastModifiedTimeNano) {
    klog("FsVirtualNode::setTimes not implemented");
    return 0;
}