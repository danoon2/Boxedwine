#include "boxedwine.h"

#ifdef BOXEDWINE_ZLIB

#include "fszipopennode.h"
#include "fszipnode.h"
#include "fszip.h"


FsZipOpenNode::FsZipOpenNode(std::shared_ptr<FsNode> node, std::shared_ptr<FsZipNode>& zipNode, U32 flags, U64 offset) : FsOpenNode(node, flags), zipNode(zipNode), pos(0), offset(offset) {
}

S64 FsZipOpenNode::length() {
    return this->node->length();
}

bool FsZipOpenNode::setLength(S64 len) {
    // if this file was open for write, it would have been copied to the file system
    kpanic("FsZipOpenNode::setLength should not have been called"); 
    return false;
}

S64 FsZipOpenNode::getFilePointer() {
    return this->pos;
}

S64 FsZipOpenNode::seek(S64 pos) {
    if (pos>(S64)this->node->length())
        this->pos = this->node->length();
    else
        this->pos = pos;
    return this->pos;
}

void FsZipOpenNode::close() {
}

bool FsZipOpenNode::isOpen() {
    return true;
}

U32 FsZipOpenNode::ioctl(KThread* thread, U32 request) {
    return -K_ENODEV;
}

void FsZipOpenNode::setAsync(bool isAsync) {
    if (isAsync)
        kdebug("FsZipOpenNode::setAsync not implemented");
}

bool FsZipOpenNode::isAsync() {
    return false;
}

void FsZipOpenNode::waitForEvents(BOXEDWINE_CONDITION& parentCondition, U32 events) {
    kdebug("FsZipOpenNode::waitForEvents not implemented");
}

bool FsZipOpenNode::isWriteReady() {
    return (this->flags & K_O_ACCMODE)!=K_O_RDONLY;
}

bool FsZipOpenNode::isReadReady() {
    return (this->flags & K_O_ACCMODE)!=K_O_WRONLY;
}

U32 FsZipOpenNode::map(KThread* thread, U32 address, U32 len, S32 prot, S32 flags, U64 off) {
    return 0;
}

bool FsZipOpenNode::canMap() {
    return true;
}

U32 FsZipOpenNode::readNative(U8* buffer, U32 len) {
    U32 result = 0;
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(*getReadMutex());
    std::shared_ptr<FsZip> fsZip = zipNode->fsZip.lock();
    if (fsZip) {
        fsZip->setupZipRead(this->offset, this->pos);
        result = unzReadCurrentFile(fsZip->zipfile, buffer, len);
        this->pos += result;
        fsZip->lastZipFileOffset = this->pos;
    }
    return result;
}

U32 FsZipOpenNode::writeNative(U8* buffer, U32 len) {
    kpanic("FsZipOpenNode::writeNative not implemented");
    return 0;
}

void FsZipOpenNode::reopen() {
    this->pos = 0;
}

BOXEDWINE_MUTEX* FsZipOpenNode::getReadMutex() {
    std::shared_ptr<FsZip> fsZip = zipNode->fsZip.lock();
    if (fsZip) {
        return &fsZip->readMutex;
    }
    return nullptr;
}

#endif
