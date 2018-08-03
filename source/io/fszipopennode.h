#ifndef __FSZIPOPENNODE_H__
#define __FSZIPOPENNODE_H__

#include "fsopennode.h"

class FsZipNode;

class FsZipOpenNode : public FsOpenNode {
public:
    FsZipOpenNode(BoxedPtr<FsZipNode> node, U32 flags, U64 offset);
    virtual S64  length();
    virtual bool setLength(S64 length);
    virtual S64  getFilePointer();
    virtual S64  seek(S64 pos);	
    virtual U32  map(U32 address, U32 len, S32 prot, S32 flags, U64 off);
    virtual bool canMap();
    virtual U32  ioctl(U32 request);	
    virtual void setAsync(bool isAsync);
    virtual bool isAsync();
    virtual void waitForEvents(U32 events);
    virtual bool isWriteReady();
    virtual bool isReadReady();
    virtual U32 readNative(U8* buffer, U32 len);
    virtual U32 writeNative(U8* buffer, U32 len);
    virtual void close();
    virtual void reopen();
    virtual bool isOpen();

private:
    BoxedPtr<FsZipNode> zipNode;
    S64 pos;
    U64 offset;
};

#endif