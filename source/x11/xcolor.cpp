#include "boxedwine.h"
#include "x11.h"

void XColor::read(KMemory* memory, U32 address) {
	pixel = memory->readd(address);
	red = memory->readw(address + 4);
	green = memory->readw(address + 6);
	blue = memory->readw(address + 8);
	flags = memory->readb(address + 10);
}