#include "boxedwine.h"
#include "fsmemnode.h"
#include "fsmemopennode.h"
#include "kstat.h"

FsMemNode::FsMemNode(U32 id, U32 rdev, BString path) : FsNode(Type::Memory, id, rdev, path, B(""), B(""), false, nullptr), openNode(nullptr) {
}

U32 FsMemNode::rename(BString path) {
    kpanic("FsMemNode::rename not supported");
    return -1;
}

bool FsMemNode::remove() {
    kpanic("FsMemNode::remove not supported");
    return false;
}

U64 FsMemNode::lastModified() {
    return this->openNode->lastModified();
}

U64 FsMemNode::length() {
    return this->openNode->length();
}

FsOpenNode* FsMemNode::open(U32 flags) {
    kpanic("FsMemNode::open not supported");
    return nullptr;
}

U32 FsMemNode::getType(bool checkForLink) {
    return 8; // DT_REG
}

U32 FsMemNode::getMode() {
    return K__S_IWRITE | K__S_IREAD | (this->getType(false) << 12);
}

U32 FsMemNode::removeDir() {
    kpanic("FsMemNode::removeDir not supported");
    return -1;
}

U32 FsMemNode::setTimes(U64 lastAccessTime, U32 lastAccessTimeNano, U64 lastModifiedTime, U32 lastModifiedTimeNano) {
    kpanic("FsMemNode::setTimes not supported");
    return 0;
}