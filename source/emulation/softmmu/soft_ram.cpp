#include "boxedwine.h"

U8* ramPageAlloc() {
    U8* ram = new U8[PAGE_SIZE+1];
    memset(ram, 0, PAGE_SIZE);
    ram[PAGE_SIZE] = 1;
    return ram;
}

void ramPageIncRef(U8* ram) {
    if (ram[PAGE_SIZE]==255) {
        kpanic("max ram page ref count reached");
    }
    ram[PAGE_SIZE]++;
}

void ramPageDecRef(U8* ram) {
    ram[PAGE_SIZE]--;
    if (ram[PAGE_SIZE]==0)
        delete[] ram;
}

U32 ramPageRefCount(U8* ram) {
    return ram[PAGE_SIZE];
}