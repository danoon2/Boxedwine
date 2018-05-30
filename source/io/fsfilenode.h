#ifndef __FSFILENODE_H__
#define __FSFILENODE_H__

#include "fsnode.h"

class FsZipNode;

class FsFileNode : public FsNode {
public:
    FsFileNode(U32 id, U32 rdev, const std::string& path, const std::string& link, bool isDirectory, BoxedPtr<FsNode> parent);
    virtual U32 rename(const std::string& path); //return 0 if success, else errno
    virtual bool remove();
    virtual U64 lastModified();
    virtual U64 length();
    virtual FsOpenNode* open(U32 flags);
    virtual U32 getType(bool checkForLink);
    virtual U32 getMode();
    virtual U32 removeDir();
    virtual U32 setTimes(U64 lastAccessTime, U32 lastAccessTimeNano, U64 lastModifiedTime, U32 lastModifiedTimeNano);
private:
    friend class FsFileOpenNode;
    friend class FsDirOpenNode;
    friend class FsZip;
    friend class FsZipNode;
    friend class Platform;

    void ensurePathIsLocal();
    BoxedPtr<FsZipNode> zipNode;
    std::string nativePath;
};

#endif