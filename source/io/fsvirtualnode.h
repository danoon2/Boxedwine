#ifndef __FSVIRTUALNODE_H__
#define __FSVIRTUALNODE_H__

#include "fsnode.h"
#include "fs.h"

class FsVirtualNode : public FsNode {
public:
    FsVirtualNode(U32 id, U32 rdev, BString path, std::function<FsOpenNode* (const std::shared_ptr<FsNode>& node, U32 flags, U32 data)> openFunc, U32 mode, std::shared_ptr<FsNode> parent, U32 openData=0) : FsNode(Type::Virtual, id, rdev, path, B(""), B(""), false, parent), mode(mode), openData(openData), openFunc(openFunc) {}

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

    BString data;
private:
    const U32 mode;
    const U32 openData;
    std::function<FsOpenNode* (const std::shared_ptr<FsNode>& node, U32 flags, U32 data)> openFunc;
};

#endif