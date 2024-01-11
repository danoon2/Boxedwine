#ifndef __FSZIPOPENNODE_H__
#define __FSZIPOPENNODE_H__

#include "fsopennode.h"

class FsZipNode;

class FsZipOpenNode : public FsOpenNode {
public:
    FsZipOpenNode(BoxedPtr<FsNode> node, std::shared_ptr<FsZipNode>& zipNode, U32 flags, U64 offset);

    // From FsOpenNode
    virtual S64  length() override;
    virtual bool setLength(S64 length) override;
    virtual S64  getFilePointer() override;
    virtual S64  seek(S64 pos) override;
    virtual U32  map(KThread* thread, U32 address, U32 len, S32 prot, S32 flags, U64 off) override;
    virtual bool canMap() override;
    virtual U32  ioctl(KThread* thread, U32 request) override;
    virtual void setAsync(bool isAsync) override;
    virtual bool isAsync() override;
    virtual void waitForEvents(BOXEDWINE_CONDITION& parentCondition, U32 events) override;
    virtual bool isWriteReady() override;
    virtual bool isReadReady() override;
    virtual U32 readNative(U8* buffer, U32 len) override;
    virtual U32 writeNative(U8* buffer, U32 len) override;
    virtual void close() override;
    virtual void reopen() override;
    virtual bool isOpen() override;

private:
    std::shared_ptr<FsZipNode> zipNode;
    S64 pos;
    U64 offset;
};

#endif