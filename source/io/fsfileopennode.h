#ifndef __FSFILEOPENNODE_H__
#define __FSFILEOPENNODE_H__

#include "fsopennode.h"

class FsFileNode;

class FsFileOpenNode : public FsOpenNode {
public:
    FsFileOpenNode(const std::shared_ptr<FsFileNode>& node, U32 flags, U32 handle);
    virtual ~FsFileOpenNode();

    // from FsOpenNode
    S64 length() override;
    bool setLength(S64 length) override;
    S64 getFilePointer() override;
    S64 seek(S64 pos) override;
    U32 map(KThread* thread, U32 address, U32 len, S32 prot, S32 flags, U64 off) override;
    bool canMap() override;
    U32 ioctl(KThread*, U32 request) override;
    void setAsync(bool isAsync) override;
    bool isAsync() override;
    void waitForEvents(BOXEDWINE_CONDITION& parentCondition, U32 events) override;
    bool isWriteReady() override;
    bool isReadReady() override;
    U32 readNative(U8* buffer, U32 len) override;
    U32 writeNative(U8* buffer, U32 len) override;
    void close() override;
    void reopen() override;
    bool isOpen() override;

private:
    std::shared_ptr<FsFileNode> fileNode;
    U32 handle;
};

#endif