#ifndef __SOFT_RAM_H__
#define __SOFT_RAM_H__

#include "platform.h"

typedef std::shared_ptr<U8[]> KRamPtr;

KRamPtr ramPageAlloc();
void shutdownRam();

#endif
