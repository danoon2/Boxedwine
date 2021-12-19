#include "boxedwine.h"
#include "fsdynamiclinknode.h"
#include "fsfilenode.h"
#include "kstat.h"

FsDynamicLinkNode::FsDynamicLinkNode(U32 id, U32 rdev, const std::string& path, BoxedPtr<FsNode> parent, bool isDirectory, std::function<std::string(void)> fnGetLink) : FsNode(FsNode::Type::Virtual, id, rdev, path, "", "", isDirectory, parent), fnGetLink(fnGetLink) {
}

U32 FsDynamicLinkNode::rename(const std::string& path) {
    return -1;
}

bool FsDynamicLinkNode::remove() {
    return false;
}

U64 FsDynamicLinkNode::lastModified() {
    return KSystem::getSystemTimeAsMicroSeconds() / 1000l;
}

bool FsDynamicLinkNode::isLink() {
    return true;
}

U64 FsDynamicLinkNode::length() {
    return getLink().length();
}

FsOpenNode* FsDynamicLinkNode::open(U32 flags) {
    return NULL;
}

U32 FsDynamicLinkNode::getType(bool checkForLink) {
    return 10; // DT_LINK
}

U32 FsDynamicLinkNode::getMode() {
    return K__S_IREAD | K__S_IEXEC | K__S_IWRITE;
}

U32 FsDynamicLinkNode::removeDir() {
    return -1;
}

U32 FsDynamicLinkNode::setTimes(U64 lastAccessTime, U32 lastAccessTimeNano, U64 lastModifiedTime, U32 lastModifiedTimeNano) {
    return 0;
}

std::string FsDynamicLinkNode::getLink() {
    return this->fnGetLink();
}