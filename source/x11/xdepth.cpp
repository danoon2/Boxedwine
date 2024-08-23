#include "boxedwine.h"
#include "x11.h"

void Depth::read(KMemory* memory, U32 address) {
	depth = X11_READD(Depth, address, depth);
	nvisuals = X11_READD(Depth, address, nvisuals);
	visuals = X11_READD(Depth, address, visuals);
}

void Depth::write(KMemory* memory, U32 address) {
	X11_WRITED(Depth, address, depth, depth);
	X11_WRITED(Depth, address, nvisuals, nvisuals);
	X11_WRITED(Depth, address, visuals, visuals);
}