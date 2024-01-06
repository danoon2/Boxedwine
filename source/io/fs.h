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

typedef FsOpenNode* (*OpenVirtualNode)(const BoxedPtr<FsNode>& node, U32 flags, U32 data);

class FsFileNode;

class Fs {
public:   
    static bool initFileSystem(BString rootPath);
    static BoxedPtr<FsNode> getNodeFromLocalPath(BString currentDirectory, BString path, bool followLink, bool* isLink=NULL);    
    static BoxedPtr<FsNode> addFileNode(BString path, BString link, BString nativePath, bool isDirectory, const BoxedPtr<FsNode>& parent);
    static BoxedPtr<FsNode> addVirtualFile(BString path, OpenVirtualNode func, U32 mode, U32 rdev, const BoxedPtr<FsNode>& parent, U32 data=0);
    static BoxedPtr<FsNode> addDynamicLinkFile(BString path, U32 rdev, const BoxedPtr<FsNode>& parent, bool isDirectory, std::function<BString(void)> fnGetLink);
    static BoxedPtr<FsNode> addRootDirectoryNode(BString path, BString nativePath, const BoxedPtr<FsNode>& parent);
    static void remoteNameToLocal(BString& path);
    static void localNameToRemote(BString& path);
    static BString localFromNative(const BString& path);
    static BString nativeFromLocal(const BString& path);
    static U32 makeLocalDirs(BString path);
    static bool makeNativeDirs(BString path);
    static U32 deleteNativeFile(BString path);
    static U32 deleteNativeDirAndAllFilesInDir(BString path);
    static U32 iterateAllNativeFiles(BString path, bool recursive, bool includeDirs, std::function<U32(BString filePath,bool isDir)> f);
    static BString getParentPath(BString path);
    static BString getNativeParentPath(BString path);
    static BString getFileNameFromPath(BString path);
    static BString getFileNameFromNativePath(BString path);
    static U32 readNativeFile(BString nativePath, U8* buffer, U32 bufferLen);
    static void splitPath(BString path, std::vector<BString>& parts);
    static bool doesNativePathExist(BString path);
    static bool isNativeDirectoryEmpty(BString path);
    static U64 getNativeDirectorySize(BString path, bool recursive);
    static U64 getNativeFileSize(BString path);
    static bool isNativePathDirectory(BString path);
    static BString getFullPath(BString currentDirectory, BString path);
    static BString getNativePathFromParentAndLocalFilename(const BoxedPtr<FsNode>& parent, const BString fileName);    
    static std::vector<BString> getFilesInNativeDirectoryWhereFileMatches(BString dirPath, BString startsWith, BString endsWith, bool ignoreCase);
    static BString trimTrailingSlash(BString s);

    static BString nativePathSeperator;

    static BoxedPtr<FsFileNode> rootNode;
	static void shutDown();
private:
    friend class KUnixSocketObject;

    static BoxedPtr<FsNode> getNodeFromLocalPath(BString currentDirectory, BString path, BoxedPtr<FsNode>& lastNode, std::vector<BString>& missingParts, bool followLink, bool* isLink=NULL);

    static std::atomic_int nextNodeId;
};

#endif
