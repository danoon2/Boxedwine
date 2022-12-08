#ifndef __BT_CODE_MEMORY_WRITE_H__
#define __BT_CODE_MEMORY_WRITE_H__

#ifdef BOXEDWINE_BINARY_TRANSLATOR

class BtCPU;

class BtCodeMemoryWrite {
public:
    BtCodeMemoryWrite(BtCPU* cpu);
    BtCodeMemoryWrite(BtCPU* cpu, U32 address, U32 len);

    void invalidateCode(U32 address, U32 len);
    void invalidateStringWriteToDi(bool repeat, U32 size);
private:
    BtCPU* cpu;
};
#endif
#endif