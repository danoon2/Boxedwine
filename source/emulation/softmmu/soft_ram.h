#ifndef __SOFT_RAM_H__
#define __SOFT_RAM_H__

#include "platform.h"

typedef U32 RamPage;

RamPage ramPageAlloc();
RamPage ramPageAllocNative(U8* native);
U8* ramPageGet(RamPage page);
void ramPageRelease(RamPage page);
void ramPageRetain(RamPage page);
U32 ramPageUseCount(RamPage page);

void shutdownRam();

#endif
