#ifndef __FSVIRTUALOPENNODE_H__
#define __FSVIRTUALOPENNODE_H__

#include "fsopennode.h"
#include "kerror.h"

class FsVirtualOpenNode: public FsOpenNode {
public:
    FsVirtualOpenNode(BoxedPtr<FsNode> node, U32 flags) : FsOpenNode(node, flags) {};

    virtual S64  length() {return 0;}
    virtual bool setLength(S64 length) {return true;}
    virtual S64  getFilePointer() {return 0;}
    virtual S64  seek(S64 pos) {return 0;}
    virtual U32  map( U32 address, U32 len, S32 prot, S32 flags, U64 off) {return 0;}
    virtual bool canMap() {return false;}
    virtual U32  ioctl(U32 request) {return -K_ENODEV;	}
    virtual void setAsync(bool isAsync) {if (isAsync) kwarn("FsVirtualOpenNode::setAsync not implemented");}
    virtual bool isAsync() {return false;}
    virtual void waitForEvents(BOXEDWINE_CONDITION& parentCondition, U32 events) {kwarn("FsVirtualOpenNode::waitForEvents not implemented");}
    virtual bool isWriteReady() {return true;}
    virtual bool isReadReady() {return true;}    
    virtual U32 readNative(U8* buffer, U32 len) = 0;
    virtual U32 writeNative(U8* buffer, U32 len) = 0;
    virtual void close() {}
    virtual void reopen() {}
    virtual bool isOpen() {return true;}
};

#endif