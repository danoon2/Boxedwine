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

    FsFileNode(U32 id, U32 rdev, BString path, BString link, BString nativeRootPath, bool isDirectory, bool isRootPath, std::shared_ptr<FsNode> parent);

    // from FsNode
    U32 rename(BString path) override; //return 0 if success, else errno
    bool remove() override;
    U64 lastModified() override;
    U64 length() override;
    FsOpenNode* open(U32 flags) override;
    U32 getType(bool checkForLink) override;
    U32 getMode() override;
    U32 removeDir() override;
    BString getLink() override;

    U32 setTimes(U64 lastAccessTime, U32 lastAccessTimeNano, U64 lastModifiedTime, U32 lastModifiedTimeNano) override;

    static std::set<BString> nonExecFileFullPaths;
private:
    friend class FsFileOpenNode;
    friend class FsDirOpenNode;
    friend class Platform;

    void ensurePathIsLocal(bool prepareForWrite);
#ifdef BOXEDWINE_ZLIB
    friend class FsZip;
    friend class FsZipNode;    
    std::shared_ptr<FsZipNode> zipNode;
#endif    
    friend class Fs;
    bool isRootPath;
};

#endif
