#include "boxedwine.h"
#include "x11.h"

void XRectangle::read(KMemory* memory, U32 address) {
	x = (S16)memory->readw(address); address += 2;
	y = (S16)memory->readw(address); address += 2;
	width = memory->readw(address); address += 2;
	height = memory->readw(address);
}

U32 XPixmapFormatValues::write(KMemory* memory, U32 address, U32 depth, U32 bits_per_pixel, U32 scanline_pad) {
	memory->writed(address, depth);
	memory->writed(address+4, bits_per_pixel);
	memory->writed(address+8, scanline_pad);
	return 12;
}