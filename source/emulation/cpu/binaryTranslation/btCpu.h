#ifndef __BT_CPU_H__
#define __BT_CPU_H__

#ifdef BOXEDWINE_BINARY_TRANSLATOR
class BtData;
class BtCodeChunk;
class BtCPU : public CPU {
public:
    BtCPU(KMemory* memory) : CPU(memory), nativeHandle(0),
        exceptionAddress(0), 
        inException(false), 
        exceptionReadAddress(false), 
        returnHostAddress(0), 
        exceptionSigNo(0), 
        exceptionSigCode(0), 
        exceptionIp(0), 
#ifdef BOXEDWINE_64BIT_MMU
        eipToHostInstructionAddressSpaceMapping(NULL),
#endif
        returnToLoopAddress(NULL),
#ifdef BOXEDWINE_64BIT_MMU
        memOffset(0),
#endif
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
    void* returnToLoopAddress;   
    void* syncToHostAddress;
    void* syncFromHostAddress;
    void* doSingleOpAddress;
    int exitToStartThreadLoop; // this will be checked after a syscall, if set to 1 then then x64CPU.returnToLoopAddress will be called

    std::vector<U32> pendingCodePages;

    std::shared_ptr<BtCodeChunk> translateChunk(U32 ip);
    virtual void translateData(const std::shared_ptr<BtData>& data, const std::shared_ptr<BtData>& firstPass = nullptr) = 0;
    virtual void link(const std::shared_ptr<BtData>& data, std::shared_ptr<BtCodeChunk>& fromChunk, U32 offsetIntoChunk = 0) = 0;
    void* translateEipInternal(U32 ip);
#ifdef __TEST
    virtual void postTestRun() = 0;
#endif

#ifdef BOXEDWINE_64BIT_MMU
    U64 memOffset;
    void markCodePageReadOnly(BtData* data);
    U64 handleAccessException(U64 ip, U64 address, bool readAddress); // returns new ip, if 0 then don't set ip, but continue execution
    U64 handleCodePatch(U64 rip, U32 address);
    virtual bool handleStringOp(DecodedOp* op);
    void* eipToHostInstructionAddressSpaceMapping;
#endif

    U64 reTranslateChunk();
    U64 handleChangedUnpatchedCode(U64 rip);
    U64 handleIllegalInstruction(U64 ip);
    U64 handleMissingCode(U32 page, U32 offset);        
    DecodedOp* getOp(U32 eip, bool existing);
    void* translateEip(U32 ip);    
    void makePendingCodePagesReadOnly();
    U64 startException(U64 address, bool readAddress);
    U64 handleFpuException(int code);
    void startThread();
    void wakeThreadIfWaiting();    
    S32 preLinkCheck(BtData* data); // returns the index of the jump that failed

    // used by handleAccessException
    U32 destEip;
    U64 regPage;
    U64 regOffset;

    U32 largeAddressJumpInstruction;
    U32 pageJumpInstruction;
    U32 pageOffsetJumpInstruction;
protected:
    U64 getIpFromEip();
    virtual std::shared_ptr<BtData> createData() = 0;
};
#endif

#endif