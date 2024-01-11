#ifndef __FSVIRTUALOPENNODE_H__
#define __FSVIRTUALOPENNODE_H__

#include "fsopennode.h"
#include "kerror.h"

class FsVirtualOpenNode: public FsOpenNode {
public:
    FsVirtualOpenNode(BoxedPtr<FsNode> node, U32 flags) : FsOpenNode(node, flags) {};

    // From FsOpenNode
    virtual S64  length() override {return 0;}
    virtual bool setLength(S64 length) override {return true;}
    virtual S64  getFilePointer() override {return 0;}
    virtual S64  seek(S64 pos) override {return 0;}
    virtual U32  map(KThread* thread, U32 address, U32 len, S32 prot, S32 flags, U64 off) override {return 0;}
    virtual bool canMap() override {return false;}
    virtual U32  ioctl(KThread* thread, U32 request) override {return -K_ENODEV;	}
    virtual void setAsync(bool isAsync) override {if (isAsync) kdebug("FsVirtualOpenNode::setAsync not implemented");}
    virtual bool isAsync() override {return false;}
    virtual void waitForEvents(BOXEDWINE_CONDITION& parentCondition, U32 events) override { kdebug("FsVirtualOpenNode::waitForEvents not implemented");}
    virtual bool isWriteReady() override {return true;}
    virtual bool isReadReady() override {return true;}
    virtual U32 readNative(U8* buffer, U32 len) override = 0;
    virtual U32 writeNative(U8* buffer, U32 len) override = 0;
    virtual void close() override {}
    virtual void reopen() override {}
    virtual bool isOpen() override {return true;}
};

#endif