#ifndef __ARMV8BT_CPU_H__
#define __ARMV8BT_CPU_H__

#ifdef BOXEDWINE_ARMV8BT
#include "../binaryTranslation/btCpu.h"

class Armv8btAsm;

class Armv8btCPU : public BtCPU {
public:
    Armv8btCPU(KMemory* memory);
    
    virtual void restart();
    virtual void* init();    

    U8* parity_lookup;                
	void*** eipToHostInstructionPages;    

    SSE sseConstants[6];

    ALIGN(U8 fpuState[512], 16);
	ALIGN(U8 originalFpuState[512], 16);
	U64 originalCpuRegs[16];	
    void* reTranslateChunkAddress;
    void* reTranslateChunkAddressFromReg;
#ifdef BOXEDWINE_BT_DEBUG_NO_EXCEPTIONS
    void* jmpAndTranslateIfNecessary;
#endif
#ifdef _DEBUG
    U32 fromEip;
    U64 exceptionRegs[32];
#endif
#ifdef __TEST
    void addReturnFromTest();
#endif

    virtual void link(const std::shared_ptr<BtData>& data, std::shared_ptr<BtCodeChunk>& fromChunk, U32 offsetIntoChunk = 0);
    virtual void translateData(const std::shared_ptr<BtData>& data, const std::shared_ptr<BtData>& firstPass = nullptr);

    virtual void setSeg(U32 index, U32 address, U32 value);

#ifdef __TEST
    virtual void postTestRun() {};
#endif
protected:
    virtual std::shared_ptr<BtData> createData();
private:      
    void writeJumpAmount(const std::shared_ptr<BtData>& data, U32 pos, U32 toLocation, U8* offset);
};
#endif
#endif
