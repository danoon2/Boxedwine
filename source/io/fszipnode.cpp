#include "boxedwine.h"
#ifdef BOXEDWINE_ZLIB
#include "fszipnode.h"
#include "fsfilenode.h"
#include UNISTD
#include <fcntl.h>
#include "fszipopennode.h"

FsZipNode::FsZipNode(BoxedPtr<FsFileNode> fileNode, const fsZipInfo& zipInfo) : FsNode(Zip, fileNode->id, fileNode->rdev, fileNode->path, fileNode->link, fileNode->isDirectory(), NULL), fileNode(fileNode) {
    this->zipInfo = zipInfo;
}

bool FsZipNode::moveToFileSystem() {
    if (this->isDirectory())
        return false;
    FsOpenNode* from = this->open(K_O_RDONLY);
    U32 to = ::open(this->fileNode->nativePath.c_str(), O_WRONLY | O_CREAT, 0666);
    U8 buffer[4096];
    U32 read = from->readNative(buffer, 4096);
    while (read) {
        if (::write(to, buffer, read)!=read)
            return false;
        read = from->readNative(buffer, 4096);
    }
    ::close(to);
    from->close();
    return true;
}

U32 FsZipNode::rename(const std::string& path) {
    kpanic("FsZipNode::rename should not have been called");
    return 0;
}

bool FsZipNode::remove() {
    kpanic("FsZipNode::remove should not have been called");
    return false;
}

U64 FsZipNode::lastModified() {
    return this->zipInfo.lastModified;
}

U64 FsZipNode::length() {
    return this->zipInfo.length;
}

FsOpenNode* FsZipNode::open(U32 flags) {
    return new FsZipOpenNode(this->fileNode->zipNode, flags, this->zipInfo.offset);
}

bool FsZipNode::canRead() {
    return this->fileNode->canRead();
}

bool FsZipNode::canWrite() {
    return this->fileNode->canWrite();
}

U32 FsZipNode::getType(bool checkForLink) {
    return this->fileNode->getType(checkForLink);
}

U32 FsZipNode::getMode() {
    return this->fileNode->getMode();
}

U32 FsZipNode::removeDir() {
    return -1;
}

U32 FsZipNode::setTimes(U64 lastAccessTime, U32 lastAccessTimeNano, U64 lastModifiedTime, U32 lastModifiedTimeNano) {
    return this->fileNode->setTimes(lastAccessTime, lastAccessTimeNano, lastModifiedTime, lastModifiedTimeNano);
}

void FsZipNode::close() {
}
#endif
