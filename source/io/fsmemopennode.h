#ifndef __FSMEMOPENNODE_H__
#define __FSMEMOPENNODE_H__

#include "fsopennode.h"

class FsMemNode;

class FsMemOpenNode : public FsOpenNode {
public:
    FsMemOpenNode(U32 flags, BoxedPtr<FsNode> node);
    virtual ~FsMemOpenNode();

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

    U64 lastModified() {return this->lastModifiedTime;}
    U32 getSeals() {return this->seals;}
    U32 addSeals(U32 seals);
private:
    U32 seals;
    std::vector<U8> buffer;
    S64 pos;
    bool isClosed;
    U64 lastModifiedTime;
};

#endif