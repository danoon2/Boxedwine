#ifndef __FSFILENODE_H__
#define __FSFILENODE_H__

#include "fsnode.h"

#ifdef BOXEDWINE_ZLIB
class FsZipNode;
#endif

S32 translateErr(U32 e);

class FsFileNode : public FsNode {
public:
    FsFileNode(U32 id, U32 rdev, const std::string& path, const std::string& link, const std::string& nativeRootPath, bool isDirectory, bool isRootPath, BoxedPtr<FsNode> parent);
    virtual U32 rename(const std::string& path); //return 0 if success, else errno
    virtual bool remove();
    virtual U64 lastModified();
    virtual U64 length();
    virtual FsOpenNode* open(U32 flags);
    virtual U32 getType(bool checkForLink);
    virtual U32 getMode();
    virtual U32 removeDir();
    virtual U32 setTimes(U64 lastAccessTime, U32 lastAccessTimeNano, U64 lastModifiedTime, U32 lastModifiedTimeNano);
    static std::set<std::string> nonExecFileFullPaths;
private:
    friend class FsFileOpenNode;
    friend class FsDirOpenNode;
    friend class Platform;

    void ensurePathIsLocal();
#ifdef BOXEDWINE_ZLIB
    friend class FsZip;
    friend class FsZipNode;    
    BoxedPtr<FsZipNode> zipNode;
#endif    
    friend class Fs;
    bool isRootPath;
};

#endif
