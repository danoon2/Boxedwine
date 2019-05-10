#ifndef __X64_CPU_H__
#define __X64_CPU_H__

#ifdef BOXEDWINE_X64
#include "../common/cpu.h"

class X64Asm;

class x64CPU : public CPU {
public:
    x64CPU();

    virtual void run();
    virtual DecodedBlock* getNextBlock();
    virtual void restart();
    void* init();
    void* translateEip(U32 ip);

    U64 nativeHandle;
    jmp_buf* jmpBuf;
    BOXEDWINE_CONDITION endCond;

    U32 negSegAddress[6];

    U64 memOffset;
    U64 negMemOffset;
    bool inException;
    void*** eipToHostInstruction;
    DecodedOp* getExistingOp(U32 eip);
#ifdef __TEST
    void addReturnFromTest();
#endif

    void translateInstruction(X64Asm* data);    
    void link(X64Asm* data, void* address);
    void makePendingCodePagesReadOnly();
private:        
    void* translateEipInternal(X64Asm* parent, U32 ip);
    void translateData(X64Asm* data);        
    void markCodePageReadOnly(X64Asm* data);

    std::vector<U32> pendingCodePages;
};
#endif
#endif