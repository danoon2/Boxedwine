#ifndef __FSZIPENODE_H__
#define __FSZIPNODE_H__

#include "fsnode.h"
#include "fszip.h"

class FsFileNode;

class FsZipNode : public FsNode {
public:
    FsZipNode(BoxedPtr<FsFileNode> fileNode, const fsZipInfo& zipInfo);
    virtual U32 rename(const std::string& path); //return 0 if success, else errno
    virtual bool remove();
    virtual U64 lastModified();
    virtual U64 length();
    virtual FsOpenNode* open(U32 flags);
    virtual bool canRead();
    virtual bool canWrite();
    virtual U32 getType(bool checkForLink);
    virtual U32 getMode();
    virtual U32 removeDir();
    virtual U32 setTimes(U64 lastAccessTime, U32 lastAccessTimeNano, U64 lastModifiedTime, U32 lastModifiedTimeNano);
    virtual void close();

    bool moveToFileSystem();
private:
    fsZipInfo zipInfo;
    BoxedPtr<FsFileNode> fileNode;
};

#endif