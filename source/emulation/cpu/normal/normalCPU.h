#ifndef __NORMAL_CPU_H__
#define __NORMAL_CPU_H__

#include "../common/cpu.h"

class KThread;

class NormalCPU : public CPU {
public:
    NormalCPU();
    void run();

    virtual DecodedBlock* getNextBlock();
};

#endif