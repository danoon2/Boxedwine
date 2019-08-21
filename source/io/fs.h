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
#define K_O_NONBLOCK 0x0800
#define K_O_ASYNC 0x2000
#define K_O_CLOEXEC 0x80000

#define FD_CLOEXEC 1

// type of lock
#define K_F_RDLCK	   0
#define K_F_WRLCK	   1
#define K_F_UNLCK	   2

#define IOCTL_ARG1 EDX
#define IOCTL_ARG2 ESI
#define IOCTL_ARG3 EDI
#define IOCTL_ARG4 EBP

#define FS_BLOCK_SIZE 8192

typedef FsOpenNode* (*OpenVirtualNode)(const BoxedPtr<FsNode>& node, U32 flags);

class FsFileNode;

class Fs {
public:   
    static bool initFileSystem(const std::string& rootPath, const std::string& zipPath);
    static BoxedPtr<FsNode> getNodeFromLocalPath(const std::string& currentDirectory, const std::string& path, bool followLink, bool* isLink=NULL);    
    static BoxedPtr<FsNode> addFileNode(const std::string& path, const std::string& link, const std::string& nativePath, bool isDirectory, const BoxedPtr<FsNode>& parent);
    static BoxedPtr<FsNode> addVirtualFile(const std::string& path, OpenVirtualNode func, U32 mode, U32 rdev, const BoxedPtr<FsNode>& parent);
    static BoxedPtr<FsNode> addRootDirectoryNode(const std::string& path, const std::string& nativePath, const BoxedPtr<FsNode>& parent);
    static void remoteNameToLocal(std::string& path);
    static void localNameToRemote(std::string& path);
    static U32 makeLocalDirs(const std::string& path);
    static std::string getParentPath(const std::string& path);
    static std::string getNativeParentPath(const std::string& path);
    static std::string getFileNameFromPath(const std::string& path);
    static U32 readNativeFile(const std::string& nativePath, U8* buffer, U32 bufferLen);
    static void splitPath(const std::string& path, std::vector<std::string>& parts);
    static bool doesNativePathExist(const std::string& path);
    static std::string getFullPath(const std::string& currentDirectory, const std::string& path);
    static std::string getNativePathFromParentAndLocalFilename(const BoxedPtr<FsNode>& parent, const std::string fileName);    

    static std::string nativePathSeperator;

    static BoxedPtr<FsFileNode> rootNode;

private:
    friend class KUnixSocketObject;

    static BoxedPtr<FsNode> getNodeFromLocalPath(const std::string& currentDirectory, const std::string& path, BoxedPtr<FsNode>& lastNode, std::vector<std::string>& missingParts, bool followLink, bool* isLink=NULL);

    static U32 nextNodeId;    
    static BOXEDWINE_MUTEX nextNodeIdMutex;
};

#endif
