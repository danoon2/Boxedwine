#ifdef BOXEDWINE_ZLIB

#ifndef __FSZIPENODE_H__
#define __FSZIPNODE_H__

#include "fsnode.h"
#include "fszip.h"

class FsFileNode;

class FsZipNode : public std::enable_shared_from_this<FsZipNode> {
public:
    FsZipNode(const fsZipInfo& zipInfo, const std::shared_ptr<FsZip>& fsZip);
    U64 lastModified();
    U64 length();
    FsOpenNode* open(std::shared_ptr<FsNode> node, U32 flags);
    bool moveToFileSystem(std::shared_ptr<FsNode> node);

    std::weak_ptr<FsZip> fsZip;
private:
    fsZipInfo zipInfo;
};
#endif
#endif
