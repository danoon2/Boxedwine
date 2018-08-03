#include "boxedwine.h"

#include "fsfilenode.h"
#include "fsdiropennode.h"

FsDirOpenNode::FsDirOpenNode(BoxedPtr<FsNode> node, U32 flags) : FsOpenNode(node, flags), pos(0) {
}

S64 FsDirOpenNode::length() {	
    return this->node->getChildCount();
}

bool FsDirOpenNode::setLength(S64 len) {
    return false;
}

S64 FsDirOpenNode::getFilePointer() {
    return this->pos;
}

S64 FsDirOpenNode::seek(S64 pos) {
    if (pos>=0 && pos<=this->getDirectoryEntryCount())
        this->pos = (S32)pos;
    else
        this->pos = this->getDirectoryEntryCount();
    return this->pos;
}

void FsDirOpenNode::close() {
}

bool FsDirOpenNode::isOpen() {
    return true;
}

U32 FsDirOpenNode::ioctl(U32 request) {
    return -K_ENODEV;
}

void FsDirOpenNode::setAsync(bool isAsync) {
    if (isAsync)
        kwarn("FsDirOpenNode::setAsync not implemented");
}

bool FsDirOpenNode::isAsync() {
    return false;
}

void FsDirOpenNode::waitForEvents(U32 events) {
    kwarn("FsDirOpenNode::waitForEvents not implemented");
}

bool FsDirOpenNode::isWriteReady() {
    return false;
}

bool FsDirOpenNode::isReadReady() {
    return false;
}

U32 FsDirOpenNode::map(U32 address, U32 len, S32 prot, S32 flags, U64 off) {
    return 0;
}

bool FsDirOpenNode::canMap() {
    return false;
}

U32 FsDirOpenNode::readNative(U8* buffer, U32 len) {
    kpanic("FsDirOpenNode::readNative not implemented: %s", node->path);
    return 0;
}

U32 FsDirOpenNode::writeNative(U8* buffer, U32 len) {
    kpanic("FsDirOpenNode::writeNative not implemented: %s", node->path);
    return 0;
}

void FsDirOpenNode::reopen() {
    this->pos = 0;
}