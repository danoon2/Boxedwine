#ifndef __FSVIRTUALNODE_H__
#define __FSVIRTUALNODE_H__

#include "fsnode.h"
#include "fs.h"

class FsVirtualNode : public FsNode {
public:
    FsVirtualNode(U32 id, U32 rdev, const std::string& path, OpenVirtualNode openFunc, U32 mode, BoxedPtr<FsNode> parent) : FsNode(Virtual, id, rdev, path, "", false, parent), mode(mode), openFunc(openFunc) {}
    virtual U32 rename(const std::string& path); //return 0 if success, else errno
    virtual bool remove();
    virtual U64 lastModified();
    virtual U64 length();
    virtual FsOpenNode* open(U32 flags);
    virtual U32 getType(bool checkForLink);
    virtual U32 getMode();
    virtual U32 removeDir();
    virtual U32 setTimes(U64 lastAccessTime, U32 lastAccessTimeNano, U64 lastModifiedTime, U32 lastModifiedTimeNano);

    std::string data;
private:
    const U32 mode;
    const OpenVirtualNode openFunc;
};

#endif