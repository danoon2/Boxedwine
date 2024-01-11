#ifndef __FSVIRTUALNODE_H__
#define __FSVIRTUALNODE_H__

#include "fsnode.h"
#include "fs.h"

class FsVirtualNode : public FsNode {
public:
    FsVirtualNode(U32 id, U32 rdev, BString path, OpenVirtualNode openFunc, U32 mode, BoxedPtr<FsNode> parent, U32 openData=0) : FsNode(Virtual, id, rdev, path, B(""), B(""), false, parent), mode(mode), openData(openData), openFunc(openFunc) {}
    virtual U32 rename(BString path) override; //return 0 if success, else errno
    virtual bool remove() override;
    virtual U64 lastModified() override;
    virtual U64 length() override;
    virtual FsOpenNode* open(U32 flags) override;
    virtual U32 getType(bool checkForLink) override;
    virtual U32 getMode() override;
    virtual U32 removeDir() override;
    virtual U32 setTimes(U64 lastAccessTime, U32 lastAccessTimeNano, U64 lastModifiedTime, U32 lastModifiedTimeNano) override;

    BString data;
private:
    const U32 mode;
    const U32 openData;
    const OpenVirtualNode openFunc;
};

#endif