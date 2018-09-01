#ifndef __NORMAL_CPU_H__
#define __NORMAL_CPU_H__

#include "../common/cpu.h"

class NormalCPU : public CPU {
public:
    NormalCPU();

    virtual void run();
    virtual DecodedBlock* getNextBlock();
};

#endif