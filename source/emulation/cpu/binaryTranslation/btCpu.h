#ifndef __BT_CPU_H__
#define __BT_CPU_H__

#ifdef BOXEDWINE_BINARY_TRANSLATOR
class BtData;

class BtCPU : public CPU {
public:
    BtCPU() : nativeHandle(0), 
        exceptionAddress(0), 
        inException(false), 
        exceptionReadAddress(false), 
        returnHostAddress(0), 
        exceptionSigNo(0), 
        exceptionSigCode(0), 
        exceptionIp(0), 
        eipToHostInstructionAddressSpaceMapping(NULL),
        returnToLoopAddress(NULL),
        memOffset(0),
        exitToStartThreadLoop(0) {}

    // from CPU
    virtual void run();
    virtual DecodedBlock* getNextBlock();

    virtual void* init() = 0; // called from run

    U64 nativeHandle;
    U64 exceptionAddress;
    bool inException;
    bool exceptionReadAddress;
    U64 returnHostAddress; // after returning from the signalHandler, this will contain the host address we should jump to
    int exceptionSigNo;
    int exceptionSigCode;
    U64 exceptionIp;
    void* eipToHostInstructionAddressSpaceMapping;    
    void* returnToLoopAddress;
    U64 memOffset;
    int exitToStartThreadLoop; // this will be checked after a syscall, if set to 1 then then x64CPU.returnToLoopAddress will be called

    std::vector<U32> pendingCodePages;
    
    jmp_buf* jmpBuf;

    virtual std::shared_ptr<BtCodeChunk> translateChunk(U32 ip) = 0;
    void* translateEipInternal(U32 ip);
#ifdef __TEST
    virtual void postTestRun() = 0;
#endif

    U64 reTranslateChunk();
    U64 handleChangedUnpatchedCode(U64 rip);
    U64 handleIllegalInstruction(U64 ip);
    DecodedOp* getOp(U32 eip, bool existing);
    void* translateEip(U32 ip);
    void markCodePageReadOnly(BtData* data);
    void makePendingCodePagesReadOnly();
    U64 startException(U64 address, bool readAddress, std::function<void(DecodedOp*)> doSyncFrom, std::function<void(DecodedOp*)> doSyncTo);
    U64 handleFpuException(int code, std::function<void(DecodedOp*)> doSyncFrom, std::function<void(DecodedOp*)> doSyncTo);
    void startThread();
    void wakeThreadIfWaiting();    
    S32 preLinkCheck(BtData* data); // returns the index of the jump that failed

protected:
    U64 getIpFromEip();
};
#endif

#endif