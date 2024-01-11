#ifndef __FSFILENODE_H__
#define __FSFILENODE_H__

#include "fsnode.h"

#ifdef BOXEDWINE_ZLIB
class FsZipNode;
#endif

S32 translateErr(U32 e);

class FsFileNode : public FsNode {
public:
    static void getTmpPath(BString& nativePath, BString& localPath);
    static BString getNativeTmpPath();
    static BString getLocalTmpPath();

    FsFileNode(U32 id, U32 rdev, BString path, BString link, BString nativeRootPath, bool isDirectory, bool isRootPath, BoxedPtr<FsNode> parent);
    virtual U32 rename(BString path) override; //return 0 if success, else errno
    virtual bool remove() override;
    virtual U64 lastModified() override;
    virtual U64 length() override;
    virtual FsOpenNode* open(U32 flags) override;
    virtual U32 getType(bool checkForLink) override;
    virtual U32 getMode() override;
    virtual U32 removeDir() override;
    virtual U32 setTimes(U64 lastAccessTime, U32 lastAccessTimeNano, U64 lastModifiedTime, U32 lastModifiedTimeNano) override;
    static std::set<BString> nonExecFileFullPaths;
private:
    friend class FsFileOpenNode;
    friend class FsDirOpenNode;
    friend class Platform;

    void ensurePathIsLocal();
#ifdef BOXEDWINE_ZLIB
    friend class FsZip;
    friend class FsZipNode;    
    std::shared_ptr<FsZipNode> zipNode;
#endif    
    friend class Fs;
    bool isRootPath;
};

#endif
