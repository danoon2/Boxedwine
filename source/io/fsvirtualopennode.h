#ifndef __FSVIRTUALOPENNODE_H__
#define __FSVIRTUALOPENNODE_H__

#include "fsopennode.h"
#include "kerror.h"

class FsVirtualOpenNode: public FsOpenNode {
public:
    FsVirtualOpenNode(std::shared_ptr<FsNode> node, U32 flags) : FsOpenNode(node, flags) {};

    // From FsOpenNode
    S64 length() override {return 0;}
    bool setLength(S64 length) override {return true;}
    S64 getFilePointer() override {return 0;}
    S64 seek(S64 pos) override {return 0;}
    U32 map(KThread* thread, U32 address, U32 len, S32 prot, S32 flags, U64 off) override {return 0;}
    bool canMap() override {return false;}
    U32 ioctl(KThread* thread, U32 request) override {return -K_ENODEV;	}
    void setAsync(bool isAsync) override {if (isAsync) kdebug("FsVirtualOpenNode::setAsync not implemented");}
    bool isAsync() override {return false;}
    void waitForEvents(BOXEDWINE_CONDITION& parentCondition, U32 events) override { kdebug("FsVirtualOpenNode::waitForEvents not implemented");}
    bool isWriteReady() override {return true;}
    bool isReadReady() override {return true;}
    U32 readNative(U8* buffer, U32 len) override = 0;
    U32 writeNative(U8* buffer, U32 len) override = 0;
    void close() override {}
    void reopen() override {}
    bool isOpen() override {return true;}
};

#endif