#ifndef __FS_H__
#define __FS_H__

#include "platform.h"
#include "fsnode.h"
#include "fsopennode.h"

#define K_O_RDONLY   0x0000
#define K_O_WRONLY   0x0001
#define K_O_RDWR     0x0002
#define K_O_ACCMODE  0x0003

#define K_O_CREAT	   0x0040
#define K_O_EXCL	   0x0080
#define K_O_TRUNC	   0x0200
#define K_O_APPEND     0x0400

// can change after open
#define K_O_NONBLOCK  0x000800
#define K_O_ASYNC     0x002000
#define K_O_LARGEFILE 0x008000
#define K_O_DIRECTORY 0x010000
#define K_O_CLOEXEC   0x080000	
#define K_O_TMPFILE   0x400000

#define FD_CLOEXEC 1

#define K_F_SEAL_SEAL           0x01
#define K_F_SEAL_SHRINK         0x02
#define K_F_SEAL_GROW           0x04
#define K_F_SEAL_WRIT           0x08
#define K_F_SEAL_FUTURE_WRITE   0x10

// type of lock
#define K_F_RDLCK	   0
#define K_F_WRLCK	   1
#define K_F_UNLCK	   2

#define IOCTL_ARG1 EDX
#define IOCTL_ARG2 ESI
#define IOCTL_ARG3 EDI
#define IOCTL_ARG4 EBP

#define FS_BLOCK_SIZE 8192

typedef FsOpenNode* (*OpenVirtualNode)(const std::shared_ptr<FsNode>& node, U32 flags, U32 data);

class FsFileNode;

#define k_mdev(x,y) ((x << 8) | y)

class Fs {
public:   
    static bool initFileSystem(const BString& rootPath);
    static std::shared_ptr<FsNode> getNodeFromLocalPath(const BString& currentDirectory, const BString& path, bool followLink, bool* isLink=nullptr);    
    static std::shared_ptr<FsFileNode> addFileNode(const BString& path, const BString& link, const BString& nativePath, bool isDirectory, const std::shared_ptr<FsNode>& parent);
    static std::shared_ptr<FsNode> addVirtualFile(const BString& path, std::function<FsOpenNode*(const std::shared_ptr<FsNode>& node, U32 flags, U32 data)> func, U32 mode, U32 rdev, const std::shared_ptr<FsNode>& parent, U32 data=0);
    static std::shared_ptr<FsNode> addVirtualFile(const BString& path, U32 mode, U32 rdev, const std::shared_ptr<FsNode>& parent, const BString& value);
    static std::shared_ptr<FsNode> addDynamicLinkFile(const BString& path, U32 rdev, const std::shared_ptr<FsNode>& parent, bool isDirectory, std::function<BString(void)> fnGetLink);
    static std::shared_ptr<FsNode> addDynamicLinkFile(const BString& path, U32 rdev, const std::shared_ptr<FsNode>& parent, bool isDirectory, const BString& link);
    static std::shared_ptr<FsNode> addRootDirectoryNode(const BString& path, const BString& nativePath, const std::shared_ptr<FsNode>& parent);
    static void remoteNameToLocal(BString& path);
    static void localNameToRemote(BString& path);
    static BString localFromNative(const BString& path);
    static BString nativeFromLocal(const BString& path);
    static U32 makeLocalDirs(const BString& path);
    static bool makeNativeDirs(const BString& path);
    static U32 deleteNativeFile(const BString& path);
    static U32 deleteNativeDirAndAllFilesInDir(const BString& path);
    static U32 iterateAllNativeFiles(const BString& path, bool recursive, bool includeDirs, std::function<U32(BString filePath,bool isDir)> f);
    static BString getParentPath(const BString& path);
    static BString getNativeParentPath(const BString& path);
    static BString getFileNameFromPath(const BString& path);
    static BString getFileNameFromNativePath(const BString& path);
    static U32 readNativeFile(const BString& nativePath, U8* buffer, U32 bufferLen);
    static void splitPath(const BString& path, std::vector<BString>& parts);
    static bool doesNativePathExist(const BString& path);
    static bool isNativeDirectoryEmpty(const BString& path);
    static U64 getNativeDirectorySize(const BString& path, bool recursive);
    static U64 getNativeFileSize(const BString& path);
    static bool isNativePathDirectory(const BString& path);
    static BString getFullPath(const BString& currentDirectory, const BString& path);
    static BString getNativePathFromParentAndLocalFilename(const std::shared_ptr<FsNode>& parent, const BString& fileName);    
    static std::vector<BString> getFilesInNativeDirectoryWhereFileMatches(const BString& dirPath, const BString& startsWith, const BString& endsWith, bool ignoreCase);
    static BString trimTrailingSlash(const BString& s);

    static BString nativePathSeperator;

    static BString getDosAttrib(const std::shared_ptr<FsNode>& file);
    static void setDosAttrib(const std::shared_ptr<FsNode>& file, const BString& attrib);
    static U32 removeDosAttrib(const std::shared_ptr<FsNode>& file);

    static std::shared_ptr<FsFileNode> rootNode;
	static void shutDown();
private:
    friend class KUnixSocketObject;

    static std::shared_ptr<FsNode> getNodeFromLocalPath(const BString& currentDirectory, const BString& path, std::shared_ptr<FsNode>* lastNode, std::vector<BString>* missingParts, bool followLink, bool* isLink= nullptr);

    static std::atomic_int nextNodeId;
};

#endif
