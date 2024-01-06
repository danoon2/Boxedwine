#ifndef __FSVIRTUALNODE_H__
#define __FSVIRTUALNODE_H__

#include "fsnode.h"
#include "fs.h"

class FsVirtualNode : public FsNode {
public:
    FsVirtualNode(U32 id, U32 rdev, BString path, OpenVirtualNode openFunc, U32 mode, BoxedPtr<FsNode> parent, U32 openData=0) : FsNode(Virtual, id, rdev, path, B(""), B(""), false, parent), mode(mode), openData(openData), openFunc(openFunc) {}
    virtual U32 rename(BString path); //return 0 if success, else errno
    virtual bool remove();
    virtual U64 lastModified();
    virtual U64 length();
    virtual FsOpenNode* open(U32 flags);
    virtual U32 getType(bool checkForLink);
    virtual U32 getMode();
    virtual U32 removeDir();
    virtual U32 setTimes(U64 lastAccessTime, U32 lastAccessTimeNano, U64 lastModifiedTime, U32 lastModifiedTimeNano);

    BString data;
private:
    const U32 mode;
    const U32 openData;
    const OpenVirtualNode openFunc;
};

#endif