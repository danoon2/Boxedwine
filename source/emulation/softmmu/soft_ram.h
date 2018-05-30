#ifndef __SOFT_RAM_H__
#define __SOFT_RAM_H__

#include "platform.h"

U8* ramPageAlloc();
void ramPageIncRef(U8* ram);
void ramPageDecRef(U8* ram);
U32 ramPageRefCount(U8* ram);

#endif