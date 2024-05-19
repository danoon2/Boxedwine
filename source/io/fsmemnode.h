#ifndef __FSMEMNODE_H__
#define __FSMEMNODE_H__

#include "fsnode.h"

class FsMemOpenNode;

class FsMemNode : public FsNode {
public:
    FsMemNode(U32 id, U32 rdev, BString path);

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

    FsMemOpenNode* openNode;
};

#endif
