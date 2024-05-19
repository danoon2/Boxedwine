#ifndef __BT_CPU_H__
#define __BT_CPU_H__

#ifdef BOXEDWINE_BINARY_TRANSLATOR
class BtData;
class BtCodeChunk;

#define CPU_OFFSET_CURRENT_OP (U32)(offsetof(BtCPU, currentSingleOp))

class BtCPU : public CPU {
public:
    BtCPU(KMemory* memory) : CPU(memory) {}

    // from CPU
    void run() override;
    DecodedBlock* getNextBlock() override;

    virtual void* init() = 0; // called from run

    U64 nativeHandle = 0;
    U64 exceptionAddress = 0;
    bool inException = false;
    bool exceptionReadAddress = false;
    U64 returnHostAddress = 0; // after returning from the signalHandler, this will contain the host address we should jump to
    int exceptionSigNo = 0;
    int exceptionSigCode = 0;
    U64 exceptionIp = 0;
    void* jmpAndTranslateIfNecessary = nullptr;
    void* returnToLoopAddress = nullptr;
    void* syncToHostAddress = nullptr;
    void* syncFromHostAddress = nullptr;
    void* doSingleOpAddress = nullptr;
    int exitToStartThreadLoop = 0; // this will be checked after a syscall, if set to 1 then then x64CPU.returnToLoopAddress will be called    

    std::vector<U32> pendingCodePages;

    std::shared_ptr<BtCodeChunk> translateChunk(U32 ip);
    virtual void translateData(BtData* data, BtData* firstPass = nullptr) = 0;
    virtual void link(BtData* data, std::shared_ptr<BtCodeChunk>& fromChunk, U32 offsetIntoChunk = 0) = 0;
    void* translateEipInternal(U32 ip);
#ifdef __TEST
    virtual void postTestRun() = 0;
#endif

    U64 reTranslateChunk();
    U64 handleMissingCode(U32 page, U32 offset);        
    DecodedOp* getOp(U32 eip, bool existing);
    void* translateEip(U32 ip);    
    void makePendingCodePagesReadOnly();
    U64 startException(U64 address, bool readAddress);
    U64 handleFpuException(int code);
    void startThread();
    void wakeThreadIfWaiting();    
    S32 preLinkCheck(BtData* data); // returns the index of the jump that failed

    U32 largeAddressJumpInstruction = 0;
    U32 pageJumpInstruction = 0;
    U32 pageOffsetJumpInstruction = 0;
protected:
    U64 getIpFromEip();
    virtual BtData* getData1() = 0;
    virtual BtData* getData2() = 0;
};
#endif

#endif
