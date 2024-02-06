#ifndef __ARMV8BT_CPU_H__
#define __ARMV8BT_CPU_H__

#ifdef BOXEDWINE_ARMV8BT
#include "../binaryTranslation/btCpu.h"

#include "armv8btAsm.h"

class Armv8btCPU : public BtCPU {
public:
    Armv8btCPU(KMemory* memory);
    
    virtual void restart() override;
    virtual void* init() override;

    U8* parity_lookup;                
	U8*** eipToHostInstructionPages;    

    SSE sseConstants[6];

    ALIGN(U8 fpuState[512], 16);
	ALIGN(U8 originalFpuState[512], 16);
	U64 originalCpuRegs[16];	
    void* reTranslateChunkAddress;
    void* reTranslateChunkAddressFromReg;
    void* jmpAndTranslateIfNecessary;
#ifdef _DEBUG
    U32 fromEip;
    U64 exceptionRegs[32];
#endif
#ifdef __TEST
    void addReturnFromTest();
#endif

    virtual void link(BtData* data, std::shared_ptr<BtCodeChunk>& fromChunk, U32 offsetIntoChunk = 0) override;
    virtual void translateData(BtData* data, BtData* firstPass = nullptr) override;

    virtual void setSeg(U32 index, U32 address, U32 value) override;

#ifdef __TEST
    virtual void postTestRun() override {};
#endif
protected:
    virtual BtData* getData1() override { data1.reset(); return &data1; }
    virtual BtData* getData2() override { data2.reset(); return &data2; }
    Armv8btAsm data1;
    Armv8btAsm data2;
private:      
    void writeJumpAmount(BtData* data, U32 pos, U32 toLocation, U8* offset);
};
#endif
#endif
