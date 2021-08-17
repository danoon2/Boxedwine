#ifndef __FSDYNAMICLINKNODE_H__
#define __FSDYNAMICLINKNODE_H__

#include "fsnode.h"

class FsDynamicLinkNode : public FsNode {
public:
    FsDynamicLinkNode(U32 id, U32 rdev, const std::string& path, BoxedPtr<FsNode> parent, bool isDirectory, std::function<std::string(void)> getLink);
    virtual U32 rename(const std::string& path); //return 0 if success, else errno
    virtual bool remove();
    virtual U64 lastModified();
    virtual U64 length();
    virtual FsOpenNode* open(U32 flags);
    virtual U32 getType(bool checkForLink);
    virtual U32 getMode();
    virtual U32 removeDir();
    virtual U32 setTimes(U64 lastAccessTime, U32 lastAccessTimeNano, U64 lastModifiedTime, U32 lastModifiedTimeNano);
    virtual std::string getLink();
    virtual bool isLink();

    std::function<std::string(void)> fnGetLink;
};

#endif