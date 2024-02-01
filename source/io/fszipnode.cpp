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
    bool result = false;
    U32 to = _open(node->nativePath.c_str(), O_WRONLY | O_CREAT, 0666);
    if (to != 0xFFFFFFFF) {
        U8 buffer[4096];
        U32 read = from->readNative(buffer, 4096);
        while (read) {
            if (_write(to, buffer, read) != (int)read)
                return false;
            read = from->readNative(buffer, 4096);
        }
        _close(to);
        result = true;
    }
    from->close();
    return result;
}

U64 FsZipNode::lastModified() {
    return this->zipInfo.lastModified;
}

U64 FsZipNode::length() {
    return this->zipInfo.length;
}

FsOpenNode* FsZipNode::open(BoxedPtr<FsNode> node, U32 flags) {
    std::shared_ptr<FsZipNode> zipNode = shared_from_this();
    return new FsZipOpenNode(node, zipNode, flags, (U64)this->zipInfo.offset);
}

#endif
