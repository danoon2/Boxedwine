#include "boxedwine.h"
#include "fsfileopennode.h"
#include UNISTD
#include <fcntl.h>
#include "fsfilenode.h"

FsFileOpenNode::FsFileOpenNode(BoxedPtr<FsFileNode> node, U32 flags, U32 handle) : FsOpenNode(node, flags), handle(handle), fileNode(node) {
}

FsFileOpenNode::~FsFileOpenNode() {
    this->close();
}

S64 FsFileOpenNode::length() {
    S32 currentPos = lseek(this->handle, 0, SEEK_CUR);
    S32 size = lseek(this->handle, 0, SEEK_END);
    lseek(this->handle, currentPos, SEEK_SET);
    return size;
}

bool FsFileOpenNode::setLength(S64 len) {
    return ftruncate(this->handle, (S32)len)==0;
}

S64 FsFileOpenNode::getFilePointer() {
    return lseek(this->handle, 0, SEEK_CUR);
}

S64 FsFileOpenNode::seek(S64 pos) {
    return lseek(this->handle, (S32)pos, SEEK_SET);
}

void FsFileOpenNode::close() {
    ::close(this->handle);
    this->handle = 0xFFFFFFFF;
}

void FsFileOpenNode::reopen() {
    int openFlags = 0;
    int flags = this->flags;
                        
    if ((flags & K_O_ACCMODE)==K_O_RDONLY) {
        openFlags|=O_RDONLY;
    } else if ((flags & K_O_ACCMODE)==K_O_WRONLY) {
        openFlags|=O_WRONLY;
    } else {
        openFlags|=O_RDWR;
    }
    if (flags & K_O_APPEND) {
        openFlags|=O_APPEND;
    }

    this->handle = ::open(this->fileNode->nativePath.c_str(), openFlags, 0666);
}

U32 FsFileOpenNode::ioctl(U32 request) {
    return -K_ENODEV;
}

void FsFileOpenNode::setAsync(bool isAsync) {
    if (isAsync)
        kwarn("FsFileOpenNode::setAsync not implemented");
}

bool FsFileOpenNode::isAsync() {
    return false;
}

void FsFileOpenNode::waitForEvents(U32 events) {
    kwarn("FsFileOpenNode::::waitForEvents not implemented");
}

bool FsFileOpenNode::isWriteReady() {
    return (this->flags & K_O_ACCMODE)!=K_O_RDONLY;
}

bool FsFileOpenNode::isReadReady() {
    return (this->flags & K_O_ACCMODE)!=K_O_WRONLY;
}

U32 FsFileOpenNode::map(U32 address, U32 len, S32 prot, S32 flags, U64 off) {
    return 0;
}

bool FsFileOpenNode::canMap() {
    return true;
}

U32 FsFileOpenNode::readNative(U8* buffer, U32 len) {
    return ::read(this->handle, buffer, len);
}

U32 FsFileOpenNode::writeNative(U8* buffer, U32 len) {
    return ::write(this->handle, buffer, len);
}