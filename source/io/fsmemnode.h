#ifndef __FSMEMNODE_H__
#define __FSMEMNODE_H__

#include "fsnode.h"

class FsMemOpenNode;

class FsMemNode : public FsNode {
public:
    FsMemNode(U32 id, U32 rdev, BString path);
    virtual U32 rename(BString path) override; //return 0 if success, else errno
    virtual bool remove() override;
    virtual U64 lastModified() override;
    virtual U64 length() override;
    virtual FsOpenNode* open(U32 flags) override;
    virtual U32 getType(bool checkForLink) override;
    virtual U32 getMode() override;
    virtual U32 removeDir() override;
    virtual U32 setTimes(U64 lastAccessTime, U32 lastAccessTimeNano, U64 lastModifiedTime, U32 lastModifiedTimeNano) override;

    FsMemOpenNode* openNode;
};

#endif
