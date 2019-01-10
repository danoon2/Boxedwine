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

    void*** opToAddressPages;
    U32 negSegAddress[6];

    U64 memOffset;
    U64 negMemOffset;
    bool inException;
#ifdef __TEST
    void addReturnFromTest();
#endif
private:
    void* translateEipInternal(X64Asm* parent, U32 ip);
    void translateData(X64Asm* data);
    void translateInstruction(X64Asm* data);
    void commitMappedAddresses(X64Asm* data, void* address);
    void markCodePageReadOnly(X64Asm* data);
    void mapAddressIntoProcess(X64Asm* data, U32 ip, void* address);
    void link(X64Asm* data, void* address);

    std::vector<U32> pendingCodePages;
};
#endif
#endif