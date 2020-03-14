#include "boxedwine.h"
#ifdef BOXEDWINE_ZLIB
#include "fszipnode.h"
#include "fsfilenode.h"
#include UNISTD
#include <fcntl.h>
#include "fszipopennode.h"

FsZipNode::FsZipNode(const fsZipInfo& zipInfo, std::shared_ptr<FsZip>& fsZip) : fsZip(fsZip) {
    this->zipInfo = zipInfo;
}

bool FsZipNode::moveToFileSystem(BoxedPtr<FsNode> node) {
    if (node->isDirectory())
        return false;
    FsOpenNode* from = this->open(node, K_O_RDONLY);
    U32 to = ::open(node->nativePath.c_str(), O_WRONLY | O_CREAT, 0666);
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

U64 FsZipNode::lastModified() {
    return this->zipInfo.lastModified;
}

U64 FsZipNode::length() {
    return this->zipInfo.length;
}

FsOpenNode* FsZipNode::open(BoxedPtr<FsNode> node, U32 flags) {
    return new FsZipOpenNode(node, shared_from_this(), flags, this->zipInfo.offset);
}

#endif
