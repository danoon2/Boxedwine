#ifndef __BT_CPU_H__
#define __BT_CPU_H__

#ifdef BOXEDWINE_BINARY_TRANSLATOR
class BtCPU : public CPU {
public:
    BtCPU() : nativeHandle(0), exceptionAddress(0), inException(false), exceptionReadAddress(false), returnHostAddress(0), exceptionSigNo(0), exceptionSigCode(0), exceptionIp(0), eipToHostInstructionAddressSpaceMapping(NULL) {}
    U64 nativeHandle;
    U64 exceptionAddress;
    bool inException;
    bool exceptionReadAddress;
    U64 returnHostAddress; // after returning from the signalHandler, this will contain the host address we should jump to
    int exceptionSigNo;
    int exceptionSigCode;
    U64 exceptionIp;
    void* eipToHostInstructionAddressSpaceMapping;    

    virtual void startThread() = 0;
    virtual U64 startException(U64 address, bool readAddress, std::function<void(DecodedOp*)> doSyncFrom, std::function<void(DecodedOp*)> doSyncTo) = 0;
    virtual U64 handleIllegalInstruction(U64 ip) = 0;    
    virtual U64 handleFpuException(int code, std::function<void(DecodedOp*)> doSyncFrom, std::function<void(DecodedOp*)> doSyncTo) = 0;
    virtual void makePendingCodePagesReadOnly() = 0;
    virtual std::shared_ptr<BtCodeChunk> translateChunk(U32 ip) = 0;
    virtual void* translateEip(U32 ip) = 0;
#ifdef __TEST
    virtual void postTestRun() = 0;
#endif
};
#endif

#endif