#ifndef __FSDYNAMICLINKNODE_H__
#define __FSDYNAMICLINKNODE_H__

#include "fsnode.h"

class FsDynamicLinkNode : public FsNode {
public:
    FsDynamicLinkNode(U32 id, U32 rdev, BString path, std::shared_ptr<FsNode> parent, bool isDirectory, std::function<BString(void)> getLink);

    // from FsNode
    U32 rename(BString path) override; //return 0 if success, else errno
    bool remove() override;
    U64 lastModified() override;
    U64 length() override;
    FsOpenNode* open(U32 flags) override;
    U32 getType(bool checkForLink) override;
    U32 getMode() override;
    U32 removeDir() override;
    U32 setTimes(U64 lastAccessTime, U32 lastAccessTimeNano, U64 lastModifiedTime, U32 lastModifiedTimeNano) override;
    BString getLink() override;
    bool isLink() override;

    std::function<BString(void)> fnGetLink;
};

#endif