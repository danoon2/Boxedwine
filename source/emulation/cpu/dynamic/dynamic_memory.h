#ifndef __DYNAMIC_MEMORY_H__
#define __DYNAMIC_MEMORY_H__

class DynamicMemoryData {
public:
    DynamicMemoryData(void* p, U32 len) : p(p), len(len) {}

    void* p;
    U32 len;
};

class DynamicMemory {
public:
    DynamicMemory() : dynamicExecutableMemoryPos(0), dynamicExecutableMemoryLen(0) {}
    ~DynamicMemory() {
        for (U32 i = 0; i < this->dynamicExecutableMemory.size(); i++) {
            Platform::releaseNativeMemory(this->dynamicExecutableMemory[i].p, this->dynamicExecutableMemory[i].len);
        }
    }
    std::vector<DynamicMemoryData> dynamicExecutableMemory;
    U32 dynamicExecutableMemoryPos;
    U32 dynamicExecutableMemoryLen;
};


#endif