#ifndef __BT_CODE_MEMORY_WRITE_H__
#define __BT_CODE_MEMORY_WRITE_H__

#ifdef BOXEDWINE_BINARY_TRANSLATOR
#define CLEAR_BUFFER_SIZE 256

class BtCPU;

class BtCodeMemoryWrite {
public:
    BtCodeMemoryWrite(BtCPU* cpu);
    BtCodeMemoryWrite(BtCPU* cpu, U32 address, U32 len);
    ~BtCodeMemoryWrite();

    void invalidateCode(U32 address, U32 len);
    void invalidateStringWriteToDi(bool repeat, U32 size);
    void restoreCodePageReadOnly();
private:
    U32 buffer[CLEAR_BUFFER_SIZE];
    U32 count;
    BtCPU* cpu;
};
#endif
#endif