#ifndef __FSMEMNODE_H__
#define __FSMEMNODE_H__

#include "fsnode.h"

class FsMemOpenNode;

class FsMemNode : public FsNode {
public:
    FsMemNode(U32 id, U32 rdev, BString path);
    virtual U32 rename(BString path); //return 0 if success, else errno
    virtual bool remove();
    virtual U64 lastModified();
    virtual U64 length();
    virtual FsOpenNode* open(U32 flags);
    virtual U32 getType(bool checkForLink);
    virtual U32 getMode();
    virtual U32 removeDir();
    virtual U32 setTimes(U64 lastAccessTime, U32 lastAccessTimeNano, U64 lastModifiedTime, U32 lastModifiedTimeNano);

    FsMemOpenNode* openNode;
};

#endif
