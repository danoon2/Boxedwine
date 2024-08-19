#include "boxedwine.h"
#include "x11.h"

void XIEventMask::read(KMemory* memory, U32 address) {
	deviceid = memory->readd(address);
	mask_len = memory->readd(address + 4);
	maskAddress = memory->readd(address + 8);
}