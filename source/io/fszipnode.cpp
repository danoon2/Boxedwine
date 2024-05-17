#include "boxedwine.h"
#ifdef BOXEDWINE_ZLIB
#include "fszipnode.h"
#include "fsfilenode.h"
#include UNISTD
#include <fcntl.h>
#include "fszipopennode.h"
#include UTIME

FsZipNode::FsZipNode(const fsZipInfo& zipInfo, const std::shared_ptr<FsZip>& fsZip) : fsZip(fsZip) {
    this->zipInfo = zipInfo;
}

bool FsZipNode::moveToFileSystem(std::shared_ptr<FsNode> node) {
    if (node->isDirectory())
        return false;
    if (node->isLink()) {
        U32 to = ::open((node->nativePath+EXT_LINK).c_str(), O_WRONLY | O_CREAT | O_BINARY, 0666);
        ::write(to, node->link.c_str(), node->link.length());
        ::close(to);
        return true;
    }

    FsOpenNode* from = this->open(node, K_O_RDONLY);
    bool result = false;
    U32 to = ::open(node->nativePath.c_str(), O_WRONLY | O_CREAT | O_BINARY, 0666);
    if (to != 0xFFFFFFFF) {
        U8 buffer[4096];
        U32 read = from->readNative(buffer, 4096);
        while (read) {
            if (::write(to, buffer, read) != (int)read)
                return false;
            read = from->readNative(buffer, 4096);
        }
        ::close(to);

        struct utimbuf settime = { 0, 0 };

        settime.actime = this->zipInfo.lastModified / 1000;
        settime.modtime = this->zipInfo.lastModified / 1000;
        utime(node->nativePath.c_str(), &settime);

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

FsOpenNode* FsZipNode::open(std::shared_ptr<FsNode> node, U32 flags) {
    std::shared_ptr<FsZipNode> zipNode = shared_from_this();
    return new FsZipOpenNode(node, zipNode, flags, (U64)this->zipInfo.offset);
}

#endif
