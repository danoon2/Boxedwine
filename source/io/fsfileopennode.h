#ifndef __FSFILEOPENNODE_H__
#define __FSFILEOPENNODE_H__

#include "fsopennode.h"

class FsFileNode;

class FsFileOpenNode : public FsOpenNode {
public:
    FsFileOpenNode(BoxedPtr<FsFileNode> node, U32 flags, U32 handle);
    virtual ~FsFileOpenNode();

    virtual S64  length();
    virtual bool setLength(S64 length);
    virtual S64  getFilePointer();
    virtual S64  seek(S64 pos);	
    virtual U32  map(U32 address, U32 len, S32 prot, S32 flags, U64 off);
    virtual bool canMap();
    virtual U32  ioctl(U32 request);	
    virtual void setAsync(bool isAsync);
    virtual bool isAsync();
    virtual void waitForEvents(BOXEDWINE_CONDITION& parentCondition, U32 events);
    virtual bool isWriteReady();
    virtual bool isReadReady();
    virtual U32 readNative(U8* buffer, U32 len);
    virtual U32 writeNative(U8* buffer, U32 len);
    virtual void close();
    virtual void reopen();
    virtual bool isOpen();

private:
    BoxedPtr<FsFileNode> fileNode;
    U32 handle;
};

#endif