#include "boxedwine.h"
#include "fsmemopennode.h"

FsMemOpenNode::FsMemOpenNode(U32 flags, std::shared_ptr<FsNode> node) : FsOpenNode(node, flags), seals(0), pos(0), isClosed(false) {
    this->lastModifiedTime = KSystem::getSystemTimeAsMicroSeconds() / 1000l;
}

FsMemOpenNode::~FsMemOpenNode() {
    FsMemOpenNode::close();
}

S64 FsMemOpenNode::length() {
    return (S64)this->buffer.size();
}

bool FsMemOpenNode::setLength(S64 length) {
    this->lastModifiedTime = KSystem::getSystemTimeAsMicroSeconds() / 1000l;
    this->buffer.resize((U32)length, 0);
    return true;
}

S64 FsMemOpenNode::getFilePointer() {
    return pos;
}

S64 FsMemOpenNode::seek(S64 pos) {
    this->pos = pos;
    return this->pos;
}

U32 FsMemOpenNode::map(KThread* thread, U32 address, U32 len, S32 prot, S32 flags, U64 off) {
    return 0;
}

bool FsMemOpenNode::canMap() {
    return true;
}

U32 FsMemOpenNode::ioctl(KThread* thread, U32 request) {
    return -K_ENODEV;
}

void FsMemOpenNode::setAsync(bool isAsync) {
    if (isAsync)
        kdebug("FsMemOpenNode::setAsync not implemented");
}

bool FsMemOpenNode::isAsync() {
    return false;
}

void FsMemOpenNode::waitForEvents(BOXEDWINE_CONDITION& parentCondition, U32 events) {
    kdebug("FsMemOpenNode::::waitForEvents not implemented");
}

bool FsMemOpenNode::isWriteReady() {
    return (this->flags & K_O_ACCMODE)!=K_O_RDONLY;
}

bool FsMemOpenNode::isReadReady() {
    return (this->flags & K_O_ACCMODE)!=K_O_WRONLY;
}

U32 FsMemOpenNode::readNative(U8* buffer, U32 len) {
    S32 todo = (S32)len;
    if (todo > (S32)this->buffer.size() - this->pos) {
        todo = (S32)(this->buffer.size()-this->pos);
    }
    if (todo>0) {
        memcpy(buffer, &this->buffer[(U32)this->pos], todo);
        this->pos+=todo;
        return (U32)todo;
    }    
    return 0;
}

U32 FsMemOpenNode::writeNative(U8* buffer, U32 len) {
    if (len==0)
        return 0;
    this->lastModifiedTime = KSystem::getSystemTimeAsMicroSeconds() / 1000l;
    if (this->pos < (S64)this->buffer.size()) {
        U32 todo = len;
        if (this->buffer.size()-(U64)this->pos < len) {
            todo = (U32)(this->buffer.size()-this->pos);
        }
        len -= todo;
        memcpy(&this->buffer[(U32)this->pos], buffer, todo);
        this->pos+=todo;
        buffer+=todo;
    }
    if (len) {
        std::copy(buffer, buffer+len, std::back_inserter(this->buffer));
    }
    return len;
}

void FsMemOpenNode::close() {
    this->isClosed = true;
}

void FsMemOpenNode::reopen() {
    this->isClosed = false;
}

bool FsMemOpenNode::isOpen() {
    return !this->isClosed;
}

U32 FsMemOpenNode::addSeals(U32 seals) {
    if (this->seals & K_F_SEAL_SEAL) {
        return -K_EPERM;
    }
    return 0;
}
